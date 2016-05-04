#pragma once

#include "VCAConfigure.h"
#include "VCAMetaLib.h"
#include <vector>
#include <afxmt.h>
#include "Engine.h"

class CVCASystem;


class CVCAEngine : public CEngine
{
public:
	
	CVCAEngine(ULONG ulEngineId, CVCASystem *pVCASystem);
	~CVCAEngine();

	DWORD	Open();
	void	Close();		

	virtual BOOL	SetConfig(VCA5_APP_ZONES *pZones, VCA5_APP_RULES *pRules, VCA5_APP_COUNTERS *pCounters);
	virtual BOOL	SetClibInfo(VCA5_APP_CALIB_INFO *pCalibInfo, DWORD inputWidth, DWORD inputHeight);
	virtual BOOL	SetClassObjs(VCA5_APP_CLSOBJECTS *pClassObjs);
	virtual BOOL	SetControl(ULONG ulVCA5Cmd, ULONG ulParam0 = 0, ULONG ulParam1 = 0, ULONG ulParam2 = 0, ULONG ulParam3 = 0);
	
	virtual VCA5_ENGINE_PARAMS*	GetEngineParams(){return &m_EngineParams;}

//	ULONG	GetEngIndex(){return m_uEngineIndex;}
		

protected:

	virtual void OnNewFrame( unsigned char *pData, BITMAPINFOHEADER *pBih, VCA5_TIMESTAMP *pTimestamp );
	virtual void OnFormatChange( BITMAPINFOHEADER *pBih );
	
	BOOL	Setup(ULONG EngineIndex);
	void	Endup();

	void	VCAEngineClose();

	friend class CVCASystem;

protected:

	ULONG				m_ulEngId;
	CVCASystem			*m_pVCA5System;


	IVCA5*				m_pVCA5API;
	CVCAMetaLib*		m_pVCAMetaLib;
	VCA5_ENGINE_PARAMS	m_EngineParams;

	BOOL		Parse(BYTE *pResult, ULONG uLength);
};
