// CalibrationSettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../resource.h"
#include "ClassificationSettingPage.h"
#include "VCADialog.h"
#include "../Common/wm_user.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CClassificationSettingPage dialog

IMPLEMENT_DYNAMIC(CClassificationSettingPage, CConfigPage)

CClassificationSettingPage::CClassificationSettingPage(CWnd* pParent /*=NULL*/)
	: CConfigPage(CClassificationSettingPage::IDD, pParent)
{
	m_pVCADialog	= NULL;
	m_nCurSelIndex	= 0;
	m_bPopulating = FALSE;
}

CClassificationSettingPage::~CClassificationSettingPage()
{
}

void CClassificationSettingPage::DoDataExchange(CDataExchange* pDX)
{
	CConfigPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLASSIFIER_LIST, m_ctrlClassifierList);
	DDX_Control(pDX, IDC_COMBO_ENABLE, m_ctrlEnableCombo);
	DDX_Control(pDX, IDC_ED_MINAREA, m_ctrlMinArea);
	DDX_Control(pDX, IDC_ED_MAXAREA, m_ctrlMaxArea);
	DDX_Control(pDX, IDC_ED_MINSPEED, m_ctrlMinSpeed);
	DDX_Control(pDX, IDC_ED_MAXSPEED, m_ctrlMaxSpeed);
	DDX_Control(pDX, IDC_ED_NAME, m_ctrlName);
}


BEGIN_MESSAGE_MAP(CClassificationSettingPage, CConfigPage)
	ON_BN_CLICKED(IDC_BTN_ADD, &CClassificationSettingPage::OnBnClickedBtnAdd)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CLASSIFIER_LIST, &CClassificationSettingPage::OnLvnItemchangedClassifierList)
	ON_BN_CLICKED(IDC_BTN_REMOVE, &CClassificationSettingPage::OnBnClickedBtnRemove)
	ON_BN_CLICKED(IDC_BTN_RESET, &CClassificationSettingPage::OnBnClickedBtnReset)
	ON_EN_CHANGE(IDC_ED_NAME, &CClassificationSettingPage::OnEnChangeEdName)
	ON_CBN_SELCHANGE(IDC_COMBO_ENABLE, &CClassificationSettingPage::OnCbnSelchangeComboEnable)
	ON_EN_CHANGE(IDC_ED_MINAREA, &CClassificationSettingPage::OnEnChangeEdMinarea)
	ON_EN_CHANGE(IDC_ED_MAXAREA, &CClassificationSettingPage::OnEnChangeEdMaxarea)
	ON_EN_CHANGE(IDC_ED_MINSPEED, &CClassificationSettingPage::OnEnChangeEdMinspeed)
	ON_EN_CHANGE(IDC_ED_MAXSPEED, &CClassificationSettingPage::OnEnChangeEdMaxspeed)
	ON_EN_KILLFOCUS(IDC_ED_NAME, &CClassificationSettingPage::OnEnKillfocusEdName)
END_MESSAGE_MAP()



void CClassificationSettingPage::SyncToDataMgr()
{
	if( m_bPopulating )
	{
		return;
	}

	// Sync from local config back to datamgr
	VCA5_APP_CLSOBJECTS* pClsObjects = m_pVCADialog->GetVCADataMgr()->GetClsObjects();
	pClsObjects->ulTotalClsObjects = m_arrClsObjects.GetCount();
	for(int i=0; i<m_arrClsObjects.GetCount(); i++)	{
		pClsObjects->pClsObjects[i].sClsObjectId = m_arrClsObjects.GetAt(i).sClsObjectId;
		strcpy_s(pClsObjects->pClsObjects[i].szClsobjectName, m_arrClsObjects.GetAt(i).szClsobjectName);
		pClsObjects->pClsObjects[i].bEnable	= m_arrClsObjects.GetAt(i).bEnable;
		pClsObjects->pClsObjects[i].tAreaSetting.usAreaLo	= m_arrClsObjects.GetAt(i).tAreaSetting.usAreaLo;
		pClsObjects->pClsObjects[i].tAreaSetting.usAreaUp	= m_arrClsObjects.GetAt(i).tAreaSetting.usAreaUp;

		pClsObjects->pClsObjects[i].tSpeedSetting.usSpeedLo	= m_arrClsObjects.GetAt(i).tSpeedSetting.usSpeedLo;
		pClsObjects->pClsObjects[i].tSpeedSetting.usSpeedUp	= m_arrClsObjects.GetAt(i).tSpeedSetting.usSpeedUp;
	}
	
	m_pVCADialog->GetVCADataMgr()->FireEvent( IVCAConfigureObserver::VCA_OBJCLS_UPDATE, NULL );
}

void CClassificationSettingPage::LoadPrevSetting()
{
	VCA5_APP_CLSOBJECTS* pClsObjects = m_pVCADialog->GetVCADataMgr()->GetClsObjects();
	for(ULONG i=0; i<pClsObjects->ulTotalClsObjects; i++)	{
		m_arrClsObjects.Add(pClsObjects->pClsObjects[i]);
	}

}

void CClassificationSettingPage::FillClsObjectsListCtrl()
{
	m_ctrlClassifierList.DeleteAllItems();
	for(int i=0; i<m_arrClsObjects.GetCount(); i++)	{
		int idx = m_arrClsObjects.GetAt(i).sClsObjectId;
		CString id; id.Format(_T("%d"), idx);
		m_ctrlClassifierList.InsertItem(idx, id);
		m_ctrlClassifierList.SetItemText(idx, 1, CA2T(m_arrClsObjects.GetAt(i).szClsobjectName, CP_UTF8));
	}
}

void CClassificationSettingPage::SetFocusOnListCtrl(int nItemindex)
{
	if((nItemindex<0) || (nItemindex >= m_arrClsObjects.GetCount())) {
		return;	// index error
	}
	m_ctrlClassifierList.SetItemState(nItemindex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_ctrlClassifierList.EnsureVisible(nItemindex, FALSE);	// for Scrollbar
}

void CClassificationSettingPage::SetFocusOnListCtrl()
{
	SetFocusOnListCtrl(m_arrClsObjects.GetCount()-1);
}

// CClassificationSettingPage message handlers

BOOL CClassificationSettingPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rt;
	m_ctrlClassifierList.GetClientRect(&rt);
	m_ctrlClassifierList.InsertColumn(0, _T("ID"), LVCFMT_CENTER, 80);
	m_ctrlClassifierList.InsertColumn(1, _T("Name"), LVCFMT_CENTER, rt.Width()-80);

	m_ctrlClassifierList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_ctrlEnableCombo.AddString(_T("Yes"));
	m_ctrlEnableCombo.AddString(_T("No"));
	m_ctrlEnableCombo.SetCurSel(0);

	m_arrClsObjects.RemoveAll();

	LoadPrevSetting();
	FillClsObjectsListCtrl();
	SetFocusOnListCtrl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CClassificationSettingPage::isAllCtrlsFilled()
{
	CString str;
	m_ctrlName.GetWindowText(str);	
	if(str.IsEmpty()) {
		AfxMessageBox(_T("Input the name"));
		m_ctrlName.SetFocus();
		return FALSE;
	}

	m_ctrlMinArea.GetWindowText(str);
	if(str.IsEmpty()) {
		AfxMessageBox(_T("Input the Min Area"));
		m_ctrlMinArea.SetFocus();
		return FALSE;
	}
	m_ctrlMaxArea.GetWindowText(str);
	if(str.IsEmpty()) {
		AfxMessageBox(_T("Input the Max Area"));
		m_ctrlMaxArea.SetFocus();
		return FALSE;
	}
	m_ctrlMinSpeed.GetWindowText(str);
	if(str.IsEmpty()) {
		AfxMessageBox(_T("Input the Min Speed"));
		m_ctrlMinSpeed.SetFocus();
		return FALSE;
	}
	m_ctrlMaxSpeed.GetWindowText(str);
	if(str.IsEmpty()) {
		AfxMessageBox(_T("Input the Max Speed"));
		m_ctrlMaxSpeed.SetFocus();
		return FALSE;
	}
	return TRUE;
}

BOOL CClassificationSettingPage::isAllCtrlsDataValid()
{
	// TODO
	return TRUE;
}

void CClassificationSettingPage::OnBnClickedBtnAdd()
{
	if(FALSE == isAllCtrlsDataValid()) {
		return;
	}
	USES_CONVERSION;
	
	VCA5_APP_CLSOBJECT pClsObject;
	CString idx, name, val; 

	pClsObject.sClsObjectId = m_arrClsObjects.GetCount();	// id
	if(m_arrClsObjects.GetCount() > VCA5_MAX_NUM_CLSOBJECTS){
		TCHAR strError[256];
		_stprintf(strError, _T("Exceed Max Object count [%d], Please remove another one "), VCA5_MAX_NUM_CLSOBJECTS);
		MessageBox(strError, _T("Error"), MB_OK);
		return;
	}

	name.Format(_T("Object %d"), m_arrClsObjects.GetCount());
	int index = 1;
	while(FindObjectName(name) != -1){
		name.Format(_T("Object %d"), m_arrClsObjects.GetCount()+index++);
	}


	strcpy_s(pClsObject.szClsobjectName, CT2A(name, CP_UTF8));		// name

	pClsObject.bEnable		= TRUE;
	pClsObject.tAreaSetting.usAreaLo = 0*10;
	pClsObject.tAreaSetting.usAreaUp = 200*10;
	pClsObject.tSpeedSetting.usSpeedLo = 0;
	pClsObject.tSpeedSetting.usSpeedUp = 200;
	m_arrClsObjects.Add(pClsObject);
	
	FillClsObjectsListCtrl();
	SetFocusOnListCtrl();

	SyncToDataMgr();
}



void CClassificationSettingPage::FillRightCtrls(int nItem=0)
{
	m_bPopulating = TRUE;
	USES_CONVERSION;

	VCA5_APP_CLSOBJECT pClsObject;

	m_nCurSelIndex = nItem;
	if(m_arrClsObjects.GetCount() <= 0) {
		m_ctrlName.SetWindowText(_T(""));
		m_ctrlEnableCombo.SetCurSel(0);
		m_ctrlMinArea.SetWindowText(_T(""));
		m_ctrlMaxArea.SetWindowText(_T(""));
		m_ctrlMinSpeed.SetWindowText(_T(""));
		m_ctrlMaxSpeed.SetWindowText(_T(""));
	}else{
		CString val;
		pClsObject = m_arrClsObjects.GetAt(nItem);
		m_ctrlName.SetWindowText(CA2T(pClsObject.szClsobjectName, CP_UTF8));
		m_ctrlEnableCombo.SetCurSel((pClsObject.bEnable==TRUE)?0:1);
		val.Format(_T("%.1f"), (float)pClsObject.tAreaSetting.usAreaLo/10.0); m_ctrlMinArea.SetWindowText(val);
		val.Format(_T("%.1f"), (float)pClsObject.tAreaSetting.usAreaUp/10.0); m_ctrlMaxArea.SetWindowText(val);
		val.Format(_T("%d"), pClsObject.tSpeedSetting.usSpeedLo); m_ctrlMinSpeed.SetWindowText(val);
		val.Format(_T("%d"), pClsObject.tSpeedSetting.usSpeedUp); m_ctrlMaxSpeed.SetWindowText(val);
	}

	m_bPopulating = FALSE;
}

void CClassificationSettingPage::OnLvnItemchangedClassifierList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	int idx = pNMLV->iItem;
	FillRightCtrls(idx);
	*pResult = 0;
}

void CClassificationSettingPage::OnBnClickedBtnRemove()
{

	POSITION pos = m_ctrlClassifierList.GetFirstSelectedItemPosition();
	if (pos == NULL) return;
	int nItem = m_ctrlClassifierList.GetNextSelectedItem(pos);
/*	
	if(nItem < 0) {
		return; // list of cls objs is empty
	}
*/
	m_arrClsObjects.RemoveAt(nItem);
	for(int i=0; i<m_arrClsObjects.GetCount(); i++)	{
		m_arrClsObjects.GetAt(i).sClsObjectId = i;
	}

	nItem = (m_arrClsObjects.GetCount() <= 0) ? 0 : nItem;	// prevent passing the minimum of the list
	if(nItem > 0) {
		nItem = (nItem > (m_arrClsObjects.GetCount()-1)) ? (m_arrClsObjects.GetCount()-1) : nItem;	// prevent passing the maximum of the list
	}

	FillClsObjectsListCtrl();
	SetFocusOnListCtrl(nItem);
	FillRightCtrls(nItem);

	SyncToDataMgr();
}

void CClassificationSettingPage::OnBnClickedBtnReset()
{

	Reset();
	FillClsObjectsListCtrl();
	SetFocusOnListCtrl();

}



void CClassificationSettingPage::Reset()
{
	VCA5_APP_CLSOBJECTS* pClsObjects = m_pVCADialog->GetVCADataMgr()->GetClsObjects();
	memset(pClsObjects, 0, sizeof(VCA5_APP_CLSOBJECTS));

	m_arrClsObjects.RemoveAll();

	pClsObjects->ulTotalClsObjects = 4;

	strcpy_s(pClsObjects->pClsObjects[0].szClsobjectName, "Person");
	pClsObjects->pClsObjects[0].sClsObjectId			= 0;
	pClsObjects->pClsObjects[0].bEnable					= TRUE;
	pClsObjects->pClsObjects[0].tAreaSetting.usAreaLo	= 5;
	pClsObjects->pClsObjects[0].tAreaSetting.usAreaUp	= 20;	
	pClsObjects->pClsObjects[0].tSpeedSetting.usSpeedLo = 0;
	pClsObjects->pClsObjects[0].tSpeedSetting.usSpeedUp = 20;
	m_arrClsObjects.Add(pClsObjects->pClsObjects[0]);

	strcpy_s(pClsObjects->pClsObjects[1].szClsobjectName, "Group of People");
	pClsObjects->pClsObjects[1].sClsObjectId			= 1;
	pClsObjects->pClsObjects[1].bEnable					= TRUE;
	pClsObjects->pClsObjects[1].tAreaSetting.usAreaLo	= 21;
	pClsObjects->pClsObjects[1].tAreaSetting.usAreaUp	= 39;	
	pClsObjects->pClsObjects[1].tSpeedSetting.usSpeedLo = 0;
	pClsObjects->pClsObjects[1].tSpeedSetting.usSpeedUp = 20;
	m_arrClsObjects.Add(pClsObjects->pClsObjects[1]);

	strcpy_s(pClsObjects->pClsObjects[2].szClsobjectName, "Vehicle");
	pClsObjects->pClsObjects[2].sClsObjectId			= 2;
	pClsObjects->pClsObjects[2].bEnable					= TRUE;
	pClsObjects->pClsObjects[2].tAreaSetting.usAreaLo	= 40;
	pClsObjects->pClsObjects[2].tAreaSetting.usAreaUp	= 1000;	
	pClsObjects->pClsObjects[2].tSpeedSetting.usSpeedLo = 0;
	pClsObjects->pClsObjects[2].tSpeedSetting.usSpeedUp = 200;
	m_arrClsObjects.Add(pClsObjects->pClsObjects[2]);

	strcpy_s(pClsObjects->pClsObjects[3].szClsobjectName, "Clutter");
	pClsObjects->pClsObjects[3].sClsObjectId			= 3;
	pClsObjects->pClsObjects[3].bEnable					= TRUE;
	pClsObjects->pClsObjects[3].tAreaSetting.usAreaLo	= 0;
	pClsObjects->pClsObjects[3].tAreaSetting.usAreaUp	= 4;	
	pClsObjects->pClsObjects[3].tSpeedSetting.usSpeedLo = 0;
	pClsObjects->pClsObjects[3].tSpeedSetting.usSpeedUp = 50;
	m_arrClsObjects.Add(pClsObjects->pClsObjects[3]);

	SyncToDataMgr();
}


void CClassificationSettingPage::OnEnChangeEdName()
{
	USES_CONVERSION;

	if(m_arrClsObjects.GetCount() <= 0) {
		return;
	}
	CString name; 
	m_ctrlName.GetWindowText(name);
	strcpy_s(m_arrClsObjects.GetAt(m_nCurSelIndex).szClsobjectName, CT2A(name, CP_UTF8));		// name

	SyncToDataMgr();
}


void CClassificationSettingPage::OnCbnSelchangeComboEnable()
{
	if(m_arrClsObjects.GetCount() <= 0) {
		return;
	}
	m_arrClsObjects.GetAt(m_nCurSelIndex).bEnable = (m_ctrlEnableCombo.GetCurSel() == 0) ? TRUE : FALSE;

	SyncToDataMgr();
}

void CClassificationSettingPage::OnEnChangeEdMinarea()
{
	if(m_arrClsObjects.GetCount() <= 0) {
		return;
	}
	CString val; 
	float usAreaLo;

	m_ctrlMinArea.GetWindowText(val); 
	_stscanf_s(val, _T("%f"), &usAreaLo);

	usAreaLo = usAreaLo + (float)0.01;
	m_arrClsObjects.GetAt(m_nCurSelIndex).tAreaSetting.usAreaLo = (USHORT)(usAreaLo*10);

	SyncToDataMgr();
}

void CClassificationSettingPage::OnEnChangeEdMaxarea()
{
	if(m_arrClsObjects.GetCount() <= 0) {
		return;
	}
	CString val; 
	float usAreaUp;

	m_ctrlMaxArea.GetWindowText(val); 
	_stscanf_s(val, _T("%f"), &usAreaUp);
	usAreaUp = usAreaUp + (float)0.01;
	m_arrClsObjects.GetAt(m_nCurSelIndex).tAreaSetting.usAreaUp = (USHORT)(usAreaUp*10);

	SyncToDataMgr();
}

void CClassificationSettingPage::OnEnChangeEdMinspeed()
{
	if(m_arrClsObjects.GetCount() <= 0) {
		return;
	}
	CString val; 
	m_ctrlMinSpeed.GetWindowText(val); 
	m_arrClsObjects.GetAt(m_nCurSelIndex).tSpeedSetting.usSpeedLo = (USHORT)_ttoi(val);

	SyncToDataMgr();
}

void CClassificationSettingPage::OnEnChangeEdMaxspeed()
{
	if(m_arrClsObjects.GetCount() <= 0) {
		return;
	}
	CString val; 
	m_ctrlMaxSpeed.GetWindowText(val); 
	m_arrClsObjects.GetAt(m_nCurSelIndex).tSpeedSetting.usSpeedUp = (USHORT)_ttoi(val);

	SyncToDataMgr();
}

void CClassificationSettingPage::OnEnKillfocusEdName()
{
	if(m_arrClsObjects.GetCount() <= 0) {
		return;
	}
	// Get current position of the List
	POSITION pos = m_ctrlClassifierList.GetFirstSelectedItemPosition();
	if (pos == NULL) return;
	int nItem = m_ctrlClassifierList.GetNextSelectedItem(pos);

	FillClsObjectsListCtrl();
	SetFocusOnListCtrl(nItem);

//	SyncToDataMgr();
}


int CClassificationSettingPage::FindObjectName(CString &ObjectName)
{
	int i;

	CString strObjectName;
	for( i = 0 ; i < m_ctrlClassifierList.GetItemCount() ; i++ ){
		strObjectName = m_ctrlClassifierList.GetItemText(i, 1);
		if(0 == strObjectName.Compare(ObjectName)) return i;
	}

	return -1;
}
