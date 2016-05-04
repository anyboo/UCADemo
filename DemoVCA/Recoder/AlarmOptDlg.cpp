#include "stdafx.h"
#include "../resource.h"
#include "AlarmOptDlg.h"
#include "../common/APPConfigure.h"



IMPLEMENT_DYNAMIC(CAlaramOptDlg, CDialog)

CAlaramOptDlg::CAlaramOptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlaramOptDlg::IDD, pParent)
{

}

CAlaramOptDlg::~CAlaramOptDlg()
{
}

BOOL CAlaramOptDlg::OnInitDialog()
{
	m_Path = CAPPConfigure::Instance()->GetAlarmSavePath();
	if (m_Path.IsEmpty()) {
		m_Path = _T("C:\\SimpleVCAApp_Log");
	}
	m_bEnable = CAPPConfigure::Instance()->IsAlarmSaveEnabled();
	m_dwPeriod = CAPPConfigure::Instance()->GetAlarmSavePeriod();
	if (m_dwPeriod <= 0) {
		m_dwPeriod = 1;
	}

	UpdateData(FALSE);
	OnBnClickedEnable();
	return CDialog::OnInitDialog();
}

void CAlaramOptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PATH, m_Path);
	DDV_MaxChars(pDX, m_Path, MAX_PATH);
	DDX_Text(pDX, IDC_EDIT_PERIOD, m_dwPeriod);
	DDV_MinMaxInt(pDX, m_dwPeriod, 1, 60);
	DDX_Check(pDX, IDC_CHECK_ENABLE, m_bEnable);
}


BEGIN_MESSAGE_MAP(CAlaramOptDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_ENABLE, &CAlaramOptDlg::OnBnClickedEnable)
	ON_BN_CLICKED(IDOK, &CAlaramOptDlg::OnBnClickedOk)
END_MESSAGE_MAP()


void CAlaramOptDlg::OnBnClickedEnable()
{
	if (IsDlgButtonChecked(IDC_CHECK_ENABLE)) {
		GetDlgItem(IDC_EDIT_PATH)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PERIOD)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_EDIT_PATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_PERIOD)->EnableWindow(FALSE);
	}
}

void CAlaramOptDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	CAPPConfigure::Instance()->SetAlarmSaveEnable(m_bEnable? 1 : 0);
	if (m_bEnable) {
		CAPPConfigure::Instance()->SetAlarmSavePath((LPCTSTR) m_Path);
		CAPPConfigure::Instance()->SetAlarmSavePeriod(m_dwPeriod);
	}
	OnOK();
}
