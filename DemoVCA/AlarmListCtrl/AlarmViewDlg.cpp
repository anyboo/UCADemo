// AlarmViewrDlg.cpp : implementation file
//
#include "stdafx.h"
#include "../resource.h"
#include "AlarmViewDlg.h"
#include "colorconversion.h"
#include "../Render/CD3d.h"
#include "../common/APPConfigure.h"

// CAlarmViewDlg dialog

#define DIR_SNAPSHOTS_A	"snapshots"
#define DIR_SNAPSHOTS_T	_T("snapshots")

static LPCSTR g_strFilterItem[VCA5_RULE_TYPE_NUM-1] =
{
	"Presence",
	"Enter",
	"Exit",
	"Appear",
	"Disappear",
	"Stopped",
	"Dwell",
	"Direction",
	"Speed",
	"Area",
	"Tailgating",
	"Abandone/Remove",
	"Linecounter_A",
	"Linecounter_B",
	"Abandone/Remove",
	"Colour Signiture",
	"Smoke",
	"Fire",
};

static void CleanDirectory(TCHAR* szPath);

IMPLEMENT_DYNAMIC(CAlarmViewDlg, CDialog)

CAlarmViewDlg::CAlarmViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmViewDlg::IDD, pParent)
{
	m_pD3d	= NULL;
	m_nCurImageIdx = 0;
	m_pBuffer = NULL;
	m_dwBufferLength = 0;
	m_pImageBuf	= (BYTE *)_aligned_malloc(MAXWIDTH_D3DCANVAS * MAXHEIGHT_D3DCANVAS * 4, 16);
}

CAlarmViewDlg::~CAlarmViewDlg()
{
	if(m_pD3d){
		delete m_pD3d;
		m_pD3d = NULL;
	}
	if (m_dwBufferLength) {
		_aligned_free(m_pBuffer);
		m_pBuffer = NULL;
		m_dwBufferLength = 0;
	}
	if (m_pImageBuf) {
		_aligned_free(m_pImageBuf);
		m_pImageBuf = NULL;
	}
}

void CAlarmViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_PREV, m_btnPrev);
	DDX_Control(pDX, IDC_BTN_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_ED_FILENAME, m_edFileName);
	DDX_Control(pDX, IDC_PROGRESS_EXPORT, m_ctrlExportProgress);
	DDX_Control(pDX, IDC_BTN_EXPORT, m_btnExport);
}


BEGIN_MESSAGE_MAP(CAlarmViewDlg, CDialog)
//	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_PREV, &CAlarmViewDlg::OnBnClickedBtnPrev)
	ON_BN_CLICKED(IDC_BTN_NEXT, &CAlarmViewDlg::OnBnClickedBtnNext)
	ON_BN_CLICKED(IDC_BTN_EXPORT, &CAlarmViewDlg::OnBnClickedBtnExport)
END_MESSAGE_MAP()


// CAlarmViewDlg message handlers

INT_PTR CAlarmViewDlg::DoModal(LPCTSTR pStartTime, LPCTSTR pEndTime, DWORD dwPeriod, LPCTSTR pEngId, LPCTSTR pAlarmId, LPCTSTR pRootPath)
{
	if( !Init(pStartTime, pEndTime, dwPeriod, pEngId, pAlarmId, pRootPath) ) return -1;
	return CDialog::DoModal();
}

BOOL CAlarmViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	const CString &Path = m_arFileNames.GetAt(0);
	Jpeg jpeg;

	if (!m_JpegCodec.GetJpegInformation((LPCTSTR)Path, &jpeg)) {
		return FALSE;
	}

	m_pD3d = new CD3d();
	if(m_pD3d->Setup(0, CD3d::CF_BGR565, NULL, jpeg.bm.biWidth, jpeg.bm.biHeight, jpeg.bm.biWidth, jpeg.bm.biHeight) != S_OK){
		MessageBox(_T("Can not setup D3D "),_T("ERROR"),MB_OK);
		delete m_pD3d;
		m_pD3d = NULL;
		return FALSE;
	}
	m_pD3d->SetDDClipperWindow(m_hWnd);

	ShowUI();
	ShowCurrentImage();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAlarmViewDlg::OnPaint()
{
	CPaintDC dc(this);
	
	RECT ScreenRect;

	ScreenRect = m_ClientRect;
	ClientToScreen((POINT*)&ScreenRect.left );
	ClientToScreen((POINT*)&ScreenRect.right );

	m_pD3d->PrimaryPresentation(&ScreenRect, &m_ClientRect);
}

void CAlarmViewDlg::OnBnClickedBtnPrev()
{
	if(m_nCurImageIdx > 0) {
		m_nCurImageIdx--;
	}

	ShowCurrentImage();
}

void CAlarmViewDlg::OnBnClickedBtnNext()
{
	if(m_nCurImageIdx < m_arFileNames.GetCount()-1) {
		m_nCurImageIdx++;
	}

	ShowCurrentImage();
}

BOOL CAlarmViewDlg::Init(LPCTSTR pStartTime, LPCTSTR pEndTime, DWORD dwPeriod, LPCTSTR pEngId, LPCTSTR pAlarmId, LPCTSTR pRootPath)
{
	m_strStartTime	= pStartTime;
	m_strEndTime	= pEndTime;
	m_strEngId		= pEngId;
	m_strAlarmId	= pAlarmId;
	m_strRootPath	= pRootPath;

	if (m_strRootPath.IsEmpty()) {
		return FALSE;
	}

	MakeFolderNameString(m_strStartTime);
	MakeFolderNameString(m_strEndTime);

	if (0 == FindJpegFileNames(pRootPath, (LPCTSTR) m_strStartTime, (LPCTSTR) m_strEndTime, dwPeriod)) {
		return FALSE;
	}
	return TRUE;
}

DWORD CAlarmViewDlg::FindJpegFileNamesInSubFolder(LPCTSTR pRootPath, LPCTSTR pSubFolderName, LPCTSTR pEngID, LPCTSTR pAlarmID)
{
	CFileFind Finder;
	CString Filter;
	CString Path, FileName;
	BOOL bCont;	
	DWORD uNumFound;

	Filter.Format(_T("%03s_%03s_"), pEngID, pAlarmID);
	Path.Format(_T("%s\\%s\\*.jpg"), pRootPath, pSubFolderName);
	uNumFound = 0;

	Finder.FindFile(Path);
	do {
		bCont = Finder.FindNextFile();

		FileName = Finder.GetFileName();
		if (FileName.Find(Filter) != -1) {
			CString FilePath;
			FilePath.Format(_T("%s\\%s\\%s"), pRootPath, pSubFolderName, (LPCTSTR) FileName);
			m_arFileNames.Add(FilePath);
			uNumFound++;
		}
	} while (bCont);

	return uNumFound;
}

DWORD CAlarmViewDlg::FindJpegFileNames(LPCTSTR pRootPath, CString strStartTime, CString strEndTime, DWORD dwPeriod)
{
	CFileFind Finder;
	DWORD uNumFound = 0;
	BOOL bCont;
	
	CString RootPath;
	RootPath.Format(_T("%s\\*.*"), pRootPath);
	bCont = Finder.FindFile(RootPath);

	if (bCont) {
		CString strFolderTime, FolderName;
	
		strStartTime = strStartTime.Left(13);
		strEndTime = strStartTime.Left(13);
		strStartTime.Remove(_T('_'));
		strEndTime.Remove(_T('_'));
		__int64 startTime = _ttoi64(strStartTime);
		__int64 endTime = _ttoi64(strEndTime);
		__int64 prevFolder=0, curFolder ;

		// Find correct folder
		do {
			bCont = Finder.FindNextFile();

			if (Finder.IsDirectory()) {
				strFolderTime = Finder.GetFileName();
				if (13 == strFolderTime.GetLength() && '_' == strFolderTime.GetAt(8)) { // YYYYMMDD_HHMMSS
					
					strFolderTime.Remove(_T('_'));
					curFolder = _ttoi64(strFolderTime);
					if ( (startTime <= curFolder) && (curFolder <= endTime) ) {
						FolderName = Finder.GetFileName();
						uNumFound += FindJpegFileNamesInSubFolder(pRootPath, FolderName, m_strEngId, m_strAlarmId);
					}
				}
			}
			
		} while (bCont);

		//if (FolderName == _T("..") || FolderName == _T("")) return 0;
	}

	Finder.Close();
	return uNumFound;
}

void CAlarmViewDlg::SetNaviButtonDisable()
{
	if (m_arFileNames.GetCount() <= 0) return;

	if(m_nCurImageIdx == 0){
		m_btnPrev.EnableWindow(FALSE);
		m_btnNext.EnableWindow(TRUE);
	}else if(m_nCurImageIdx == m_arFileNames.GetCount()-1){
		m_btnPrev.EnableWindow(TRUE);
		m_btnNext.EnableWindow(FALSE);
	}else{
		m_btnPrev.EnableWindow(TRUE);
		m_btnNext.EnableWindow(TRUE);
	}
}

void CAlarmViewDlg::AdjustDlgRect(int width, int height)
{
	RECT rt;
	CRect rtProgress, rtExportBtn;
	GetWindowRect(&rt);
	m_ctrlExportProgress.GetWindowRect(&rtProgress);
	m_btnExport.GetWindowRect(&rtExportBtn);

	MoveWindow(rt.left, rt.top, width+8, height+100);		// 100 pixel for navigator control
}

void CAlarmViewDlg::RelocateControls(int width, int height)
{
	RECT rtEdit,rtBtn1, rtBtn2;
	m_edFileName.GetWindowRect(&rtEdit);
	int w = rtEdit.right - rtEdit.left;
	int h = rtEdit.bottom - rtEdit.top;
	rtEdit.left = (width/2) - (w)/2;
	rtEdit.top  = height + 10;
	rtEdit.right = rtEdit.left + w;
	rtEdit.bottom = rtEdit.top + h;
	m_edFileName.MoveWindow(&rtEdit);

	m_btnPrev.GetWindowRect(&rtBtn1);
	w = rtBtn1.right - rtBtn1.left;
	h = rtBtn1.bottom - rtBtn1.top;
	rtBtn1.left = rtEdit.left - 5 - w;
	rtBtn1.top = rtEdit.top;
	rtBtn1.right = rtBtn1.left + w;
	rtBtn1.bottom = rtBtn1.top + h;
	m_btnPrev.MoveWindow(&rtBtn1);

	m_btnNext.GetWindowRect(&rtBtn2);
	w = rtBtn2.right - rtBtn2.left;
	h = rtBtn2.bottom - rtBtn2.top;
	rtBtn2.left = rtEdit.right + 5;
	rtBtn2.top = rtEdit.top;
	rtBtn2.right = rtBtn2.left + w;
	rtBtn2.bottom = rtBtn2.top + h;
	m_btnNext.MoveWindow(&rtBtn2);
}


void CAlarmViewDlg::DrawImage(int width, int height, BYTE* pImage, BITMAPINFOHEADER bm)
{
	
	SIZE	sourceSize = {width, height};

	const CString &filePathName = m_arFileNames.GetAt(m_nCurImageIdx);
	CString fineName = filePathName.Right(IMAGE_NAME_LEN);

	m_edFileName.SetWindowText(fineName);

	RECT rcROI = {0,0,sourceSize.cx, sourceSize.cy};

	m_pD3d->DrawImage(m_ClientRect, sourceSize, CD3d::CF_YUY2, CD3d::RT_000, rcROI, pImage);
}


void CAlarmViewDlg::ShowCurrentImage()
{
	const CString &Path = m_arFileNames.GetAt(m_nCurImageIdx);
	Jpeg jpeg;
	ZeroMemory(&jpeg, sizeof(jpeg));

	if (!m_JpegCodec.GetJpegInformation((LPCTSTR) Path, &jpeg)) return;
	if (m_dwBufferLength < jpeg.dwLength) {
		if (m_pBuffer) _aligned_free(m_pBuffer);
		m_pBuffer = (BYTE *) _aligned_malloc(jpeg.dwLength, 16);
		m_dwBufferLength = jpeg.dwLength;
	}

	jpeg.pBuffer = (BYTE *) m_pBuffer;
	if (!m_JpegCodec.Decode((LPCTSTR) Path, &jpeg)) {
		return;
	}

	int width = (jpeg.bm.biWidth<= 360)?(2*jpeg.bm.biWidth):(jpeg.bm.biWidth);
	int height = (jpeg.bm.biHeight<= 288)?(2*jpeg.bm.biHeight):(jpeg.bm.biHeight);

	DWORD dwEngId = _ttoi(m_strEngId.GetBuffer());

	CRect	rcVCAROI;
	VCA5_RECT	rcROIEng = CAPPConfigure::Instance()->GetAPPEngineInfo( dwEngId )->tSourceData.rcROI;

	rcVCAROI.left	= rcROIEng.x;
	rcVCAROI.top	= rcROIEng.y;
	rcVCAROI.right	= rcROIEng.x + rcROIEng.w;
	rcVCAROI.bottom	= rcROIEng.y + rcROIEng.h;

	if( rcVCAROI.Size() == CSize(0, 0) ) {
		rcVCAROI.CopyRect(&m_ClientRect);
	} else {
		rcVCAROI.right = min(rcVCAROI.right-1, width);
		rcVCAROI.bottom = min(rcVCAROI.bottom-1, height);
		PIXELTOPERCENT(rcVCAROI.left, rcVCAROI.left, width);
		PIXELTOPERCENT(rcVCAROI.top, rcVCAROI.top, height);
		PIXELTOPERCENT(rcVCAROI.right, rcVCAROI.right, width);
		PIXELTOPERCENT(rcVCAROI.bottom, rcVCAROI.bottom, height);

		PERCENTTOPIXEL(rcVCAROI.left, rcVCAROI.left, m_ClientRect.right);
		PERCENTTOPIXEL(rcVCAROI.top, rcVCAROI.top, m_ClientRect.bottom);
		PERCENTTOPIXEL(rcVCAROI.right, rcVCAROI.right, m_ClientRect.right);
		PERCENTTOPIXEL(rcVCAROI.bottom, rcVCAROI.bottom, m_ClientRect.bottom);
	}

	m_ClientRect.left = 0;
	m_ClientRect.top = 0;
	m_ClientRect.right = width;
	m_ClientRect.bottom = height;

	AdjustDlgRect(width, height);
	RelocateControls(width, height);
	SetNaviButtonDisable();
	
	DrawImage(jpeg.bm.biWidth, jpeg.bm.biHeight, jpeg.pImage, jpeg.bm);
	DrawSingleZone(rcVCAROI, jpeg.pZone);
	DrawObject(rcVCAROI, jpeg.bm.biWidth, jpeg.bm.biHeight, jpeg.pObject);
	if(jpeg.pRule) DrawTimeStampAndRuleType(jpeg.bm.biWidth, jpeg.bm.biHeight, jpeg.i64TimeStamp, jpeg.pRule->usRuleType);

	
	RECT	ScreenRect;
	ScreenRect = m_ClientRect;

	ClientToScreen((POINT*)&ScreenRect.left );
	ClientToScreen((POINT*)&ScreenRect.right );

	m_pD3d->PrimaryPresentation(&ScreenRect, &m_ClientRect);
}

BOOL CAlarmViewDlg::ExportSnapShots(LPCTSTR pStartTime, LPCTSTR pEndTime, DWORD dwPeriod, LPCTSTR pEngId, LPCTSTR pAlarmId, LPCTSTR pRootPath)
{
	if(!Init(pStartTime, pEndTime, dwPeriod, pEngId, pAlarmId, pRootPath) ) return FALSE;

	// Make avi with the JPEGs
	CString strPath = m_arFileNames.GetAt(0);
	int strCnt = strPath.GetLength() - m_strRootPath.GetLength() - 1;
	strPath = strPath.Right(strCnt);
	strCnt = strPath.GetLength() - 9;
	strPath = strPath.Left(strCnt);
	strPath.Replace(_T('\\'), _T('-'));
	strPath += _T(".avi");

	USES_CONVERSION;
	char szCmd[MAX_PATH] = {0};

	CString strPathPrefix = m_arFileNames.GetAt(0);
	strCnt = strPathPrefix.GetLength() - 8;
	strPathPrefix = strPathPrefix.Left(strCnt);

	int qp = 10; // Video quality. 1~50. Lower value is better quality.
	double fps = 10;
	int numSnapshots = m_arFileNames.GetCount();
	if(numSnapshots < 50) {
		fps = (double)numSnapshots / 5.0;
	}

// Dump file list to text file
#if 0
	FILE* fp = fopen("filelist.txt", "wt");
	for(int i=0; i<m_arFileNames.GetCount(); i++) {
		CString strPicFile = m_arFileNames.GetAt(i);
		fprintf(fp, "%s\n", T2A(strPicFile.GetBuffer()));
	}
	fclose(fp);
#endif

	sprintf(szCmd, ".\\ffmpeg.exe -r %1.1lf -i %s%%04d.jpg -vf vflip -q %d %s", fps, T2A(strPathPrefix.GetBuffer()), qp, T2A(strPath.GetBuffer()));
	system(szCmd);

	CString option;
	option.Format(_T("/select,%s"), strPath);

	m_strAviFileName = strPath;

 //	ShellExecute(NULL, _T("open"), _T("explorer"), option, NULL, SW_SHOW);

	return TRUE;
}

void CAlarmViewDlg::DrawSingleZone(RECT rcVCAROI, VCA5_APP_ZONE *pZone)
{
	if(!pZone) return;

	CD3d::MYPOLYGON	Polygon;
	BYTE alpha = 64;
	SIZE Size = {m_ClientRect.right, m_ClientRect.bottom};
	CD3d::eBRUSHTYPE eBrushType = (pZone->usZoneType == VCA5_ZONE_TYPE_NONDETECTION) ? CD3d::BRT_HATCH : CD3d::BRT_SOLID ;

	if (pZone->uiStatus & VCA5_APP_AREA_STATUS_NOTIFIED ||
		pZone->uiStatus & VCA5_APP_AREA_STATUS_SELECTED ) {
			alpha = 128;
	}

	CRect rcVCAROI1(rcVCAROI);

	if( rcVCAROI1.Size() == CSize(0, 0) ) {
		rcVCAROI1.CopyRect(&m_ClientRect);
	}

	Polygon.cnt_point = pZone->ulTotalPoints;
	for (ULONG i = 0; i < Polygon.cnt_point; ++i) {
		PERCENTTOPIXEL(Polygon.atpoint[i].x, pZone->pPoints[i].x, rcVCAROI1.Width());
		PERCENTTOPIXEL(Polygon.atpoint[i].y, pZone->pPoints[i].y, rcVCAROI1.Height());
		Polygon.atpoint[i].x += rcVCAROI1.left;
		Polygon.atpoint[i].y += rcVCAROI1.top;
	}

	// draw zone
	if (VCA5_ZONE_STYLE_POLYGON == pZone->usZoneStyle) {
		Polygon.cnt_point = pZone->ulTotalPoints - 1;
		m_pD3d->DrawPolygonList(m_ClientRect, Size, &Polygon, 1, eBrushType, alpha, pZone->uiColor);

	} else if (VCA5_ZONE_STYLE_TRIPWIRE == pZone->usZoneStyle) {
		for (ULONG i = 0; i < Polygon.cnt_point-1; ++i) {
			CD3d::MYPOLYGON TripWire;	
			int x1, y1, x2, y2;

			x1 = Polygon.atpoint[i].x;
			y1 = Polygon.atpoint[i].y;
			x2 = Polygon.atpoint[i+1].x;
			y2 = Polygon.atpoint[i+1].y;

			float fTheta = atan2( (float)(y2-y1), (float)(x2-x1) );
			float fx = sin(fTheta);
			float fy = cos(fTheta);
			float fxOff, fyOff;
			fxOff = 2 * fx;
			fyOff = 2 * fy;

			TripWire.cnt_point = 4;
			TripWire.atpoint[0].x = (LONG) (x1 - fxOff);
			TripWire.atpoint[0].y = (LONG) (y1 + fyOff);
			TripWire.atpoint[1].x = (LONG) (x1 + fxOff);
			TripWire.atpoint[1].y = (LONG) (y1 - fyOff);
			TripWire.atpoint[2].x = (LONG) (x2 + fxOff);
			TripWire.atpoint[2].y = (LONG) (y2 - fyOff);
			TripWire.atpoint[3].x = (LONG) (x2 - fxOff);
			TripWire.atpoint[3].y = (LONG) (y2 + fyOff);

			m_pD3d->DrawPolygonList(m_ClientRect, Size, &TripWire, 1, CD3d::BRT_SOLID, alpha, pZone->uiColor);
		}

	} 

	if (pZone->uiStatus & VCA5_APP_AREA_STATUS_SELECTED) {

		for (ULONG i = 0; i < Polygon.cnt_point; ++i)	{
			m_pD3d->DrawCircle(m_ClientRect, Size, Polygon.atpoint[i], 6, 1, alpha, pZone->uiColor);
			m_pD3d->DrawCircle(m_ClientRect, Size, Polygon.atpoint[i], 4, 1, alpha, RGB(255,255,255) );
		}
	}
}


void CAlarmViewDlg::DrawObject(RECT rcCanvas, LONG width, LONG height, VCA5_PACKET_OBJECT *pObject)
{
	if(!pObject) return;

	COLORREF Color = RGB(0xFF, 0, 0);
	SIZE	SourceSize = {width, height};

	DrawBoundinxBox(m_ClientRect, SourceSize, rcCanvas, pObject->bBox, 0xFF, Color);
	
//	if((0 == pObject->ulCalibArea)&& (0 == pObject->ulCalibSpeed)) return;
//	DrawObjectInformation(m_ClientRect, SourceSize, pObject, 0xFF, Color);
}


void CAlarmViewDlg::DrawTimeStampAndRuleType(LONG width, LONG height, __int64 i64TimeStamp, int iruleType)
{
	char Text[128];
	SIZE size = {width, height};
	POINT pt = {0, 0};

	CTime cTime = CTime( (time_t) i64TimeStamp );
	struct tm	osTime;
	cTime.GetLocalTm( &osTime );


	sprintf_s(Text, _countof(Text), "%4d/%02d/%02d %02d:%02d:%02d : RuleType [%s]",
		osTime.tm_year+1900, osTime.tm_mon+1, osTime.tm_mday, osTime.tm_hour, osTime.tm_min, osTime.tm_sec, g_strFilterItem[iruleType-1]);


	m_pD3d->DrawText(m_ClientRect, size, pt, &Text[0], 128, RGB(0, 255, 0));
}


void CAlarmViewDlg::MakeFolderNameString(CString &date)
{	
	if(date.IsEmpty()) return;
	CString tempStr(date);
	CStringArray arrDate;
	int curPos = 0;
	CString resToken= tempStr.Tokenize(_T("- :"),curPos);
	while (resToken != _T(""))
	{
		arrDate.Add(resToken);
		resToken = tempStr.Tokenize(_T("- :"), curPos);
	};   

	date.Empty();
	for(int i=0; i<6; i++){
		if(i==3) date += _T("_");
		date += arrDate.GetAt(i);
	}
}


void CAlarmViewDlg::DrawBoundinxBox(RECT clientRect, SIZE SourceSize, RECT rcVCAROI, VCA5_RECT	bBox, BYTE alpha, COLORREF color)
{
	CD3d::MYPOLYGON	Bound[4];
	LONG Xmin;
	LONG Ymin;
	LONG Xmax;
	LONG Ymax;

	CRect rcVCAROI1(rcVCAROI);
	if( rcVCAROI1.Size() == CSize(0, 0) ) {
		rcVCAROI1.SetRect(0, 0, SourceSize.cx, SourceSize.cy);
	}

	PERCENTTOPIXEL( Xmin, (bBox.x - bBox.w/2), rcVCAROI1.Width());
	PERCENTTOPIXEL( Ymin, (bBox.y - bBox.h/2), rcVCAROI1.Height());
	PERCENTTOPIXEL( Xmax, (bBox.x + bBox.w/2), rcVCAROI1.Width());
	PERCENTTOPIXEL( Ymax, (bBox.y + bBox.h/2), rcVCAROI1.Height());

	Xmin = max(0, Xmin);
	Ymin = max(0, Ymin);
	Xmax = min(rcVCAROI1.Width()-1, Xmax);
	Ymax = min(rcVCAROI1.Height()-1, Ymax);

	Xmin += rcVCAROI1.left;
	Ymin += rcVCAROI1.top;
	Xmax += rcVCAROI1.left;
	Ymax += rcVCAROI1.top;

	Bound[0].atpoint[0].x  = Xmin;
	Bound[0].atpoint[0].y  = Ymin;
	Bound[0].atpoint[1].x  = Xmax;
	Bound[0].atpoint[1].y  = Ymin;
	Bound[1].atpoint[0].x  = Xmax;
	Bound[1].atpoint[0].y  = Ymin;
	Bound[1].atpoint[1].x  = Xmax;
	Bound[1].atpoint[1].y  = Ymax;
	Bound[2].atpoint[0].x  = Xmin;
	Bound[2].atpoint[0].y  = Ymax;
	Bound[2].atpoint[1].x  = Xmax;
	Bound[2].atpoint[1].y  = Ymax;
	Bound[3].atpoint[0].x  = Xmin;
	Bound[3].atpoint[0].y  = Ymin;
	Bound[3].atpoint[1].x  = Xmin;
	Bound[3].atpoint[1].y  = Ymax;

	for (DWORD k = 0; k < 4; ++k){
		m_pD3d->DrawLineList(clientRect, SourceSize, &Bound[k].atpoint[0], 2, 0xFF, color);
	}
}

static void	GetTextSFBelowHalfD1Height(int nScaledHeight, float &fScalor, UINT &uiFontSize, UINT &uiFontSizeRemain)
{
	switch(nScaledHeight)
	{
	case 1:
		fScalor = 0.5f;
		uiFontSize = 10;
		uiFontSizeRemain = 4;
		break;
	case 2:
		fScalor = 0.55f;
		uiFontSize = 10;
		uiFontSizeRemain = 3;
		break;
	case 3:
		fScalor = 0.6f;
		uiFontSize = 7;
		uiFontSizeRemain = 3;
		break;
	case 4:
		fScalor = 0.65f;
		uiFontSize = 7;
		uiFontSizeRemain = 3;
		break;
	case 5:
		fScalor = 0.7f;
		uiFontSize = 6;
		uiFontSizeRemain = 2;
		break;
	case 6:
		fScalor = 0.75f;
		uiFontSize = 6;
		uiFontSizeRemain = 2;
		break;
	case 7:
		fScalor = 0.8f;
		uiFontSize = 6;
		uiFontSizeRemain = 2;
		break;
	case 8:
		fScalor = 0.85f;
		uiFontSize = 5;
		uiFontSizeRemain = 1;
		break;
	case 9:
		fScalor = 0.9f;
		uiFontSize = 4;
		uiFontSizeRemain = 1;
		break;
	}
}

static void	GetTextSFAboveHalfD1Height(int nScaledHeight, float &fScalor, UINT &uiFontSize, UINT &uiFontSizeRemain)
{
	switch(nScaledHeight)
	{
	case 1:
		fScalor = 0.5f;
		uiFontSize = 30;
		uiFontSizeRemain = 10;
		break;
	case 2:
		fScalor = 0.55f;
		uiFontSize = 20;
		uiFontSizeRemain = 7;
		break;
	case 3:
		fScalor = 0.6f;
		uiFontSize = 20;
		uiFontSizeRemain = 7;
		break;
	case 4:
		fScalor = 0.65f;
		uiFontSize = 15;
		uiFontSizeRemain = 5;
		break;
	case 5:
		fScalor = 0.7f;
		uiFontSize = 15;
		uiFontSizeRemain = 5;
		break;
	case 6:
		fScalor = 0.75f;
		uiFontSize = 13;
		uiFontSizeRemain = 5;
		break;
	case 7:
		fScalor = 0.8f;
		uiFontSize = 13;
		uiFontSizeRemain = 5;
		break;
	case 8:
		fScalor = 0.85f;
		uiFontSize = 13;
		uiFontSizeRemain = 3;
		break;
	case 9:
		fScalor = 0.9f;
		uiFontSize = 10;
		uiFontSizeRemain = 3;
		break;
	}
}


static BOOL	GetTextScalingFactor(int height, SIZE SourceSize, float &fScalor, UINT &uiFontSize, UINT &uiFontSizeRemain)
{
	if( height<120 || height>920 ) {
		fScalor = 0.5f;
		uiFontSize = 40;
		uiFontSizeRemain = 10;
		return FALSE; // unsupported image height
	}

	int nScaledHeight = (height-20)/100;

	if(SourceSize.cy <= 288) {
		GetTextSFBelowHalfD1Height(nScaledHeight, fScalor, uiFontSize, uiFontSizeRemain);
	}
	else {
		GetTextSFAboveHalfD1Height(nScaledHeight, fScalor, uiFontSize, uiFontSizeRemain);
	}

	return TRUE;
}

void	CAlarmViewDlg::DrawObjectInformation(RECT clientRect, SIZE SourceSize, VCA5_PACKET_OBJECT *pObject, BYTE alpha, COLORREF Color)
{
	char	string[1024] = {0,};
	
	int height = CRect(&clientRect).Height();
	float	fScalor = 0.0f;
	UINT uiFontSize = 0;
	UINT uiFontSizeRemain = 0;	

	GetTextScalingFactor(height, SourceSize, fScalor, uiFontSize, uiFontSizeRemain);

	LONG Xmin, Ymin;
	
	PERCENTTOPIXEL( Xmin, (pObject->bBox.x - pObject->bBox.w/2), SourceSize.cx);
	PERCENTTOPIXEL( Ymin, (pObject->bBox.y - pObject->bBox.h/2), SourceSize.cy);
	
	Xmin = max(0, Xmin);
	Ymin = max(0, Ymin);
	
	POINT	ptStartXY = {Xmin, Ymin};

	ptStartXY.x -= (LONG)(fScalor * 4.0f);
	ptStartXY.y -= uiFontSize + uiFontSizeRemain;

	int nCntInfoNumToShow = 2;

	int topPntObjMsg = ptStartXY.y - (nCntInfoNumToShow*uiFontSize);
	if(topPntObjMsg <= 0) {
		POINT	ptOrgStartXY = {Xmin, Ymin};
		ptStartXY = ptOrgStartXY;
		ptStartXY.y += nCntInfoNumToShow*uiFontSize;
	}

	sprintf_s( string, _countof(string), "%1.1f m", (float)pObject->ulCalibHeight/10.0f );	
	m_pD3d->DrawText( clientRect, SourceSize, ptStartXY, string, alpha, Color,fScalor);
	ptStartXY.y -= uiFontSize;
	
	sprintf_s( string, _countof(string), "%d  km/h", (int)pObject->ulCalibSpeed);
	m_pD3d->DrawText( clientRect, SourceSize, ptStartXY, string, alpha, Color,fScalor);
	ptStartXY.y -= uiFontSize;

	sprintf_s( string, _countof(string), "%.1f  sqm", (int)pObject->ulCalibArea/10.0f);
	m_pD3d->DrawText( clientRect, SourceSize, ptStartXY, string, alpha, Color,fScalor);
}

/*
void	CVCAD3DRender::DrawTrail(VCA5_PACKET_OBJECT *pObject, BYTE alpha, COLORREF Color)
{
	CD3d::MYPOLYGON ptTrailLineList[VCA5_MAX_NUM_TRAIL_POINTS+1];	//-- Trail

	unsigned int uiTrailX[ VCA5_MAX_NUM_TRAIL_POINTS + 1];	
	unsigned int uiTrailY[ VCA5_MAX_NUM_TRAIL_POINTS + 1];	
	int i;

	LONG Xmin, Xmax, Ymax;
	
	PERCENTTOPIXEL( Xmin, (pObject->bBox.x - pObject->bBox.w/2), m_SourceSize.cx);
	PERCENTTOPIXEL( Xmax, (pObject->bBox.x + pObject->bBox.w/2), m_SourceSize.cx);
	PERCENTTOPIXEL( Ymax, (pObject->bBox.y + pObject->bBox.h/2), m_SourceSize.cy);

	Xmin = max(0, Xmin);
	Xmax = min(m_SourceSize.cx-1, Xmax);
	Ymax = min(m_SourceSize.cy-1, Ymax);

	POINT	ptStartXY = {(Xmin + Xmax) / 2, Ymax};

	for( i = 0; i < pObject->trail.usNumTrailPoints; i++ )
	{
		PERCENTTOPIXEL( uiTrailX[i], pObject->trail.trailPoints[i].x, m_SourceSize.cx );
		PERCENTTOPIXEL( uiTrailY[i], pObject->trail.trailPoints[i].y, m_SourceSize.cy );
	}

	//	Start point of the trail
	uiTrailX[pObject->trail.usNumTrailPoints] = ptStartXY.x;
	uiTrailY[pObject->trail.usNumTrailPoints] = ptStartXY.y;
	
	//	Construct all the lineList (No. = MAX_NUM_TRAIL_POINTS - 1)
	for ( i = 0; i < pObject->trail.usNumTrailPoints; i++ )
	{
		ptTrailLineList[i].atpoint[0].x = uiTrailX[i];
		ptTrailLineList[i].atpoint[0].y = uiTrailY[i];
		ptTrailLineList[i].atpoint[1].x = uiTrailX[i + 1];
		ptTrailLineList[i].atpoint[1].y = uiTrailY[i + 1];
	}
	
	//	Draw the trails
	for ( i = 0; i < pObject->trail.usNumTrailPoints; i++ )
	{
		m_pD3d->m_DrawLineList( m_ClientRect, m_SourceSize, ptTrailLineList[i].atpoint, 2, alpha, Color );
	}

}
*/


BOOL CAlarmViewDlg::SaveSnapShot(CString strPath, DWORD dwEngineID, DWORD dwAlarmID, DWORD idx,
								 BYTE *pImage, BITMAPINFOHEADER *pbm, __int64 iTimeStamp)
{
	CString savePath;
	savePath.Format(_T("%s\\%04d.JPG"), strPath, idx);
	
	if (!m_JpegCodec.Encode(savePath.GetBuffer(), pImage, pbm, 0l, NULL, NULL, NULL)) {
		ASSERT(TRUE);
		return FALSE;
	}
	return TRUE;
}


void CAlarmViewDlg::ShowUI()
{
	m_edFileName.ShowWindow(SW_SHOW);
	m_btnPrev.ShowWindow(SW_SHOW);
	m_btnNext.ShowWindow(SW_SHOW);
	m_ctrlExportProgress.ShowWindow(SW_HIDE);
	m_btnExport.ShowWindow(SW_HIDE);
}


static void CleanDirectory(TCHAR* szPath)
{
	TCHAR fileFound[MAX_PATH];
	WIN32_FIND_DATA info;
	HANDLE hp; 
	_stprintf(fileFound, _T("%s\\*.*"), szPath);
	hp = FindFirstFile(fileFound, &info);
	do
	{
		if (!((_tcscmp(info.cFileName, _T("."))==0)||
			(_tcscmp(info.cFileName, _T(".."))==0)))
		{
			if((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==
				FILE_ATTRIBUTE_DIRECTORY)
			{
				TCHAR szTemp[MAX_PATH];
				_stprintf(szTemp, _T("%s\\%s"), szPath, info.cFileName);
				CleanDirectory(szTemp);
				//RemoveDirectory(szTemp);
			}
			else
			{
				_stprintf(fileFound, _T("%s\\%s"), szPath, info.cFileName);
				BOOL retVal = DeleteFile(fileFound);
			}
		}

	}while(FindNextFile(hp, &info));
	//RemoveDirectory(szPath);
	FindClose(hp);
}

void CAlarmViewDlg::OnBnClickedBtnExport()
{
	CreateDirectory(DIR_SNAPSHOTS_T, NULL);
	CleanDirectory(DIR_SNAPSHOTS_T);
	//ExportSnapShots();
}
