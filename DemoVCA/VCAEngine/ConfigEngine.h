#pragma once

#include "VideoSource\VCAVideoSource.h"
#include "..\Extension\VCAMediaBundle.h"
#include <vector>
#include <deque>
#include <afxmt.h>
#include "Engine.h"

class IVCAEventSink;
class CVCAMetaLib;
interface IVCADataSource;

class ConfigEngine : public CEngine
{
public:
	ConfigEngine( ULONG ulEngineId );
	~ConfigEngine(void);

	BOOL AddMetadata( IVCAMediaSample *pSample );
	void SetDataSrc( IVCADataSource *pSrc );

	VCA5_ENGINE_PARAMS *GetEngineParams();

	BOOL Run();
	void Stop();

protected:
	void OnNewFrame( unsigned char *pData, BITMAPINFOHEADER *pBih, VCA5_TIMESTAMP *pTimestamp );

	virtual void Thread();
	
protected:

	CCriticalSection		m_csMetaLock;
	std::deque< IVCAMediaSample *>	m_metaSamples;
	IVCAMediaSample			*m_pCurMetaSamp;

	CVCAMetaLib		*m_pMetaLib;
	IVCADataSource	*m_pDataSrc;
	VCA5_ENGINE_PARAMS m_Params;
};
