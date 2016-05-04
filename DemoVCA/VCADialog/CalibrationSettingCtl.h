#pragma once
#include "VCADataMgr.h"
#include <d3d.h>
#include <d3dx.h>
#include "VCAConfigureObserver.h"

class CVCADialog;
class CCalibrationSettingCtl : public IVCAConfigureObserver
{
public:
	CCalibrationSettingCtl(void);
	~CCalibrationSettingCtl(void);

	BOOL	Setup(CVCADialog *pVCADialog);
	void	Reset(BOOL bResetPause = TRUE);
	void	SetVideoInputSize(SIZE sz){m_csVideoSize = sz;}

	void	OnLButtonUp(UINT nFlags, POINT points);
	void	OnLButtonDown(UINT nFlags, POINT points);
	void	OnRButtonUp(UINT nFlags, POINT points);
	void	OnRButtonDown(UINT nFlags, POINT points);
	void	OnMouseMove(UINT nFlags, POINT points);
	void	OnMouseWheel(UINT nFlags, short zDelta, POINT pt);
	void	OnCommand(WPARAM wParam, LPARAM lParam);
	void	OnChangeClientRect(RECT rect) {m_ClientRect = rect; CheckModelPositions(); }

protected:
	virtual void FireOnEvent( DWORD uiEvent, DWORD uiContext );

private:
	BOOL		m_bSetup;
	CVCADialog	*m_pVCADialog;
	CVCADataMgr	*m_pDataMgr;
	HWND		m_hWnd;
	HMENU		m_hMenu;
		
	RECT		m_ClientRect;
	BOOL		m_bMouseDraging;

	//Calibration setting
	float		m_fCameraHeight;
	float		m_fHumanHeight;
	float		m_fManAnimAcc;
	float		m_fYFOV_deg;
	float		m_fYPan_deg;
	float		m_fYRoll_deg;
	float		m_fTiltAngle_deg;
	float		m_fScale_fact;
	BOOL		m_bTiltingGrid;
	POINT		m_tiltStartPos;
	SIZE		m_csVideoSize;
	BOOL		m_bMetricNotImp;
	
	void		InitCalibParam();
	//Change by edit control, Call this function to update object and drawing parameters.
	void		SetCameraParams(float fCameraHeight, float fTiltAngle_deg, float fYFOV_deg, 
								float fYPan_deg, float fYRoll_deg, float fScale_fact, BOOL bCentre);
	//Chage by SettingCtrl Mouse, Call this function to update Edit control 
	void		OnChangeCalibparams();	

	void		SetupMatricies();
	void		SetupModelMatrix(int iModelIdx);
	void		CheckModelPositions();
	void		ClipCoords(POINT &screenCoord);
	
	void		DragGrid(POINT screenCoord);
	void		PickGrid(POINT screenCoord);
	void		ZoomGrid(POINT screenCoord, long zDelta);
	void		MouseOver(POINT screenCoord);
	void		FlyAwayGrid();
	void		EndGridDrag();
	void		HoldModelPositions_Begin();
	void		HoldModelPositions_End();

	ObjectModel*	PickObject(CPoint screenCoord, D3DXVECTOR3 *pPickedPos);
	
	D3DXVECTOR3 m_vecGridRot;	
	D3DXMATRIX	m_matView;
	D3DXMATRIX	m_matProjImage;
	D3DXMATRIX	m_matProjModel;
	D3DXMATRIX	m_matProjModel_Clipped;
	D3DXMATRIX	m_matView_noTrans;
	
	DWORD		m_lastTickCount;
	BOOL		m_bUpdatingCalibParam;

	D3DXVECTOR3	UnprojectXplane(CPoint screenCoord, int modelIdx = -1, float fXPlane = 0.0f, float *pfDist2Plane = NULL);
	D3DXVECTOR3	UnprojectYplane(CPoint screenCoord, int modelIdx = -1, float fYPlane = 0.0f, float *pfDist2Plane = NULL);
	D3DXVECTOR3	UnprojectZplane(CPoint screenCoord, int modelIdx = -1, float fZPlane = 0.0f, float *pfDist2Plane = NULL);
	D3DXVECTOR3	UnprojectZplane(D3DXVECTOR3 logicalCoord, int modelIdx, float fZPlane, float *pfDist2Plane);

	void	BeginRuler( CPoint screenCoord );
	void	UpdateRuler( CPoint screenCoord );
	void	InsertObjectModel( CPoint screenCoord );
};

