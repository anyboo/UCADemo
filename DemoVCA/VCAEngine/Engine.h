#pragma once

#include <VCA5CoreLib.h>
#include <vector>
#include <afxmt.h>
#include "../Common/VCAConfigure.h"

class IVCAVideoSource;
class IVCAEventSink;

#define VCA5OPEN_FAIL	-1

class CEngine
{
public:
	CEngine( ULONG ulEngineId );
	virtual ~CEngine(void);

	enum VCA_ENGINE_STATUS
	{
		VCA_ENGINE_FREE,
		VCA_ENGINE_READY,
		VCA_ENGINE_WORK,
		VCA_ENGINE_STREAMING,
	};


	BOOL	SetVideoSource( IVCAVideoSource *pSrc );
	IVCAVideoSource *GetVideoSource() { return m_pVideoSource; }

	BOOL	RegisterVCAEventSink(IVCAEventSink *pVCAEventSink);
	BOOL	UnregisterVCAEventSink( IVCAEventSink *pVCAEventSink );

	virtual DWORD	Open();
	virtual void	Close();		

	virtual BOOL	Run();
	virtual void	Stop();

	ULONG GetStatus() { return m_ulStatus; }

	ULONG	GetEngId()	{return m_ulEngineId;}

	virtual BOOL	SetConfig(VCA5_APP_ZONES *pZones, VCA5_APP_RULES *pRules, VCA5_APP_COUNTERS *pCounters) { return FALSE; }
	virtual BOOL	SetClibInfo(VCA5_APP_CALIB_INFO *pCalibInfo, DWORD inputWidth, DWORD inputHeight)		{ return FALSE; }
	virtual BOOL	SetClassObjs(VCA5_APP_CLSOBJECTS *pClassObjs)											{ return FALSE; }
	virtual BOOL	SetControl(ULONG ulVCA5Cmd, ULONG ulParam0 = 0, ULONG ulParam1 = 0, ULONG ulParam2 = 0, ULONG ulParam3 = 0) { return FALSE; }
	
	virtual VCA5_ENGINE_PARAMS*	GetEngineParams() = 0;

//	ULONG	GetEngIndex(){return m_uEngineIndex;}

	ULONG	GetFunction() { return m_ulFunction;}


protected:
	

	static UINT ThreadStub( LPVOID lpThis );
	virtual void Thread();

	virtual void OnNewFrame( unsigned char *pData, BITMAPINFOHEADER *pBih, VCA5_TIMESTAMP *pTimestamp ) = 0;
	virtual void OnFormatChange( BITMAPINFOHEADER *pBih ) {;}
	BOOL CheckFormatChange();

protected:
	ULONG				m_ulEngineId;
	ULONG				m_ulStatus;
	ULONG				m_ulImgSize;
	ULONG				m_ulColorFmt;
	ULONG				m_ulFunction;

	
	BITMAPINFOHEADER	m_bih;

	IVCAVideoSource		*m_pVideoSource;

	std::vector< IVCAEventSink * > m_EventSinks;
	CCriticalSection			m_csSinkLock;

	HANDLE		m_hThread;
	HANDLE		m_hEndEvent;
};
