#pragma once
#include "resource.h"

// StreamDlg dialog

class StreamDlg : public CDialog
{
	DECLARE_DYNAMIC(StreamDlg)

public:
	StreamDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~StreamDlg();

// Dialog Data
	enum { IDD = IDD_STREAMDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
