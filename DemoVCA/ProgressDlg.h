#pragma once
#include "afxcmn.h"


// CProgressDlg dialog

class CProgressDlg : public CWnd
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	CProgressDlg(CWnd* pParent = NULL );   // standard constructor
	virtual ~CProgressDlg();

public:

// Dialog Data
	enum { IDD = IDD_PROGRESSDLG };

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual int OnCreate( LPCREATESTRUCT lpCreateStruct );
	virtual void OnPaint( );
	virtual BOOL OnEraseBkgnd( CDC *pDC );
	virtual void OnTimer( UINT_PTR nIDEvent );

	DECLARE_MESSAGE_MAP()

private:
	CImageList		m_ImageList;
	CBitmap			m_Bmp;

	int				m_iIdx;

	CWnd			*m_pParent;
};
