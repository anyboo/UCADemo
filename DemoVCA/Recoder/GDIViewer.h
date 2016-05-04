#pragma once

class CGDIViewer
{
public:
	enum {COLORTYPE_UNKNOWN, COLORTYPE_YUY2, COLORTYPE_YV12, COLORTYPE_UYVY, COLORTYPE_RGB16, COLORTYPE_RGB24, COLORTYPE_RGB32};
	enum {ROTATE_0 = 0, ROTATE_90, ROTATE_180, ROTATE_270};
	CGDIViewer();
	~CGDIViewer();

	BOOL	Setup(HWND hWnd, int width, int height, int srcColor, int dstColor, int Rotate = ROTATE_0);
	void	Endup();

	BOOL	DrawImage(BYTE *pImage);
	//Current Not support Fill and Alpha.
	BOOL	DrawRect(RECT rc, COLORREF color, BOOL bFill = FALSE, DWORD Alpha = 255);
	BOOL	DrawText(LPCTSTR lpszText, RECT rc, COLORREF color, int nFontHeightInPixel );
	BOOL	DrawZone(DWORD PTCnt, POINT* pPTs, COLORREF color, BOOL bFill = FALSE, DWORD Alpha = 255);
	BOOL	DrawPixelMap( BYTE *pPixMap, DWORD dwWidth, DWORD dwHeight, COLORREF color );
	BOOL	Update(int x = 0 , int y = 0);
	BOOL	Update(HDC hDC, int x = 0 , int y = 0);
	BYTE*	GetImageBuffer();

private:

	BOOL	m_bSetup;
	
	int		m_nWidth;
	int		m_nHeight;
	int		m_srcColorType;
	int		m_dstColorType;
	int		m_Rotate;

	HWND	m_hWnd;
	HDC		m_BackBufDC;
	HPEN	m_hPen;
	HBITMAP	m_BackBufBitmap;
	BYTE*	m_pBackBufData;
	BYTE*	m_pRotateImage;
	BYTE*	m_pTempImage;

	BOOL	CreateBackBufDC(int width, int height, int bitCount);
	void	DestroyBackBufDC();

	//YV12 -> RGB15
	BOOL	ColorConversion(BYTE* pSrc, BYTE* pDst, int SrcColorType, int DstColorType, int width, int height);
};
