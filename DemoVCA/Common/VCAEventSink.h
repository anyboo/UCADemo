#pragma once

#include "VCAConfigure.h"
#include "VCAMetaLib.h"

class IVCAEventSink
{
public:
	virtual BOOL	ChangeVCASourceInfo(DWORD EngId, BITMAPINFOHEADER *pbm) = 0;
	virtual BOOL	ProcessVCAData(DWORD EngId, BYTE *pImage, BITMAPINFOHEADER *pbm, VCA5_TIMESTAMP *pTimestamp, BYTE *pMetaData, int iMataDataLen, CVCAMetaLib *pVCAMetaLib) = 0;
};
