#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "VCADataMgr.h"
#include "ConfigPage.h"


// CClassificationSettingPage dialog
class CVCADialog;
class CClassificationSettingPage : public CConfigPage
{
	DECLARE_DYNAMIC(CClassificationSettingPage)

public:
	CClassificationSettingPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CClassificationSettingPage();

// Dialog Data
	enum { IDD = IDD_CLS_SETTING_DIALOG };


private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void LoadPrevSetting();
	BOOL isAllCtrlsFilled();
	BOOL isAllCtrlsDataValid();
	void FillClsObjectsListCtrl();
	void Reset();
	void FillRightCtrls(int nItem);
	void SetFocusOnListCtrl();
	void SetFocusOnListCtrl(int nItemindex);
	void SyncToDataMgr();
	
	DECLARE_MESSAGE_MAP()

	BOOL		m_bPopulating;
	int			m_nCurSelIndex;
	CListCtrl	m_ctrlClassifierList;
	CEdit		m_ctrlName;
	CComboBox	m_ctrlEnableCombo;
	CEdit		m_ctrlMinArea;
	CEdit		m_ctrlMaxArea;
	CEdit		m_ctrlMinSpeed;
	CEdit		m_ctrlMaxSpeed;

		
	CArray <VCA5_APP_CLSOBJECT, VCA5_APP_CLSOBJECT> m_arrClsObjects;

	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedClassifierList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnRemove();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnReset();


	afx_msg void OnEnChangeEdName();
	afx_msg void OnCbnSelchangeComboEnable();
	afx_msg void OnEnChangeEdMinarea();
	afx_msg void OnEnChangeEdMaxarea();
	afx_msg void OnEnChangeEdMinspeed();
	afx_msg void OnEnChangeEdMaxspeed();
	afx_msg void OnEnKillfocusEdName();

	int FindObjectName(CString &ObjectName);
};
