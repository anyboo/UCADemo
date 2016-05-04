#pragma once
#include "afxwin.h"
#include "VCAConfigure.h"
#include "VCAMetalib.h"
#include "JpegCodec.h"
#include "afxcmn.h"

// CAlarmViewDlg dialog
#define		IMAGE_NAME_LEN	16 // XXX_XXX_XXXX.JPG

class CD3d;
class CAlarmViewDlg : public CDialog
{
	DECLARE_DYNAMIC(CAlarmViewDlg)

public:
	CAlarmViewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmViewDlg();

// Dialog Data
	enum { IDD = IDD_ALARMVIWER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual INT_PTR DoModal(LPCTSTR pStartTime, LPCTSTR pEndTime, DWORD dwPeriod, LPCTSTR pEngId, LPCTSTR pAlarmId, LPCTSTR pRootPath);
	virtual BOOL OnInitDialog();

	BOOL ExportSnapShots(LPCTSTR pStartTime, LPCTSTR pEndTime, DWORD dwPeriod, LPCTSTR pEngId, LPCTSTR pAlarmId, LPCTSTR pRootPath);
	CString GetAviFilePath() { return m_strAviFileName; }

private:
	CD3d*		m_pD3d;
	int			m_nCurImageIdx;
	CStringArray	m_arFileNames;

	RECT			m_ClientRect;
	CButton			m_btnPrev;
	CButton			m_btnNext;
	CEdit			m_edFileName;
	CProgressCtrl	m_ctrlExportProgress;
	CButton			m_btnExport;

	CJpegCodec	m_JpegCodec;
	BYTE		*m_pBuffer;
	DWORD		m_dwBufferLength;

	CString m_strStartTime;
	CString m_strEndTime;
	CString m_strEngId;
	CString m_strAlarmId;
	CString m_strRootPath;
	BYTE*	m_pImageBuf;
	CString m_strAviFileName;

private:
	afx_msg void OnPaint();
	afx_msg void OnBnClickedBtnPrev();
	afx_msg void OnBnClickedBtnNext();
	BOOL Init(LPCTSTR pStartTime, LPCTSTR pEndTime, DWORD dwPeriod, LPCTSTR pEngId, LPCTSTR pAlarmId, LPCTSTR pRootPath);

	DWORD	FindJpegFileNames(LPCTSTR pRootPath, CString strStartTime, CString strEndTime, DWORD dwPeriod);
	DWORD	FindJpegFileNamesInSubFolder(LPCTSTR pRootPath, LPCTSTR pSubFolderName, LPCTSTR pEngID, LPCTSTR pAlarmID);
	void	MakeFolderNameString(CString &date);

	void SetNaviButtonDisable();
	void AdjustDlgRect(int width, int height);
	void RelocateControls(int width, int height);
	void DrawImage(int width, int height, BYTE* pImage, BITMAPINFOHEADER bm);
	void ShowCurrentImage();
	BOOL SaveSnapShot(CString strPath, DWORD dwEngineID, DWORD dwAlarmID, 
		DWORD idx, BYTE *pImage, BITMAPINFOHEADER *pbm, __int64 iTimeStamp);

	void	DrawSingleZone(RECT rcVCAROI, VCA5_APP_ZONE *pZone);
	void	DrawTimeStampAndRuleType(LONG width, LONG height, __int64 i64TimeStamp, int iruleType);
	void	DrawObject(RECT rcCanvas, LONG width, LONG height, VCA5_PACKET_OBJECT *pObject);
	void	DrawBoundinxBox(RECT clientRect, SIZE SourceSize, RECT rcVCAROI, VCA5_RECT	bBox, BYTE alpha, COLORREF color);
	void	DrawObjectInformation(RECT clientRect, SIZE SourceSize, VCA5_PACKET_OBJECT *pObject, BYTE alpha, COLORREF Color);

	void	ShowUI();

public:
	afx_msg void OnBnClickedBtnExport();
};
