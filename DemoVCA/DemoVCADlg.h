// DemoVCADlg.h : header file
//

#if !defined(AFX_SIMPLEVCAAPPDLG_H__C2E4E53A_435F_46BD_AC40_891983453E6E__INCLUDED_)
#define AFX_SIMPLEVCAAPPDLG_H__C2E4E53A_435F_46BD_AC40_891983453E6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDemoVCADlg dialog
#include "APPConfigure.h"
#include "VCAConfigure.h"
#include "VCASystem.h"

#include "./Recoder/AlarmRecorder.h"
#include "StartupWnd.h"
#include "enums.h"

#include "AppDlg.h"


class CVCADialog;
class CBlankDialog;
class IVCAVideoSource;

class CDemoVCADlg : public CAppDlg
{
// Construction
public:
	CDemoVCADlg( CWnd* pParent = NULL);	// standard constructor
	~CDemoVCADlg();
	BOOL	InitInstance();

// Dialog Data
	//{{AFX_DATA(CDemoVCADlg)
	enum { IDD = IDD_SIMPLEVCAAPP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemoVCADlg)
	
	
protected:
	virtual UINT GetNumEngine() { return CAPPConfigure::Instance()->GetEngineCnt(); }
	virtual CEngine *GetEngine( int iEngId );
	virtual BOOL CreateEngine( int iEngId );
	virtual BOOL DestroyEngine( int iEngId );
	virtual VCA5_APP_VIDEOSRC_INFO *GetSrcInfo( int iEngId );
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

		
	// Generated message map functions
	//{{AFX_MSG(CDemoVCADlg)
	virtual BOOL OnInitDialog();

	afx_msg void OnMenuControlExit();
	afx_msg void OnMenuOptionRecording();
	afx_msg void OnMenuOptionSaveCurSetting();
	afx_msg LRESULT OnShowWaitWnd( WPARAM wParam, LPARAM lParam );

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CVCASystem		m_VCASystem;
	CAlarmRecorder	m_AlarmRecorder;
	CWinThread		*m_pStartupThread;


	BOOL		Setup();
	void		Endup();
	BOOL		SaveCurSetting();
	BOOL		SaveEngSetting( int iEngId, LPTSTR lpszFilename = NULL );

	void	UpdateMenuControl(BOOL bStart);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLEVCAAPPDLG_H__C2E4E53A_435F_46BD_AC40_891983453E6E__INCLUDED_)
