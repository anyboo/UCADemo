#pragma once

#include "CalibrationCtl.h"
#include "ConfigPage.h"

class CVCADialog;

// CCalibrationConfigPage dialog

class CCalibrationConfigPage : public CConfigPage
{
	DECLARE_DYNAMIC(CCalibrationConfigPage)

public:
	CCalibrationConfigPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCalibrationConfigPage();

// Dialog Data
	enum { IDD = IDD_CALIBRATIONCONFIGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

private:
	CCalibrationCtl	m_CalibCtl;
	CWnd			*m_pOldParent;
	BOOL			m_bAlive;
	DWORD			m_OldDisplay;

public:
	afx_msg void OnBnClickedBnCalibPause();
	afx_msg void OnBnClickedButtonUncalibrate();
};
