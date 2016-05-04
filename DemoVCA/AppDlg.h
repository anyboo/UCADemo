#pragma once

#include "./VCADialog/ConfigDlg.h"
#include "./VideoSource/CAP5System.h"
#include "./AlarmListCtrl/AlarmListCtrl.h"

// CAppDlg dialog

class CVCADialog;
class CBlankDialog;
class CEngine;
class IVCAVideoSource;

class CAppDlg : public CDialog
{
	DECLARE_DYNAMIC(CAppDlg)

public:
	CAppDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAppDlg();

// Dialog Data
	enum { IDD = IDD_APPDLG };

protected:


	enum DISPLAY_MODE
	{
		DISPLAY_PREVIEW,
		DISPLAY_CONFIG
	};

	// Implementation

	void WindowLayout( int cx, int cy );

	SIZE			m_SplitConfig;
	CVCADialog		*m_pVCADialogs[VCA5_MAX_NUM_ENGINE];
	CBlankDialog	*m_pBlankDialogs[VCA5_MAX_NUM_ENGINE];
	IVCAVideoSource	*m_pVideoSources[VCA5_MAX_NUM_ENGINE];
	AlarmListCtrl	m_AlarmListCtrl;

	DISPLAY_MODE	m_eDisplayMode;

	CConfigDlg		m_ConfigDlg;
	CCap5System		m_CAP5System;

	HICON m_hIcon;

protected:
	virtual UINT GetNumEngine() = 0;
	virtual CEngine *GetEngine( int iEngId ) = 0;
	virtual VCA5_APP_VIDEOSRC_INFO *GetSrcInfo( int iEngId ) = 0;
	virtual BOOL CreateEngine( int iEngId ) = 0;
	virtual BOOL DestroyEngine( int iEngId ) = 0;
	virtual void Split( SIZE &size );
	virtual void OnConfigUpdated( int iEngId ) {;}


	BOOL	SetupEngine( int iEngId, BOOL bUpdateDisplayFlagsOfDataMgr );
	BOOL	TeardownEngine( int iEngId );
	IVCAVideoSource	*CreateVideoSource( int iEngId );

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPaint();
	afx_msg LRESULT OnExportConfig( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnImportConfig( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnConfigMode( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnVideoSrcChanged( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSetup( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnConfigUpdated( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()
};
