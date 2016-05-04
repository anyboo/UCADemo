// StreamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "StreamDlg.h"


// StreamDlg dialog

IMPLEMENT_DYNAMIC(StreamDlg, CDialog)

StreamDlg::StreamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(StreamDlg::IDD, pParent)
{

}

StreamDlg::~StreamDlg()
{
}

void StreamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(StreamDlg, CDialog)
END_MESSAGE_MAP()


// StreamDlg message handlers
