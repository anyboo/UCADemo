#include "StdAfx.h"
#include "GDIViewer.h"
#include <intrin.h>
#include "../common/colorconversion.h"
//#include <ipp.h>

#ifndef TRACE
#include <atlbase.h>
#define TRACE AtlTrace
#endif


void RotateImage(BYTE* pSrcImage, BYTE* pRorateImage, int width, int height, int Rotate)
{
	WORD* pSrcImageW	= (WORD *)pSrcImage;
	WORD* pRorateImageW	= (WORD *)pRorateImage;

	int x,y, temp;
	if(CGDIViewer::ROTATE_90 == Rotate || CGDIViewer::ROTATE_270 == Rotate){
		temp	= width;
		width	= height;
		height	= temp;
	}

	if(CGDIViewer::ROTATE_90 == Rotate){
		for(y = 0 ; y < height ; y++){
			for(x = 0 ; x < width ; x++){
				pRorateImageW[(height-y-1) + x*height] = pSrcImageW[x+y*width];
			}
		}
	}
	if(CGDIViewer::ROTATE_180 == Rotate){
		for(y = 0 ; y < height ; y++){
			for(x = 0 ; x < width ; x++){
				pRorateImageW[(width-x-1)+(height-y-1)*width] = pSrcImageW[x+y*width];
			}
		}
	}
	if(CGDIViewer::ROTATE_270 == Rotate){
		for(y = 0 ; y < height ; y++){
			for(x = 0 ; x < width ; x++){
				pRorateImageW[y + (width-x-1)*height] = pSrcImageW[x+y*width];
			}
		}
	}
}



#define CHECK_SETUP()\
	if(!m_bSetup){\
		TRACE("CGDIViewer Does not setup Before\n");\
		return FALSE;\
	}\


CGDIViewer::CGDIViewer()
{
	m_bSetup		= FALSE;
	m_BackBufDC		= NULL;
	m_BackBufBitmap	= NULL;
	m_pBackBufData	= NULL;
	m_pRotateImage	= NULL;
	m_pTempImage	= NULL;
	m_dstColorType	= COLORTYPE_RGB32;
}

CGDIViewer::~CGDIViewer()
{
	if(m_bSetup){
		Endup();
	}
}

BOOL	CGDIViewer::Setup(HWND hWnd, int width, int height, int srcColor, int dstColor, int Rotate)
{
	if(m_bSetup){
		TRACE("CGDIViewer Setuped Before\n");		
		Endup();
	}

	BOOL isSupportedColor = (COLORTYPE_YV12 == srcColor) || 
							(COLORTYPE_YUY2 == srcColor) ||
							(COLORTYPE_UYVY == srcColor) ||
							(COLORTYPE_RGB24 == srcColor)||
							(COLORTYPE_RGB16 == srcColor);
	if(!isSupportedColor){
		TRACE("CGDIViewer does not support ColorType [%d] \n", srcColor);
		return FALSE;
	}

	int bitCount = 32;	// default = COLORTYPE_RGB32
	if( COLORTYPE_RGB24 == dstColor ) {
		bitCount = 24;
	} else if( COLORTYPE_RGB16 == dstColor ) {
		bitCount = 16;
	}
	
	m_Rotate = Rotate;
	if( ROTATE_0 != Rotate ){
		m_pRotateImage = (BYTE *) _aligned_malloc(width*height*4, 16);
	}
	m_pTempImage = (BYTE *) _aligned_malloc(width*height*4 , 16);
	
	if( ROTATE_90 == Rotate || ROTATE_270 == Rotate){
		int temp = width;
		width = height;
		height= temp;	
	}

	if(!CreateBackBufDC(width, height, bitCount)){
		TRACE("CGDIViewer can not create DC[%d:%d] \n", width, height);
		return FALSE;
	}

	m_hWnd			= hWnd;
	m_nWidth		= width;
	m_nHeight		= height;
	m_srcColorType	= srcColor;
	m_dstColorType	= dstColor;

	m_bSetup		= TRUE;
	return m_bSetup;
}


void	CGDIViewer::Endup()
{
	DestroyBackBufDC();
	if(m_pRotateImage){
		_aligned_free(m_pRotateImage);
		m_pRotateImage = NULL;
	}
	if(m_pTempImage){
		_aligned_free(m_pTempImage);
		m_pTempImage = NULL;
	}
	m_bSetup = FALSE;
}


BOOL	CGDIViewer::DrawImage(BYTE *pImage)
{
	CHECK_SETUP();
	BYTE *pSrcImage = pImage;
	if(ROTATE_0 != m_Rotate){
		RotateImage(pImage, m_pRotateImage, m_nWidth, m_nHeight, m_Rotate);
		pSrcImage = m_pRotateImage;
	}

	return ColorConversion(pSrcImage, m_pBackBufData, m_srcColorType, m_dstColorType, m_nWidth, m_nHeight);
}

BOOL	CGDIViewer::DrawRect(RECT rc, COLORREF color, BOOL bFill, DWORD Alpha)
{
	CHECK_SETUP();

	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	SelectObject(m_BackBufDC, hPen);
	//DrawEdge(m_BackBufDC, &rc, BDR_RAISEDINNER|BDR_SUNKENOUTER, BF_RECT);
	MoveToEx(m_BackBufDC, rc.left, rc.top,NULL);
	LineTo(m_BackBufDC, rc.left, rc.bottom);
	LineTo(m_BackBufDC, rc.right, rc.bottom);
	LineTo(m_BackBufDC, rc.right, rc.top);
	LineTo(m_BackBufDC, rc.left, rc.top);

	DeleteObject(hPen);

	return TRUE;
}

BOOL CGDIViewer::DrawText( LPCTSTR lpszText, RECT rc, COLORREF color, int nFontHeightInPixel )
{
	CHECK_SETUP();

	LOGFONT lf;
	lf.lfHeight = (LONG)nFontHeightInPixel;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_BOLD;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH|FF_ROMAN;
//	lf.lfFaceName = (CHAR*)_T("Arial");
	_stprintf_s( lf.lfFaceName, _countof(lf.lfFaceName), _T("Arial") );

	HFONT hFont = CreateFontIndirect( &lf );

	::SetBkMode( m_BackBufDC, TRANSPARENT);
	//::SetBkColor( m_BackBufDC, color );
	::SetTextColor( m_BackBufDC, color );

	HFONT hFontPrev = (HFONT)SelectObject( m_BackBufDC, hFont );
	::DrawText( m_BackBufDC, lpszText, -1, &rc, DT_LEFT|DT_TOP );
	SelectObject( m_BackBufDC, hFontPrev );

	DeleteObject( hFont );

	return TRUE;
}

BOOL	CGDIViewer::DrawPixelMap( BYTE *pPixMap, DWORD dwWidth, DWORD dwHeight, COLORREF color )
{
	CHECK_SETUP();

	HBRUSH hBrush = CreateSolidBrush( color );
	
	int widthPix = m_nWidth / dwWidth;
	int heightPix = m_nHeight / dwHeight;

	BYTE *pBit = pPixMap;

	for( unsigned int y = 0; y < dwHeight; y++ )
	{
		for( unsigned int x = 0; x < dwWidth; x++ )
		{
			RECT rc;
			rc.left = x * widthPix;
			rc.right = (x+1) * widthPix;
			rc.top = y * heightPix;
			rc.bottom = (y+1) * heightPix;

			if( *pBit )
			{
				FillRect( m_BackBufDC, &rc, hBrush );
			}

			pBit++;
		}
	}

	DeleteObject( hBrush );

	return TRUE;
}
	

BOOL	CGDIViewer::DrawZone(DWORD PTCnt, POINT* pPTs, COLORREF color, BOOL bFill , DWORD Alpha )
{
	CHECK_SETUP();

	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	SelectObject(m_BackBufDC, hPen);
	//DrawEdge(m_BackBufDC, &rc, BDR_RAISEDINNER|BDR_SUNKENOUTER, BF_RECT);
	MoveToEx(m_BackBufDC, pPTs[0].x, pPTs[0].y,NULL);
	for(unsigned int i = 1 ; i < PTCnt ; i++){
		LineTo(m_BackBufDC, pPTs[i].x, pPTs[i].y);
	}
		
	DeleteObject(hPen);
	return TRUE;
}

BOOL	CGDIViewer::Update(int x, int y)
{
	CHECK_SETUP();
	
	HDC hDC = GetDC(m_hWnd);
	BitBlt(hDC, x, y, m_nWidth, m_nHeight, m_BackBufDC, 0, 0, SRCCOPY);
	ReleaseDC(m_hWnd, hDC);
	return TRUE;
}


BOOL	CGDIViewer::Update(HDC hDC, int x, int y)
{
	CHECK_SETUP();
	
	BitBlt(hDC, x, y, m_nWidth, m_nHeight, m_BackBufDC, 0, 0, SRCCOPY);
	return TRUE;
}



BOOL	CGDIViewer::CreateBackBufDC(int width, int height, int bitCount)
{
	BOOL		bSuccess	= FALSE;
	BITMAPINFO* pBitmapInfo	= NULL;
	HDC			hDC	= NULL;

	BITMAPINFOHEADER bif;
	memset(&bif, 0, sizeof(BITMAPINFOHEADER));
	bif.biSize			= sizeof(BITMAPINFOHEADER);
	bif.biWidth			= width;
	bif.biHeight		= -height;
	bif.biPlanes		= 1;
	bif.biBitCount		= bitCount;
	bif.biCompression	= BI_RGB;

	hDC = ::GetDC(NULL);
	m_BackBufBitmap	= CreateDIBSection(hDC, (BITMAPINFO*)&bif, DIB_RGB_COLORS, (void**)&m_pBackBufData, 0, 0);
	if(!m_BackBufBitmap){
		TRACE("Can Create DIBSection for Back Buffer\n");
		goto FAILED;
	}

	m_BackBufDC	= CreateCompatibleDC(hDC);
	SelectObject(m_BackBufDC, m_BackBufBitmap);
	
	return TRUE;

FAILED:
	DestroyBackBufDC();
	return FALSE;
}


void	CGDIViewer::DestroyBackBufDC()
{
	if(m_BackBufDC){
		DeleteDC(m_BackBufDC);
		m_BackBufDC	= NULL;
	}

	if(m_BackBufBitmap){
		 DeleteObject(m_BackBufBitmap);
		 m_BackBufBitmap	= NULL;
		 m_pBackBufData		= NULL;
	}
}


#define CLIP(value) ((value<0)?0:(value>255)?255:value)

BOOL	CGDIViewer::ColorConversion(BYTE* psrc, BYTE* pdst, int SrcColorType, int DstColorType, int iSrcWidth, int iSrcHeight)
{
	if((COLORTYPE_YUY2 == SrcColorType && COLORTYPE_RGB16 == DstColorType))
	{
		YUY2_To_RGB555(psrc, pdst, iSrcWidth*2, iSrcWidth, iSrcHeight);
	}
	else
	if((COLORTYPE_UYVY == SrcColorType && COLORTYPE_RGB16 == DstColorType))
	{
		unsigned short *pS = (unsigned short *)psrc;
		// UYVY is like YUY2 but with the bytes swapped at 16-bit level
		for( int i = 0; i < (iSrcWidth * iSrcHeight); i++, pS++ )
		{
			*pS = _byteswap_ushort( *pS );
		}
		YUY2_To_RGB555(psrc, pdst, iSrcWidth*2, iSrcWidth, iSrcHeight);

	}
/*
	else
	if((COLORTYPE_YUY2 == SrcColorType && COLORTYPE_RGB24 == DstColorType))
	{
		IppiSize roiSize;
		roiSize.width	= iSrcWidth;
		roiSize.height	= iSrcHeight;
		//IppStatus status = ippiYCbCr422ToRGB_8u_C2C3R(psrc, iSrcWidth*2, pdst, iSrcWidth*3, roiSize);
	}
*/
	else
	if((COLORTYPE_YUY2 == SrcColorType && COLORTYPE_RGB32 == DstColorType))
	{
		YUY2_To_RGB32_SSE(psrc, pdst, iSrcWidth*2, iSrcWidth, iSrcHeight);
	}
	else
	if((COLORTYPE_YV12 == SrcColorType && COLORTYPE_RGB16 == DstColorType))
	{
		BYTE* pY, *pU, *pV, *pULine, *pVLine;
		WORD *pDst16, RGB16;
		pY	= psrc;
		pV	= psrc+iSrcWidth*iSrcHeight+(iSrcWidth*iSrcHeight)/4;
		pU	= psrc+iSrcWidth*iSrcHeight;
		pDst16 = (WORD *)pdst;
		BYTE B,G,R;
		int i,j;
		int Y,U,V, U_save, V_save;

		for (i = 0; i < iSrcHeight; i++)
		{
			if(i%2 == 0)
			{
				pULine = pU + (iSrcWidth>>2)*i;
				pVLine = pV + (iSrcWidth>>2)*i;
			}
			
			for (j = 0; j < iSrcWidth ; j+=2)
			{
				Y = *pY++;
				U = *pULine++;
				V =	*pVLine++;			
				
				U_save = ((U-128)*475)>>8;
				V_save = ((V-128)*403)>>8;

				R = CLIP(V_save + Y);
				B = CLIP(U_save + Y);
				G = CLIP(Y + ((Y*51)>>7) - ((R*38+B*13)>>7));
				
				RGB16 = ((R>>3)<<10)|((G>>3)<<5)|(B>>3); 
				*pDst16++ = RGB16;

				Y = *pY++;
				R = CLIP(V_save + Y);
				B = CLIP(U_save + Y);
				G = CLIP(Y + ((Y*51)>>7) - ((R*38+B*13)>>7));
				
				RGB16 = ((R>>3)<<10)|((G>>3)<<5)|(B>>3); 
				*pDst16++ = RGB16;
			}
		}
	}
	else
	if( (COLORTYPE_RGB24 == SrcColorType) || (COLORTYPE_RGB16 == SrcColorType) )
	{
		memcpy(pdst, psrc, iSrcWidth * (SrcColorType == COLORTYPE_RGB24 ? 3 : 2) * iSrcHeight);
	}
	else
	{
		return FALSE;
	}
	
	return TRUE;
}

BYTE* CGDIViewer::GetImageBuffer()
{
	int bitCount = 32;	// default = COLORTYPE_RGB32
	if( COLORTYPE_RGB24 == m_dstColorType ) {
		bitCount = 24;
	} else if( COLORTYPE_RGB16 == m_dstColorType ) {
		bitCount = 16;
	}
	memcpy(m_pTempImage, m_pBackBufData, m_nWidth * m_nHeight * bitCount/8);
	return m_pTempImage;
}