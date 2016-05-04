#pragma once
#include "configtabctrl.h"
#include "../resource.h"
#include "AdvancedSettingsPage.h"
#include "ClassificationSettingPage.h"
#include "ZonesRulesConfigPage.h"
#include "CalibrationConfigPage.h"
#include "TamperConfigPage.h"
#include "VideoSource/VideoConfigPage.h"
#include "VCACOnfigureObserver.h"
#include "LicenseConfigPage.h"
#include "SceneChangeConfigPage.h"


// CConfigDlg dialog

class CConfigDlg : public CDialog, public IVCAConfigureObserver
{
	DECLARE_DYNAMIC(CConfigDlg)

public:
	CConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigDlg();

// Dialog Data
	enum { IDD = IDD_CONFIG_DIALOG };

	void SetTabMask( unsigned int uiTabMask );
	void SetVCADialog( CVCADialog *pVCADialog );
	BOOL IsDirty() { return m_uiDirtyMask > 0; }

	enum CONFIG_TABS
	{
		CONFIG_ZONESRULES		= 0x00000001,
		CONFIG_CALIBRATION		= 0x00000002,
		CONFIG_CLASSIFICATION	= 0x00000004,
		CONFIG_VIDEOSRC			= 0x00000008,
		CONFIG_ADVANCED			= 0x00000010,
		CONFIG_LICENSE			= 0x00000020,
		CONFIG_TAMPER			= 0x00000040,
		CONFIG_SCENECHANGE		= 0x00000080,
		CONFIG_ALL				= 0xFFFFFFFF
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	virtual void FireOnEvent( DWORD uiEvent, DWORD uiContext );

	void UpdateTabs();

	DECLARE_MESSAGE_MAP()

private:

	BOOL	m_bAlive;

	unsigned int	m_uiAllowedTabMask;
	unsigned int	m_uiActualTabMask;
	unsigned int	m_uiDirtyMask;

	CVCADialog					*m_pVCADialog;
	// Dialogs
	CAdvancedSettingsPage		m_AdvancedPage;
	CClassificationSettingPage	m_ClassificationPage;
	CZonesRulesConfigPage		m_ZonesRulesPage;
	CCalibrationConfigPage		m_CalibPage;
	CTamperConfigPage			m_TamperPage;
	CSceneChangeConfigPage		m_SceneChangePage;
	CVideoConfigPage			m_VideoPage;
	CLicenseConfigPage			m_LicensePage;


private:
	CConfigTabCtrl m_TabCtrl;

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedConfigApply();
	afx_msg void OnBnClickedConfigExit();
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg LRESULT OnConfigDirty( WPARAM wParam, LPARAM lParam );
};
