#pragma once

#include "VCADataMgr.h"
#include "VCAEventSink.h"
#include "ZoneTreeCtrl.h"
#include "ZoneSettingCtl.h"
#include "CalibrationSettingCtl.h"
#include "afxwin.h"

class CEngine;
class IVCARender;
class CDemoVCADlg;

#define MAX_WINDOW_TITLE_LEN	128
class CVCADialog : public CDialog, public IVCAEventSink
{
	DECLARE_DYNAMIC(CVCADialog)

	enum VIEW_MODE
	{
		VIEW_MODE_DISABLED		= 0,
		VIEW_MODE_PREVIEW		= 1, 
		VIEW_MODE_CONFIGURE		= 2,
		VIEW_MODE_CALIBRATION	= 3,
		VIEW_MODE_TAMPER		= 4,
		VIEW_MODE_SOURCE_SELECT	= 5,	
		VIEW_MODE_MAX
	};

	
public:
	CVCADialog( int iEngId, CWnd* pParent = NULL );  
	virtual ~CVCADialog();

	enum { IDD = IDD_VCADIALOG };

	enum eSourceStatus
	{
		SS_OK,
		SS_NOTASSIGNED,
		SS_ERROR
	};
	
	void		SetSourceStatus( CVCADialog::eSourceStatus ss ) { m_eSourceStatus = ss; }
	void		SetEngine(CEngine *pEngine);
	CEngine		*GetEngine(){return m_pEngine;}
	int			GetEngId() { return m_iEngId; }
	CVCADataMgr*	GetVCADataMgr(){return &m_DataMgr;}

	BOOL	ChangeVCASourceInfo(DWORD EngId, BITMAPINFOHEADER *pbm);
	BOOL	ProcessVCAData(DWORD EngId, BYTE *pImage, BITMAPINFOHEADER *pbm, VCA5_TIMESTAMP *pTimestamp, 
		BYTE *pMetaData, int iMataDataLen, CVCAMetaLib *pVCAMetaLib);

	
	void	SplitMove(int x, int y, int cx, int cy);
	void	ApplyAllSetting(){
		ApplyZoneSetting();
		ApplyCalibInfoSetting();
		ApplyClassObjsSetting();
//		ApplyStabEnable();
		ApplyAdvancedSetting();
//		ApplyRetriggerTime();
		ApplyTamperInfo();
		ApplySceneChangeInfo();
		ApplyEngineSetting();
	}

	void	ApplyZoneSetting();
	void	ApplyCalibInfoSetting();
	void	ApplyClassObjsSetting();
//	void	ApplyStabEnable();
	void	ApplyAdvancedSetting();
//	void	ApplyRetriggerTime();
	void	ApplyTamperInfo();
	void	ApplySceneChangeInfo();
	void	ApplyEngineSetting();

	void	SetSrcID(DWORD id){m_SrcId = id;}

	void	ChangeViewMode( VIEW_MODE viewMode, BOOL bForce = FALSE );

	void	EnableBlob( BOOL bDisplay );
	void	EnableColorSignatute( BOOL bDisplay );

private:

	CDemoVCADlg *m_pSimpleVCAAppDlg;
	CEngine			*m_pEngine;
	DWORD			m_SrcId;

	IVCARender		*m_pVCARender;
	CVCADataMgr		m_DataMgr;
	
	VIEW_MODE		m_eViewMode;
	POINT			m_ptClick;
	eSourceStatus	m_eSourceStatus;
	
	
	CRect			m_ClientRect;
	CZoneSettingCtl	m_ZoneCtrl;
	CCalibrationSettingCtl	m_CalibrationCtrl;
	int				m_iEngId;
		
//	void	ChangeViewMode(DWORD modePrev, DWORD modeCurr);
	void	ShowCalibDlgCtrl(BOOL bEnable);
	void	ResizeCalibDlgCtrl();
	void	InvalidateRightRigion();

protected:
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd( CDC *pDC );
	DECLARE_MESSAGE_MAP()
	
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
	void DecideDrawTrailMode();
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};
