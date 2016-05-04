// ./VCADialog/LicenseConfigPage.cpp : implementation file
//

#include "stdafx.h"
#include "LicenseConfigPage.h"
#include "VCADialog.h"
#include "..\LicenseDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CLicenseConfigPage dialog

IMPLEMENT_DYNAMIC(CLicenseConfigPage, CConfigPage)

CLicenseConfigPage::CLicenseConfigPage(CWnd* pParent /*=NULL*/)
	: CConfigPage(CLicenseConfigPage::IDD, pParent)
{

}

CLicenseConfigPage::~CLicenseConfigPage()
{
}

void CLicenseConfigPage::DoDataExchange(CDataExchange* pDX)
{
	CConfigPage::DoDataExchange(pDX);
}

// CLicenseConfigPage message handlers
BEGIN_MESSAGE_MAP(CLicenseConfigPage, CConfigPage)
	ON_BN_CLICKED(IDC_BTN_LICENSE_REGISTER, &CLicenseConfigPage::OnBnClickedBtnLicenseRegister)
	ON_BN_CLICKED(IDC_BTN_LICENSE_REMOVE, &CLicenseConfigPage::OnBnClickedBtnLicenseUnregister)
	ON_BN_CLICKED(IDC_BUTTON_ASSIGN, &CLicenseConfigPage::OnBnClickedButtonAssign)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CLicenseConfigPage::OnBnClickedButtonRemove)
END_MESSAGE_MAP()




BOOL CLicenseConfigPage::OnInitDialog()
{
	//initalize and insert license list 
	CListCtrl *pLicenseFileList = (CListCtrl *)GetDlgItem(IDC_LICENSE_CONFIG_LIST);
	pLicenseFileList->SetExtendedStyle( LVS_EX_FULLROWSELECT );
	pLicenseFileList->InsertColumn(0, _T("License ID"), LVCFMT_LEFT, 70);
	pLicenseFileList->InsertColumn(1, _T("File Name"), LVCFMT_LEFT, 80);
	pLicenseFileList->InsertColumn(2, _T("Support Ch"), LVCFMT_LEFT, 70);
	pLicenseFileList->InsertColumn(3, _T("Used Ch"), LVCFMT_LEFT, 70);
	pLicenseFileList->InsertColumn(4, _T("Description"), LVCFMT_LEFT, 230);

	//
	CListCtrl *pLicenseSelEngList = (CListCtrl *)GetDlgItem(IDC_LICENSE_CONFIG_LIST_ENGINE);
	pLicenseSelEngList->SetExtendedStyle( LVS_EX_FULLROWSELECT );
		
	pLicenseSelEngList->InsertColumn(0, _T("License ID"), LVCFMT_LEFT, 100);
	pLicenseSelEngList->InsertColumn(1, _T("Description"), LVCFMT_LEFT, 230);
	
	m_pEngineInfo = m_pVCADialog->GetVCADataMgr()->GetEngineInfo();//CAPPConfigure::Instance()->GetAPPEngineInfo(m_engId);
	 
	
	Update();
	return TRUE;
}

void CLicenseConfigPage::UpdateData(BOOL bSave)
{
	CLicenseMgr*  pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();
	VCA_APP_LICENSE_INFO* pLicenseInfos = pLicenseMgr->GetLicenseInfo();

	for(DWORD i = 0 ; i < pLicenseMgr->GetLicenseCnt() ; i++){
		pLicenseInfos[i].TmpUsedCount = pLicenseInfos[i].UsedCount;
	}

	Update(bSave);
}

void CLicenseConfigPage::Update(BOOL bSave)
{
	if(GetSafeHwnd()){
		UpdateLicenseInfo();
		UpdateEngineLicenseInfo();
	}

	if(bSave){
		CAPPConfigure *pAppCfg = CAPPConfigure::Instance();
		pAppCfg->Save();
	}
}

void CLicenseConfigPage::UpdateLicenseInfo()
{

	CLicenseMgr* pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();

	//initalize and insert license list 
	CListCtrl *pLicenseFileList = (CListCtrl *)GetDlgItem(IDC_LICENSE_CONFIG_LIST);
	pLicenseFileList->DeleteAllItems();
	
	LVITEM lv;
	VCA_APP_LICENSE_INFO *pLicenseInfo;
	TCHAR strTemp[64];

	USES_CONVERSION;
	ULONG j, i;
	for(j = 0, i = 0 ; i < pLicenseMgr->GetLicenseCnt() ; i++){
		memset(&lv, 0, sizeof(lv));
		pLicenseInfo = pLicenseMgr->GetLicenseInfo(i);
		if(!pLicenseInfo) continue;
		if(LICENSE_STATUS_ACTIVATE != pLicenseInfo->Status) continue;

		_stprintf_s(strTemp, 64 ,_T("License %d"), pLicenseInfo->VCA5LicenseInfo.nLicenseID);

		lv.iItem	= j;
		lv.mask		= LVIF_TEXT|LVIF_PARAM;
		lv.iSubItem = 0;
		lv.pszText	= strTemp;
		lv.lParam	= pLicenseInfo->VCA5LicenseInfo.nLicenseID;
		pLicenseFileList->InsertItem(&lv);

		
		USES_CONVERSION;
		pLicenseFileList->SetItemText(j,1, pLicenseInfo->szLicensePath); 
		_itot(pLicenseInfo->VCA5LicenseInfo.ulNumOfEngine, strTemp, 10);
		pLicenseFileList->SetItemText(j,2, strTemp); 
		_itot(pLicenseInfo->TmpUsedCount, strTemp, 10);
		pLicenseFileList->SetItemText(j,3, strTemp); 
		pLicenseFileList->SetItemText(j,4, A2T(pLicenseInfo->VCA5LicenseInfo.szLicenseDesc));
		j++;
	}
}


void CLicenseConfigPage::UpdateEngineLicenseInfo()
{
	
	DWORD EngID = m_pVCADialog->GetEngId();

	CLicenseMgr* pLicenseMgr			= CAPPConfigure::Instance()->GetLicenseMgr();
	CListCtrl* pLicenseSelEngList		= (CListCtrl *)GetDlgItem(IDC_LICENSE_CONFIG_LIST_ENGINE);

	pLicenseSelEngList->DeleteAllItems();

	LVITEM lv;
	TCHAR strTemp[64];
	VCA_APP_LICENSE_INFO *pLicenseInfo;
	USES_CONVERSION;

	for(ULONG i = 0 ; i < m_pEngineInfo->ulLicenseCnt ;i++){
		pLicenseInfo = pLicenseMgr->GetLicenseInfoById(m_pEngineInfo->ucLicenseId[i]);
		if(!pLicenseInfo) continue;
		if(LICENSE_STATUS_ACTIVATE != pLicenseInfo->Status) continue;
		

		memset(&lv, 0, sizeof(lv));
		_stprintf_s(strTemp, 64 ,_T("License %d"), pLicenseInfo->VCA5LicenseInfo.nLicenseID);

		lv.iItem	= i;
		lv.mask		= LVIF_TEXT|LVIF_PARAM;
		lv.iSubItem = 0;
		lv.pszText	= strTemp;
		lv.lParam	= pLicenseInfo->VCA5LicenseInfo.nLicenseID;
		pLicenseSelEngList->InsertItem(&lv);
		pLicenseSelEngList->SetItemText(i,1, A2T(pLicenseInfo->VCA5LicenseInfo.szLicenseDesc));		
	}
}


void CLicenseConfigPage::OnBnClickedBtnLicenseRegister()
{
	CLicenseDlg dlg(CAPPConfigure::Instance()->GetLicenseMgr(), NULL);
	dlg.SetSameFileAsFail(FALSE);
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK){
		Update(TRUE);
	}
}


void CLicenseConfigPage::OnBnClickedBtnLicenseUnregister()
{
	CListCtrl *pLicenseFileList = (CListCtrl *)GetDlgItem(IDC_LICENSE_CONFIG_LIST);
	int iItem = GetFirstSelectItem(pLicenseFileList);
	
	LVITEM lv;
	memset(&lv, 0, sizeof(lv));
	lv.iItem	= iItem;
	lv.mask		= LVIF_PARAM;
	DWORD	LicenseID;
	
	CLicenseMgr* pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();
	if(pLicenseFileList->GetItem(&lv)){
		LicenseID	= lv.lParam;
		VCA_APP_LICENSE_INFO *pInfo = pLicenseMgr->GetLicenseInfoById(LicenseID);
		if(pInfo && pInfo->TmpUsedCount){
			AfxMessageBox(_T("License used by engine, Can not Unregister License"));
			return;
		}

		if(pLicenseMgr->RemoveLicense(LicenseID)){
			Update(TRUE);
		}
	}
}


void CLicenseConfigPage::OnBnClickedButtonAssign()
{
	CListCtrl *pLicenseFileList = (CListCtrl *)GetDlgItem(IDC_LICENSE_CONFIG_LIST);
	int iItem = GetFirstSelectItem(pLicenseFileList);
	
	LVITEM lv;
	memset(&lv, 0, sizeof(lv));
	lv.iItem	= iItem;
	lv.mask		= LVIF_PARAM;
	DWORD i ;

	CLicenseMgr* pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();
	VCA_APP_LICENSE_INFO *pLicenseInfo;


	if(pLicenseFileList->GetItem(&lv)){
		DWORD LicenseID = lv.lParam;
		pLicenseInfo = pLicenseMgr->GetLicenseInfo(LicenseID);


		//Check same License id
		for( i = 0 ; i < m_pEngineInfo->ulLicenseCnt ; i++){
			if(m_pEngineInfo->ucLicenseId[i] == LicenseID){
				AfxMessageBox(_T("Warning : same license assigned"));
				return;
			}
		}

		m_pEngineInfo->ucLicenseId[m_pEngineInfo->ulLicenseCnt] = (UCHAR)LicenseID;
		m_pEngineInfo->ulLicenseCnt += 1;

		VCA_APP_LICENSE_INFO *pLicenseInfo = pLicenseMgr->GetLicenseInfoById(LicenseID);
		if(pLicenseInfo) pLicenseInfo->TmpUsedCount += 1;
		UpdateLicenseInfo();
		UpdateEngineLicenseInfo();

		CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();
		pDataMgr->FireEvent( IVCAConfigureObserver::VCA_ENGINE_UPDATE, NULL );
	}
}


void CLicenseConfigPage::OnBnClickedButtonRemove()
{
	CListCtrl *pLicenseEngineList = (CListCtrl *)GetDlgItem(IDC_LICENSE_CONFIG_LIST_ENGINE);
	DWORD EngID = m_pVCADialog->GetEngId();
		
	int iItem = GetFirstSelectItem(pLicenseEngineList);
	LVITEM lv;
	memset(&lv, 0, sizeof(lv));
	lv.iItem	= iItem;
	lv.mask		= LVIF_PARAM;
	
	DWORD i,j;
	UCHAR	ucLicenseIdTemp[VCA5_MAX_NUM_LICENSE];
	CLicenseMgr* pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();

	if(pLicenseEngineList->GetItem(&lv)){
		DWORD LicenseID = lv.lParam;
		//
		for(i = 0 , j = 0; i < m_pEngineInfo->ulLicenseCnt ; i++){
			if(m_pEngineInfo->ucLicenseId[i] != LicenseID){
				ucLicenseIdTemp[j] = m_pEngineInfo->ucLicenseId[i];
				j++;
			}
		}

		m_pEngineInfo->ulLicenseCnt = j;
		memcpy(m_pEngineInfo->ucLicenseId, ucLicenseIdTemp, m_pEngineInfo->ulLicenseCnt);

		VCA_APP_LICENSE_INFO *pLicenseInfo = pLicenseMgr->GetLicenseInfoById(LicenseID);
		if(pLicenseInfo) pLicenseInfo->TmpUsedCount -= 1;
		UpdateLicenseInfo();
		UpdateEngineLicenseInfo();
		
		CVCADataMgr *pDataMgr = m_pVCADialog->GetVCADataMgr();
		pDataMgr->FireEvent( IVCAConfigureObserver::VCA_ENGINE_UPDATE, NULL );

	}
}

int		CLicenseConfigPage::GetFirstSelectItem(CListCtrl *pListCtrl)
{
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
	if (pos == NULL){
	   TRACE0("No items were selected!\n");
		return -1;
	}
	return pListCtrl->GetNextSelectedItem(pos);
	
}