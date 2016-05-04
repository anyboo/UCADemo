#pragma once
#include "D3DVCARender.h"
#include "CD3d.h"

class CCalibrationD3DRender : public IVCARender
{
public:

	CCalibrationD3DRender(void);
	virtual ~CCalibrationD3DRender(void);

	BOOL	Setup(HWND hWnd, CVCADataMgr *pDataMgr);
	void	Endup();
	void	OnChangeClientRect(RECT rect);
		
	BOOL	ChangeVCASourceInfo(BITMAPINFOHEADER *pbm);
	BOOL	RenderVCAData(BYTE *pImage, BITMAPINFOHEADER *pbm, ULONG ulRotate, CVCAMetaLib *pVCAMetaLib, RECT rcImageROI, RECT rcVCAROI, VCA5_TIMESTAMP *pTimestamp);
	
	void	SetRenderMode(UINT flags)		{ m_RenderMode = flags; m_VCARender.SetRenderMode(flags);}
	UINT	GetRenderMode()				{ return m_RenderMode; }

	BOOL	RenderVCAMetaData(BYTE *pImage, BITMAPINFOHEADER *pbm, ULONG ulRotate, BYTE *pMetadata, int nLength, RECT rcImageROI, RECT rcVCAROI, VCA5_TIMESTAMP *pTimestamp){
		return m_VCARender.RenderVCAMetaData(pImage, pbm, ulRotate, pMetadata, nLength, rcImageROI, rcVCAROI, pTimestamp);
	}

protected:
	void	DrawVCAData(VCA5_PACKET_OBJECTS *pObjects, VCA5_PACKET_EVENTS *pEvents, VCA5_PACKET_COUNTS *pCounters);
	void	SetCanvasSize( RECT* pCanvas );
	void	DrawGrid();
	
	UINT	m_RenderMode;
	CVCAD3DRender m_VCARender;

	// duplicated pointers corresponding to member variables of CVCAD3DRender
	CD3d*			m_pD3d;
	CVCADataMgr*	m_pDataMgr;
	RECT*			m_pClientRect;

	BYTE*	m_pBackBufImage;	
	BOOL	m_bPauseImageCopied;

	UINT	GetImageSize();

	void	DrawRulerMeasure(float fLength, int meaUnits, CPoint ptStartMsg);
	void	DrawMsg( POINT ptStartMsg,char* pszMsg,int alpha, COLORREF colour);

};
