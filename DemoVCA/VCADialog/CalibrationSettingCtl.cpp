#include "stdafx.h"
#include "../resource.h"
#include "CalibrationSettingCtl.h"
#include "VCADialog.h"
#include "../Render/VCARender.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CCalibrationSettingCtl::CCalibrationSettingCtl()
{
	m_bSetup		= FALSE;
	m_pDataMgr		= NULL;
	m_hWnd			= NULL;
	m_hMenu			= NULL;
	m_bUpdatingCalibParam	= FALSE;
}

CCalibrationSettingCtl::~CCalibrationSettingCtl()
{
	if( m_hMenu ) 
		DestroyMenu(m_hMenu);
}

BOOL CCalibrationSettingCtl::Setup(CVCADialog *pVCADialog)
{
	if(m_bSetup){
		TRACE("CCalibrationSettingCtl::Setup before \n");
		return TRUE;
	}
	
	m_pVCADialog= pVCADialog;
	m_pDataMgr	= m_pVCADialog->GetVCADataMgr();
	m_hWnd		= m_pVCADialog->GetSafeHwnd();

	m_bMouseDraging = FALSE;
	m_bMetricNotImp = TRUE;

	//need to client rect to set default position of mimic
	m_ClientRect.left	= m_ClientRect.top = 0;
	m_ClientRect.right	= 720;
	m_ClientRect.bottom	= 576;

	m_pDataMgr->RegisterObserver( this );
	
	InitCalibParam();

	m_bSetup = TRUE;
	return TRUE;
}

void CCalibrationSettingCtl::Reset(BOOL bResetPause)
{
	VCA5_APP_CALIB_INFO* pCalibInfo = m_pDataMgr->GetCalibInfo();

	m_fCameraHeight = pCalibInfo->fHeight;

	if( VCA5_HEIGHT_UNITS_FEET == pCalibInfo->ulHeightUnits )
	{
		m_fCameraHeight *= 0.3048f;
	}

	SetCameraParams(m_fCameraHeight, pCalibInfo->fTilt, pCalibInfo->fFOV, pCalibInfo->fPan, pCalibInfo->fRoll, 1, m_pDataMgr->m_bResetObjectModel);

	m_pDataMgr->m_bResetObjectModel = FALSE;
	m_bUpdatingCalibParam	= TRUE;

	if( VCA5_HEIGHT_UNITS_METERS == pCalibInfo->ulHeightUnits ){
		m_bMetricNotImp = TRUE;
	}
	else{
		m_bMetricNotImp = FALSE;
	}

//	OnChangeCalibparams();

//	ShowCaliInfoStatus();
	DWORD CurDisplayOption = m_pDataMgr->GetDisplayFlags();
	if(bResetPause){
		CurDisplayOption &= (~IVCARender::CALIB_PAUSE);
		m_pDataMgr->SetDisplayFlags(CurDisplayOption);
		CheckDlgButton(m_hWnd, IDC_CHECK_PAUSE, BST_UNCHECKED); 
	}

	m_bUpdatingCalibParam	= FALSE;
	m_bMouseDraging = FALSE;

	if(pCalibInfo->calibrationStatus >= VCA5_CALIB_STATUS_CALIBRATED){
		CurDisplayOption = m_pDataMgr->GetDisplayFlags();
		CurDisplayOption |= (IVCARender::DISPLAY_OBJECT_HEIGHT|IVCARender::DISPLAY_OBJECT_SPEED|IVCARender::DISPLAY_OBJECT_AREA);
		m_pDataMgr->SetDisplayFlags(CurDisplayOption);
	}else{
		CurDisplayOption = m_pDataMgr->GetDisplayFlags();
		CurDisplayOption &= ~(IVCARender::DISPLAY_OBJECT_HEIGHT|IVCARender::DISPLAY_OBJECT_SPEED|IVCARender::DISPLAY_OBJECT_AREA|IVCARender::DISPLAY_OBJECT_CLASSIFICATION);
		m_pDataMgr->SetDisplayFlags(CurDisplayOption);
	}
}


void CCalibrationSettingCtl::FireOnEvent(DWORD uiEvent, DWORD uiContext)
{
	if( uiEvent & IVCAConfigureObserver::VCA_CALIB_UPDATE )
	{
		// Calibration has been updated from elsewhere
		// So sync up to that
		Reset(FALSE);
	}
}

void CCalibrationSettingCtl::OnChangeCalibparams()
{
	//2. Update Data Manager.
	VCA5_APP_CALIB_INFO*	pCalibInfo = m_pDataMgr->GetCalibInfo();

	pCalibInfo->fHeight	= m_fCameraHeight;
	pCalibInfo->fTilt	= m_fTiltAngle_deg;
	pCalibInfo->fFOV	= m_fYFOV_deg; 
	pCalibInfo->fPan	= m_fYPan_deg; 
	pCalibInfo->fRoll	= m_fYRoll_deg; 

	if( m_bMetricNotImp ){
		pCalibInfo->ulHeightUnits	= VCA5_HEIGHT_UNITS_METERS;
		pCalibInfo->ulSpeedUnits	= VCA5_SPEED_UNITS_KPH;
	}
	else
	{
		pCalibInfo->ulHeightUnits	= VCA5_HEIGHT_UNITS_FEET;
		pCalibInfo->ulSpeedUnits	= VCA5_SPEED_UNITS_MPH;

		pCalibInfo->fHeight *= 3.2808399f;
	}

	// By definition we must be setting some calibration
	//pCalibInfo->calibrationStatus = VCA5_CALIB_STATUS_CALIBRATED;
	m_pDataMgr->SetChangeCalibInfo(TRUE);
	
	// Fire event for update
	m_pDataMgr->FireEvent( IVCAConfigureObserver::VCA_CALIB_UPDATE, this );
}


void CCalibrationSettingCtl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int	wmEvent = HIWORD(wParam);

	if(wmId == IDC_CHECK_PAUSE){
		DWORD CurDisplayOption = m_pDataMgr->GetDisplayFlags();
		if(CurDisplayOption & IVCARender::CALIB_PAUSE)
			CurDisplayOption &= (~IVCARender::CALIB_PAUSE);
		else
			CurDisplayOption |= (IVCARender::CALIB_PAUSE);
		m_pDataMgr->SetDisplayFlags(CurDisplayOption);
	}else if(wmId == ID_CALIB_SKY){
		m_pDataMgr->m_uiCalibDrawOptions ^= VCA_CALIB_DRAW_HORIZON;
	}else if(wmId == ID_CALIB_RULER){
		m_pDataMgr->m_uiCalibDrawOptions |= VCA_CALIB_RULER_DRAW; 
		BeginRuler(m_pDataMgr->m_LastClickedMousePos);
	}else if(wmId == ID_CALIB_TRANS_GRID){
		m_pDataMgr->m_uiCalibDrawOptions ^= VCA_CALIB_TRANS_GRID; // 
	}else if(wmId == ID_CALIB_INSERT_OBJECT){
		InsertObjectModel(m_pDataMgr->m_LastClickedMousePos);
	}
}


void CCalibrationSettingCtl::BeginRuler( CPoint screenCoord )
{
	float fDist2Plane;
	D3DXVECTOR3 v = UnprojectZplane(screenCoord, -1, 0, &fDist2Plane);
	if (fDist2Plane >= 0.0f && fDist2Plane <= 1.0f)
	{
		m_pDataMgr->m_RulerPos[0] = m_pDataMgr->m_RulerPos[1] = v;
	}
}


void CCalibrationSettingCtl::UpdateRuler( CPoint screenCoord )
{
	float fDist2Plane;
	D3DXVECTOR3 v = UnprojectZplane(screenCoord, -1, 0, &fDist2Plane);
	if (fDist2Plane >= 0.0f && fDist2Plane <= 1.0f)
	{
		m_pDataMgr->m_RulerPos[1] = v;
	}
}


void CCalibrationSettingCtl::InsertObjectModel( CPoint screenCoord )
{
	D3DXVECTOR3 v;
	float fDist2Plane;
	v = UnprojectZplane(screenCoord, -1, m_fHumanHeight/2, &fDist2Plane);

	if (fDist2Plane >= 0.0f && fDist2Plane <= 1.0f){
		m_pDataMgr->InsertObjectModel(v);
	}
}


void CCalibrationSettingCtl::OnLButtonDown(UINT nFlags, POINT PixPoint)
{
	SetFocus( m_hWnd );
	PickGrid(PixPoint);
	m_bMouseDraging = TRUE;
	if(m_pDataMgr->m_uiCalibDrawOptions&VCA_CALIB_RULER_DRAW){
		m_pDataMgr->m_uiCalibDrawOptions ^= VCA_CALIB_RULER_DRAW;
	}

	m_pDataMgr->m_LastClickedMousePos = PixPoint;
	SetCapture(m_hWnd);
}


void CCalibrationSettingCtl::OnLButtonUp(UINT nFlags, POINT point)
{
	EndGridDrag();
	m_bMouseDraging = FALSE;
	ReleaseCapture();
}


void CCalibrationSettingCtl::OnRButtonUp(UINT nFlags, POINT PixPoint)
{
	ClipCoords(PixPoint);
	m_pDataMgr->m_LastClickedMousePos = PixPoint;
	::ClientToScreen( m_hWnd, (LPPOINT)&PixPoint.x );
	
	m_hMenu = CreatePopupMenu();

	AppendMenu( m_hMenu, MF_STRING,	ID_CALIB_RULER,		_T("Virtual Ruler") );
	AppendMenu( m_hMenu, MF_STRING,	ID_CALIB_SKY,		_T("Draw Horizon" ));
	AppendMenu( m_hMenu, MF_STRING,	ID_CALIB_TRANS_GRID,		_T("Transparent Grid" ));
	AppendMenu( m_hMenu, MF_STRING,	ID_CALIB_INSERT_OBJECT,		_T("Add Mimic" ));

	CheckMenuItem( m_hMenu,	ID_CALIB_SKY,	m_pDataMgr->m_uiCalibDrawOptions & VCA_CALIB_DRAW_HORIZON ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( m_hMenu,	ID_CALIB_TRANS_GRID,	m_pDataMgr->m_uiCalibDrawOptions & VCA_CALIB_TRANS_GRID ? MF_CHECKED : MF_UNCHECKED);

	TrackPopupMenu( m_hMenu, TPM_LEFTALIGN, PixPoint.x , PixPoint.y , 0, m_hWnd, NULL );
}


void CCalibrationSettingCtl::OnRButtonDown(UINT nFlags, POINT PixPoint)
{
	if(m_pDataMgr->m_uiCalibDrawOptions&VCA_CALIB_RULER_DRAW){
		m_pDataMgr->m_uiCalibDrawOptions &= ~VCA_CALIB_RULER_DRAW;	
	}
}


void CCalibrationSettingCtl::OnMouseWheel(UINT nFlags, short zDelta, POINT pt)
{
	ZoomGrid(pt, zDelta);
}


void CCalibrationSettingCtl::OnMouseMove(UINT nFlags, POINT PixPoint)
{
	if(m_bMouseDraging){
		DragGrid(PixPoint);
	}else if(m_pDataMgr->m_uiCalibDrawOptions&VCA_CALIB_RULER_DRAW) {
		UpdateRuler(PixPoint);
	}else{
		MouseOver(PixPoint);
	}

	m_pDataMgr->m_LastClickedMousePos = PixPoint;
}


void CCalibrationSettingCtl::ClipCoords(POINT &screenCoord)
{
	UINT w, h;
	w	= m_ClientRect.right;
	h	= m_ClientRect.bottom;
	screenCoord.x = (LONG)min((LONG)w-1, max(0, screenCoord.x));
	screenCoord.y = (LONG)min((LONG)h-1, max(0, screenCoord.y));
}

void CCalibrationSettingCtl::InitCalibParam()
{
	// Initialize cam params
	VCA5_APP_CALIB_INFO*	pCalibInfo = m_pDataMgr->GetCalibInfo();

	m_fTiltAngle_deg = pCalibInfo->fTilt;	
	m_fYFOV_deg  = pCalibInfo->fFOV;
	m_fYPan_deg  = pCalibInfo->fPan;
	m_fYRoll_deg  = pCalibInfo->fRoll;

	if( pCalibInfo->ulHeightUnits == VCA5_HEIGHT_UNITS_METERS )
	{
		m_bMetricNotImp = TRUE;
		m_fCameraHeight = pCalibInfo->fHeight;
	}
	else
	{
		m_bMetricNotImp = FALSE;
		m_fCameraHeight = pCalibInfo->fHeight;

		// Convert back to m
		m_fCameraHeight *= 0.3048f;
	}

//	m_fCameraHeight		= 10.0f;
//	m_fYFOV_deg			= 40.0f;
//	m_fTiltAngle_deg	= 30.0f;
	m_fHumanHeight		= HUMAN_HEIGHT;
	m_fScale_fact		= 1.0f;

	
	m_bTiltingGrid		= FALSE;
	m_vecGridRot		= D3DXVECTOR3(D3DXToRadian(- 90.0f - m_fTiltAngle_deg), 0.0f, 0.0f);

	m_lastTickCount		= 0;
	
	SetupMatricies();
}

void CCalibrationSettingCtl::SetCameraParams(float fCameraHeight, float fTiltAngle_deg, float fYFOV_deg, 
											 float fYPan_deg, float fYRoll_deg, float fScale_fact, BOOL bCentre)
{
	if (!bCentre)
	{
		HoldModelPositions_Begin();
	}

	m_fCameraHeight		= fCameraHeight;
	m_fYFOV_deg			= fYFOV_deg;
	m_fYPan_deg			= fYPan_deg;
	m_fYRoll_deg		= fYRoll_deg;
	m_fTiltAngle_deg	= fTiltAngle_deg;
	m_fScale_fact		= fScale_fact;

//	m_vecGridRot		= D3DXVECTOR3(D3DXToRadian(- 90.0f + fTiltAngle_deg), 0.0f, 0.0f);
	m_vecGridRot.x = D3DXToRadian(m_fTiltAngle_deg);
	m_vecGridRot.z = D3DXToRadian(m_fYPan_deg);
	m_vecGridRot.y = D3DXToRadian(m_fYRoll_deg);

	/// recalculate the model matracies
	SetupMatricies();

	if (bCentre){
		/// centre the models to nice position
		UINT w, h;
		w	=	m_ClientRect.right;
		h	=	m_ClientRect.bottom;
		
		CPoint centrePoint;
		if (fTiltAngle_deg >= 0.0f && fTiltAngle_deg <= 60.0f) 
			centrePoint = CPoint(w/2, h-100);
		else if (fTiltAngle_deg > 60.0f && fTiltAngle_deg < 120.0f )
			centrePoint = CPoint(w/2, h/2);
		else if (fTiltAngle_deg < 0.0f)
			centrePoint = CPoint(w/2, h-1);
		else if (fTiltAngle_deg <= 180.0f && fTiltAngle_deg >= 120.0f) 
			centrePoint = CPoint(w/2, 100);
		else if (fTiltAngle_deg > 180.0f)
			centrePoint = CPoint(w/2, 1);
		else
			centrePoint = CPoint(w/2, h/2);

		D3DXVECTOR3 v = UnprojectZplane(centrePoint, -1);

		ObjectModel *pObjectModel = m_pDataMgr->GetObjectModel(0);
		pObjectModel->vecPos.x = v.x - 1.0f;
		pObjectModel->vecPos.y = v.y - 1.0f;

		pObjectModel = m_pDataMgr->GetObjectModel(1);
		pObjectModel->vecPos.x = v.x + 1.0f;
		pObjectModel->vecPos.y = v.y + 1.0f;

	}else{ // make sure they not on the edge
		HoldModelPositions_End();
		//CheckModelPositions();
	}
}


void CCalibrationSettingCtl::SetupMatricies()
{
	D3DXMATRIX matTranslation, matModelRotationTilt, matModelTiltLevel, matModelRotationPan, matModelRotationRoll;

	D3DXMatrixPerspectiveFov( &m_matProjModel, D3DX_PI*m_fYFOV_deg/180.0f, float(m_csVideoSize.cy)/float(m_csVideoSize.cx), 0.1f, 1000.0f );
	D3DXMatrixPerspectiveFov( &m_matProjModel_Clipped, D3DX_PI*m_fYFOV_deg/180.0f, float(m_csVideoSize.cy)/float(m_csVideoSize.cx), 0.1f, 510.0f );

	D3DXMatrixTranslation(&matTranslation, 0, 0, -m_fCameraHeight);
	D3DXMatrixRotationX(&matModelRotationTilt, m_vecGridRot.x);
	D3DXMatrixRotationX(&matModelTiltLevel, -D3DX_PI/2);
	D3DXMatrixRotationY(&matModelRotationPan, m_vecGridRot.z);
	D3DXMatrixRotationZ(&matModelRotationRoll, m_vecGridRot.y);

	m_matView_noTrans = matModelTiltLevel*matModelRotationPan*matModelRotationTilt*matModelRotationRoll;
	m_matView = matTranslation*m_matView_noTrans;
}


void CCalibrationSettingCtl::SetupModelMatrix(int iModelIdx)
{
	D3DXMATRIX matRotationY, matTranslation, matModelTranslation, matTranslation2;

	ObjectModel *pObjectModel = m_pDataMgr->GetObjectModel(iModelIdx);

	D3DXMatrixRotationY(&matRotationY, pObjectModel->vecRot.z);
	D3DXMatrixTranslation(&matTranslation, 0, 0, -m_fHumanHeight/2);
	D3DXMatrixTranslation(&matTranslation2, 0, 0, m_fHumanHeight/2);

	matRotationY = matTranslation*matRotationY*matTranslation2;

	D3DXMatrixTranslation(&matModelTranslation, pObjectModel->vecPos.x, pObjectModel->vecPos.y, 0);
	pObjectModel->matObjectModel = matRotationY*matModelTranslation;
}


void CCalibrationSettingCtl::CheckModelPositions()
{
	D3DXMATRIX matTransform;
	D3DXVECTOR4 out;
	D3DXVECTOR3 v, v2;
	CPoint screenCoord;
	float fDist2Plane;

	SetupMatricies();

	for (int i = 0; i < m_pDataMgr->GetObjectModelCnt(); i++){
		ObjectModel *pObjectModel = m_pDataMgr->GetObjectModel(i);
		matTransform = pObjectModel->matObjectModel*m_matView*m_matProjModel;

		/// check at each corner on the bounding box
		for (int k = 0; k < 8; k++){
			bool repositionX = false;
			bool repositionY = false;

			v.x = (k&1) == 0 ? pObjectModel->fMinX : pObjectModel->fMaxX;
			v.y = ((k/2)&1) == 0 ? pObjectModel->fMinY : pObjectModel->fMaxY;
			v.z = ((k/4)&1) == 0 ? pObjectModel->fMinZ : pObjectModel->fMaxZ;

			D3DXVec3Transform(&out, &v, &matTransform);

			out.x /= out.w;
			out.y /= out.w;

			if (out.x > 1.0f) {
				out.x = 1.0f;
				repositionX = true;
			}else if (out.x < -1.0f){
				out.x = -1.0f;
				repositionX = true;
			}

			if (out.y > 1.0f){
				out.y = 1.0f;
				repositionY = true;
			}else if (out.y < -1.0f){
				out.y = -1.0f;
				repositionY = true;
			}

			if (repositionX || repositionY){
				v2.x = out.x;
				v2.y = out.y;

				D3DXVec3Transform(&out, &v, &pObjectModel->matObjectModel);
				v2 = UnprojectZplane(v2, -1, out.z, &fDist2Plane);

				if (fDist2Plane >= 0.0f && fDist2Plane < 0.9f){
					pObjectModel->vecPos.x += v2.x - out.x;;
					pObjectModel->vecPos.y += v2.y - out.y;
				}

				SetupModelMatrix(i);
				matTransform = pObjectModel->matObjectModel*m_matView*m_matProjModel;
			}
		}
	}
}

ObjectModel* CCalibrationSettingCtl::PickObject(CPoint screenCoord, D3DXVECTOR3 *pPickedPos)
{
	D3DXVECTOR3 v, vPicked;
	ObjectModel* pObj;
	ObjectModel* pObjPicked = NULL;
	float fDist2PlaneMin = FLT_MAX;
	float fDist2Plane;

	/// try pick object from any bounding box face
	for (int i = 0; i < m_pDataMgr->GetObjectModelCnt(); i++, pObj++){
		pObj = m_pDataMgr->GetObjectModel(i);
		v = UnprojectXplane(screenCoord, i, pObj->fMinX, &fDist2Plane);
		if (v.y > pObj->fMinY && v.y < pObj->fMaxY && v.z > pObj->fMinZ && v.z < pObj->fMaxZ && fDist2Plane < fDist2PlaneMin){
			fDist2PlaneMin = fDist2Plane;
			pObjPicked = pObj;
			vPicked = v;
		}

		v = UnprojectXplane(screenCoord, i, pObj->fMaxX,  &fDist2Plane);
		if (v.y > pObj->fMinY && v.y < pObj->fMaxY && v.z > pObj->fMinZ && v.z < pObj->fMaxZ && fDist2Plane < fDist2PlaneMin){
			fDist2PlaneMin = fDist2Plane;
			pObjPicked = pObj;
			vPicked = v;
		}

		v = UnprojectYplane(screenCoord, i, pObj->fMinY,  &fDist2Plane);
		if (v.x > pObj->fMinX && v.x < pObj->fMaxX && v.z > pObj->fMinZ && v.z < pObj->fMaxZ && fDist2Plane < fDist2PlaneMin){
			fDist2PlaneMin = fDist2Plane;
			pObjPicked = pObj;
			vPicked = v;
		}

		v = UnprojectYplane(screenCoord, i, pObj->fMaxY,  &fDist2Plane);
		if (v.x > pObj->fMinX && v.x < pObj->fMaxX && v.z > pObj->fMinZ && v.z < pObj->fMaxZ && fDist2Plane < fDist2PlaneMin){
			fDist2PlaneMin = fDist2Plane;
			pObjPicked = pObj;
			vPicked = v;
		}

		v = UnprojectZplane(screenCoord, i, pObj->fMinZ,  &fDist2Plane);
		if (v.x > pObj->fMinX && v.x < pObj->fMaxX && v.y > pObj->fMinY && v.y < pObj->fMaxY && fDist2Plane < fDist2PlaneMin){
			fDist2PlaneMin = fDist2Plane;
			pObjPicked = pObj;
			vPicked = v;
		}

		v = UnprojectZplane(screenCoord, i, pObj->fMaxZ,  &fDist2Plane);
		if (v.x > pObj->fMinX && v.x < pObj->fMaxX && v.y > pObj->fMinY && v.y < pObj->fMaxY && fDist2Plane < fDist2PlaneMin){
			fDist2PlaneMin = fDist2Plane;
			pObjPicked = pObj;
			vPicked = v;
		}
	}

	if (pPickedPos)
		*pPickedPos = vPicked;

	return pObjPicked;
}


void CCalibrationSettingCtl::PickGrid(POINT screenCoord)
{
	D3DXVECTOR3 vPickedPos;
	ObjectModel* pObjPicked = PickObject(screenCoord, &vPickedPos);

	if (pObjPicked){
		D3DXVECTOR4 out;
		D3DXVec3Transform(&out, &vPickedPos, &pObjPicked->matObjectModel); // project picked coords from model space to view space
		pObjPicked->vecPickPos	= D3DXVECTOR3(out.x, out.y, out.z) - pObjPicked->vecPos; // get offset from current object position
		pObjPicked->bDragging	= TRUE;
		m_bTiltingGrid			= FALSE;
	}else{
		m_tiltStartPos			= screenCoord;
		m_bTiltingGrid			= TRUE;
	}
}


void CCalibrationSettingCtl::EndGridDrag()
{
	for (int i = 0; i < m_pDataMgr->GetObjectModelCnt(); i++){
		ObjectModel *pObjectModel = m_pDataMgr->GetObjectModel(i);
		pObjectModel->bDragging = FALSE;
	}
	m_bTiltingGrid = FALSE;
}


void CCalibrationSettingCtl::HoldModelPositions_Begin()
{
	D3DXMATRIX matTransform;
	D3DXVECTOR4 out;
	CPoint screenCoord;

	SetupMatricies();

	for (int i = 0; i < m_pDataMgr->GetObjectModelCnt(); i++){
		ObjectModel *pObjectModel = m_pDataMgr->GetObjectModel(i);

		matTransform = m_matView*m_matProjModel;

		D3DXVec3Transform(&out, &pObjectModel->vecPos, &matTransform);

		out.x /= out.w;
		out.y /= out.w;

		pObjectModel->vecLogicalScreenPos.x = out.x;
		pObjectModel->vecLogicalScreenPos.y = out.y;

	}
}


void CCalibrationSettingCtl::HoldModelPositions_End()
{
	D3DXVECTOR3 v,v2;

	SetupMatricies();
	float fDist2Plane;

	for (int i = 0; i < m_pDataMgr->GetObjectModelCnt(); i++){
		ObjectModel *pObjectModel = m_pDataMgr->GetObjectModel(i);

		v.x = pObjectModel->vecLogicalScreenPos.x;
		v.y = pObjectModel->vecLogicalScreenPos.y;
		v.z = 0;

		v2 = UnprojectZplane(v, -1, 0, &fDist2Plane);

		if (fDist2Plane >= 0.0f && fDist2Plane <= 1.0f){
			pObjectModel->vecPos = v2;
		}
	}
}


void CCalibrationSettingCtl::DragGrid(POINT screenCoord)
{
	D3DXVECTOR3 v, v2;
	float fDist2Plane;
	ObjectModel* pObj;
	int i;

	for (i = 0; i < m_pDataMgr->GetObjectModelCnt(); i++){
		pObj = m_pDataMgr->GetObjectModel(i);
		if (pObj->bDragging){
			ClipCoords(screenCoord);
			v = UnprojectZplane(screenCoord, -1, pObj->vecPickPos.z, &fDist2Plane);

			if (fDist2Plane >= 0.0f && fDist2Plane <= 1.0f){
				pObj->vecPos.y = v.y - pObj->vecPickPos.y;
				pObj->vecPos.x = v.x - pObj->vecPickPos.x;
			}
			break;
		}
	}

	if (m_bTiltingGrid && i == m_pDataMgr->GetObjectModelCnt()){	
		UINT w, h;

		HoldModelPositions_Begin();

		w	= m_ClientRect.right;
		h	= m_ClientRect.bottom;

		float fang		= float(m_tiltStartPos.y - screenCoord.y)*D3DXToRadian(m_fYFOV_deg)/float(h);
		m_vecGridRot.x += fang;
		//m_vecGridRot.x	= min(D3DXToRadian(m_fYFOV_deg/2), max(-D3DX_PI/2 -D3DXToRadian(m_fYFOV_deg/2), m_vecGridRot.x));
		m_vecGridRot.x = min(D3DXToRadian(90.0f + m_fYFOV_deg/2), max(D3DXToRadian(- m_fYFOV_deg/2), m_vecGridRot.x));
		m_tiltStartPos	= screenCoord;
		m_fTiltAngle_deg= 90.0f + D3DXToDegree(m_vecGridRot.x);

		m_fTiltAngle_deg = D3DXToDegree(m_vecGridRot.x);
		m_fYPan_deg = D3DXToDegree(m_vecGridRot.z);

		//CheckModelPositions();
		OnChangeCalibparams();

		HoldModelPositions_End();
	}	
}


void CCalibrationSettingCtl::MouseOver(POINT screenCoord)
{
	D3DXVECTOR3 vPickedPos;
	ObjectModel* pObjPicked = PickObject(screenCoord, &vPickedPos);

	for (int i = 0; i < m_pDataMgr->GetObjectModelCnt() ; i++){
		ObjectModel* pObj = m_pDataMgr->GetObjectModel(i);
		pObj->bHighlighted = FALSE;
	}

	if (pObjPicked){
		pObjPicked->bHighlighted = TRUE;
	}
}

void CCalibrationSettingCtl::ZoomGrid(POINT screenCoord, long zDelta)
{
	HoldModelPositions_Begin();

	if (zDelta > 0)
		m_fCameraHeight /= 1.05f;
	else if (zDelta < 0)
		m_fCameraHeight *= 1.05f;

	m_fCameraHeight = min(200.0f, max(0.2f, m_fCameraHeight));

	HoldModelPositions_End();

//	CheckModelPositions();
	OnChangeCalibparams();
}


D3DXVECTOR3 CCalibrationSettingCtl ::UnprojectXplane(CPoint screenCoord, int modelIdx, float fXPlane, float *pfDist2Plane)
{

	D3DXMATRIX matTransform;  // complete transform
	D3DXMATRIX matInvTransform;
	float fDet;
	D3DXVECTOR3 v;
	D3DXVECTOR4 unprojnear, unprojfar, direction;
	UINT w, h;
	

	if (modelIdx == -1)
		matTransform = m_matView*m_matProjModel_Clipped;
	else{
		ObjectModel* pObj = m_pDataMgr->GetObjectModel(modelIdx);
		matTransform = pObj->matObjectModel*m_matView*m_matProjModel_Clipped;
	}

	w	= m_ClientRect.right;
	h	= m_ClientRect.bottom;
	
	/// convert to logical coords
	v.x = 2.0f*(float)screenCoord.x/(float)w - 1.0f;
	v.y = -2.0f*(float)(screenCoord.y)/(float)h + 1.0f;
	v.z = 0.0f;

	/// inverse transform matrix for unprojection
	D3DXMatrixInverse(&matInvTransform, &fDet, &matTransform);

	//// near clipping plane unprojection 
	D3DXVec3Transform(&unprojnear, &v, &matInvTransform);

	unprojnear.x /= unprojnear.w;
	unprojnear.y /= unprojnear.w;
	unprojnear.z /= unprojnear.w;

	/// far clipping plane projection
	v.z = 1.0f;
	D3DXVec3Transform(&unprojfar, &v, &matInvTransform);

	unprojfar.x /= unprojfar.w;
	unprojfar.y /= unprojfar.w;
	unprojfar.z /= unprojfar.w;

	/// find ray intersection point with man plane (y = 0)
	direction = unprojfar - unprojnear;

	float t = (fXPlane-unprojnear.x)/direction.x;
	float Y = unprojnear.y + t*direction.y;
	float Z = unprojnear.z + t*direction.z;

	if (pfDist2Plane)
		*pfDist2Plane = t;

	return D3DXVECTOR3(fXPlane, Y, Z);
}


D3DXVECTOR3 CCalibrationSettingCtl::UnprojectYplane(CPoint screenCoord, int modelIdx, float fYPlane, float *pfDist2Plane)
{

	D3DXMATRIX matTransform;  // complete transform
	D3DXMATRIX matInvTransform;
	float fDet;
	D3DXVECTOR3 v;
	D3DXVECTOR4 unprojnear, unprojfar, direction;
	UINT w, h;

	if (modelIdx == -1)
		matTransform = m_matView*m_matProjModel_Clipped;
	else{
		ObjectModel* pObj = m_pDataMgr->GetObjectModel(modelIdx);
		matTransform = pObj->matObjectModel*m_matView*m_matProjModel_Clipped;
	}

	w	= m_ClientRect.right;
	h	= m_ClientRect.bottom;

	/// convert to logical coords
	v.x = 2.0f*(float)screenCoord.x/(float)w - 1.0f;
	v.y = -2.0f*(float)(screenCoord.y)/(float)h + 1.0f;
	v.z = 0.0f;

	/// inverse transform matrix for unprojection
	D3DXMatrixInverse(&matInvTransform, &fDet, &matTransform);

	//// near clipping plane unprojection 
	D3DXVec3Transform(&unprojnear, &v, &matInvTransform);

	unprojnear.x /= unprojnear.w;
	unprojnear.y /= unprojnear.w;
	unprojnear.z /= unprojnear.w;

	/// far clipping plane projection
	v.z = 1.0f;
	D3DXVec3Transform(&unprojfar, &v, &matInvTransform);

	unprojfar.x /= unprojfar.w;
	unprojfar.y /= unprojfar.w;
	unprojfar.z /= unprojfar.w;

	/// find ray intersection point with man plane (y = 0)
	direction = unprojfar - unprojnear;

	float t = (fYPlane-unprojnear.y)/direction.y;
	float X = unprojnear.x + t*direction.x;
	float Z = unprojnear.z + t*direction.z;

	if (pfDist2Plane)
		*pfDist2Plane = t;

	return D3DXVECTOR3(X, fYPlane, Z);
}


D3DXVECTOR3 CCalibrationSettingCtl::UnprojectZplane(CPoint screenCoord, int modelIdx, float fZPlane, float *pfDist2Plane)
{
	D3DXVECTOR3 v;
	UINT w, h;	

	w	= m_ClientRect.right;
	h	= m_ClientRect.bottom;

	/// convert to logical coords
	v.x = 2.0f*(float)screenCoord.x/(float)w - 1.0f;
	v.y = -2.0f*(float)(screenCoord.y)/(float)h + 1.0f;
	v.z = 0.0f;

	return UnprojectZplane(v, modelIdx, fZPlane, pfDist2Plane);
}


D3DXVECTOR3 CCalibrationSettingCtl::UnprojectZplane(D3DXVECTOR3 logicalCoord, int modelIdx, float fZPlane, float *pfDist2Plane)
{

	D3DXMATRIX matTransform;  // complete transform
	D3DXMATRIX matInvTransform;
	float fDet;
	D3DXVECTOR3 v;
	D3DXVECTOR4 unprojnear, unprojfar, direction;
	UINT w, h;

	if (modelIdx == -1)
		matTransform = m_matView*m_matProjModel_Clipped;
	else{
		ObjectModel* pObj = m_pDataMgr->GetObjectModel(modelIdx);
		matTransform = pObj->matObjectModel*m_matView*m_matProjModel_Clipped;
	}

	w	= m_ClientRect.right;
	h	= m_ClientRect.bottom;


	/// convert to logical coords
	v.x = logicalCoord.x;
	v.y = logicalCoord.y;
	v.z = 0.0f;

	/// inverse transform matrix for unprojection
	D3DXMatrixInverse(&matInvTransform, &fDet, &matTransform);

	//// near clipping plane unprojection 
	D3DXVec3Transform(&unprojnear, &v, &matInvTransform);

	unprojnear.x /= unprojnear.w;
	unprojnear.y /= unprojnear.w;
	unprojnear.z /= unprojnear.w;

	/// far clipping plane projection
	v.z = 1.0f;
	D3DXVec3Transform(&unprojfar, &v, &matInvTransform);

	unprojfar.x /= unprojfar.w;
	unprojfar.y /= unprojfar.w;
	unprojfar.z /= unprojfar.w;

	/// find ray intersection point with plane z = 0
	direction = unprojfar - unprojnear;

	float t = (fZPlane-unprojnear.z)/direction.z;
	float X = unprojnear.x + t*direction.x;
	float Y = unprojnear.y + t*direction.y;

	if (pfDist2Plane)
		*pfDist2Plane = t;

	return D3DXVECTOR3(X, Y, fZPlane);
}
