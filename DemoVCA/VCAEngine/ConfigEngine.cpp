#include "stdafx.h"
#include "ConfigEngine.h"
#include "VCAMetaParserMSXML.h"
#include "VCAEventSink.h"
#include "VCAConfigure.h"

#include <mmsystem.h>

#define MAX_NUM_META_SAMPLES 60

ConfigEngine::ConfigEngine( ULONG ulEngineId ) : CEngine( ulEngineId )
{
	m_pMetaLib		= CreateVCAMetaLib( CVCAMetaLib::MSXML_PARSER );

	m_pCurMetaSamp	= NULL;

	memset( &m_Params, 0, sizeof( m_Params ) );

	// Update Engine info with details from VCAConfigure - these have been loaded
	// by the XML passed in
	VCA5_APP_ENGINE_CONFIG *pConfig = CVCAConfigure::Instance()->GetEngineConf( ulEngineId );

	m_ulFunction	= pConfig->ulFunction;
	// Slightly frig - remove the metadata ability to force the use of VMR instead of internal drawing
//	m_EngineInfo.ulFunction &= ~VCA5_FEATURE_METADATA;
}

ConfigEngine::~ConfigEngine(void)
{
	delete m_pMetaLib;
}

BOOL ConfigEngine::AddMetadata(IVCAMediaSample *pSample)
{
	m_csMetaLock.Lock();

	pSample->AddRef();
	m_metaSamples.push_back( pSample );

	// Check for overflow
	while( m_metaSamples.size() > MAX_NUM_META_SAMPLES )
	{
		// Ditch them
		m_metaSamples.front()->Release();
		m_metaSamples.pop_front();
	}

	m_csMetaLock.Unlock();

	return TRUE;
}


VCA5_ENGINE_PARAMS *ConfigEngine::GetEngineParams()
{
	// Meaningless function since we have no real engine
//	VCACONFIG_CHANNEL_SRC src;
//	m_pDataSrc->GetChannelSource( m_ulEngineId, &src );

	return &m_Params;
}

void ConfigEngine::SetDataSrc( IVCADataSource *pSrc )
{
	m_pDataSrc = pSrc;
}

BOOL ConfigEngine::Run()
{
	// Start data source
	m_pDataSrc->StartStream( m_ulEngineId );

	return CEngine::Run();
}

void ConfigEngine::Stop()
{
	m_pDataSrc->StopStream( m_ulEngineId );

	CEngine::Stop();
}

void ConfigEngine::OnNewFrame( unsigned char *pData, BITMAPINFOHEADER *pBih, VCA5_TIMESTAMP *pTimestamp )
{
	BOOL bNewMeta = FALSE;
	IVCAMediaSample *pSamp = NULL;
	char *pszMeta = NULL;
	int iMetaLen = 0;
	int	iPeriodMs = 40;

	m_csMetaLock.Lock();

	__int64 tsVideo = pTimestamp->ulHighDateTime;
	tsVideo <<= 32;
	tsVideo |= pTimestamp->ulLowDateTime;

	// Try and find the closest matching metadata sample for this video frame
	IVCAMediaSample *pBestMatch = NULL;
	unsigned int uiSmallestDiff = 0xFFFFFFFF;
	std::deque< IVCAMediaSample *>::iterator it;
	for( it = m_metaSamples.begin(); it != m_metaSamples.end(); it++ )
	{
		pSamp = *it;
		VCACONFIG_MEDIASAMPLEHDR hdr;
		pSamp->GetHeader( hdr );

		__int64 tsMeta = hdr.timeStamp;


		if( tsMeta > tsVideo )
		{
			// This timestamp is in the future (i.e. greater than the videots), then just display nothing
			// we'll come back later and get this
			pBestMatch = pSamp;

			// TBD: Get the video to slow down a bit
			break;
		}
		
		__int64 iDiff = tsVideo - tsMeta;
		iDiff /= 10000; // 100ns -> ms
		unsigned int uiDiffMs = abs((int)iDiff);

		if( uiDiffMs < uiSmallestDiff )
		{
			uiSmallestDiff = uiDiffMs;

			pBestMatch = pSamp;
		}
	}


	// Now go through and remove all samples up to the best match
	if( pBestMatch )
	{
		while( m_metaSamples.size() )
		{
			if( pBestMatch == m_metaSamples.front() )
			{
				break;
			}

			// This sample is older than the current video sample
			// and there is a better match elsewhere. Remove it
			m_metaSamples.front()->Release();
			m_metaSamples.pop_front();
		}

		pBestMatch->GetDataPtr( (void **)&pszMeta, iMetaLen );

		if( m_pCurMetaSamp )
		{
			// No longer needed
			m_pCurMetaSamp->Release();
		}
		m_pCurMetaSamp = pBestMatch;

		// In case it gets removed from the queue while it's in use (due to overflow)
		m_pCurMetaSamp->AddRef();
	}

	m_csMetaLock.Unlock();

	if( pszMeta )
	{
		m_pMetaLib->ParseMetaData( (unsigned char *)pszMeta, iMetaLen );
	}

	m_csSinkLock.Lock();
	// Now display - pass to each sink
	std::vector< IVCAEventSink *>::iterator sinkIt;
	for( sinkIt = m_EventSinks.begin(); sinkIt != m_EventSinks.end(); sinkIt++ )
	{
		(*sinkIt)->ProcessVCAData( 0, pData, pBih, pTimestamp, (BYTE *)pszMeta, iMetaLen, m_pMetaLib );
	}
	m_csSinkLock.Unlock();

}


void ConfigEngine::Thread()
{
	DWORD				bEOF;
	DWORD				uTimeOut;
	BYTE				*pData;
	FILETIME			timestamp;
	BOOL				bRun = TRUE;
	DWORD				dwFrameRate = 0;
	__int64 iRefClock = 0;
	__int64 iStreamClock = 0;
	unsigned int uiSleepTime = 33;
	__int64 iLastStreamTime = 0;
	__int64 iLastRefTime = 0;
	float				fAvgTimeBetweenFrames = 0;

	float				fWaitTime = 33.0f;

	CoInitialize(NULL);

	m_pVideoSource->Start();

	VERIFY( m_pVideoSource->Control( IVCAVideoSource::CMD_GET_FRAMERATE, (DWORD)&dwFrameRate, 0 ) );

	//CheckFormatChange();

	ASSERT( dwFrameRate > 0 );
	// Prevent divide by 0 error
	if( !dwFrameRate ) dwFrameRate = 30;

	uTimeOut	= (100000/(dwFrameRate));

	HANDLE	hEvents[2];
	hEvents[0] =	m_hEndEvent;
	hEvents[1] =	m_pVideoSource->GetEvent();

	if( m_pVideoSource->GetEvent() )
	{
		// Using event, not timeout
		uTimeOut = INFINITE;
	}

	while( bRun )
	{
		switch( WaitForMultipleObjects( 2, hEvents, FALSE, uTimeOut ) )
		{
			case WAIT_OBJECT_0:
			{
				bRun = FALSE;
			}
			break;

			//-----------------------------------------------------------------------------
			// SIMPLE streaming
			case WAIT_TIMEOUT:
			{
				if( m_pVideoSource->GetRawImage(&pData, &timestamp, &bEOF))
				{
					// Got one frame, must be streaming
					m_ulStatus = VCA_ENGINE_STREAMING;

					CheckFormatChange();

					OnNewFrame( pData, &m_bih, (VCA5_TIMESTAMP *)&timestamp );

					m_pVideoSource->ReleaseRawImage();
				}
				else
				{
					TRACE("Engine[%d] Can not GetRawImage \n", m_ulEngineId);

					// Check for EOF
					if( bEOF )
					{
						bRun = FALSE;
					}
				}
			}
			break;

			//-----------------------------------------------------------------------------
			// ADAPTIVE streaming
			case WAIT_OBJECT_0+1:
			{

				while(bRun && m_pVideoSource->GetRawImage(&pData, &timestamp, &bEOF))
				{
					// Got one frame, must be streaming
					m_ulStatus = VCA_ENGINE_STREAMING;

					if( !iRefClock ) iRefClock = ((__int64)GetTickCount()) * 10000;
					int iTicks = GetTickCount();

					__int64 iRefDiff = (((__int64)iTicks) * 10000) - iRefClock;


					__int64 ts = timestamp.dwHighDateTime;
					ts <<= 32;
					ts |= timestamp.dwLowDateTime;


					if( !iStreamClock ) iStreamClock = ts;

					__int64 iStreamDiff = ts - iStreamClock;

					// Now, the time we have to wait is the difference between stream and reference time (stream time in the future)
					int iDiff = (int)(iStreamDiff/10000) - (int)(iRefDiff/10000);

					__int64 iLastDiff = ts - iLastStreamTime;
					__int64 iLastRefDiff = (__int64)iTicks - iLastRefTime;

	//				TRACE( _T("REFDIFF: %I64d STREAMDIFF: %I64d in ms: refdiff:%d streamdiff:%d FINALDIFF:%d LASTSTREAMDIFF:%d LASTREFDIFF:%d\n"),
	//						iRefDiff, iStreamDiff, (int)(iRefDiff/10000), (int)(iStreamDiff/10000), iDiff, (int)(iLastDiff/10000), (int)(iLastRefDiff) );


					if( !fAvgTimeBetweenFrames ) fAvgTimeBetweenFrames = 33.0f;//(float)(iLastDiff/10000);

					fAvgTimeBetweenFrames = (0.9f * fAvgTimeBetweenFrames) + (0.1f * (float)(iLastDiff/10000));

					iLastStreamTime = ts;
					iLastRefTime = iTicks;

					if( abs(iDiff) > 1000 )
					{
						// Way off the mark here, reset
						TRACE( _T("-------RESETTING----------\n"));

						iRefClock = 0;
						iStreamClock = 0;
						iDiff = 10;
					}
					else
//					if( iDiff < 0 )
//					{
//						OutputDebugString( _T("FRAME WAS LATE, DISPLAY NOW\n"));
//						// our reference clock is running ahead of the stream clock (in the future)
//						// schedule straight away to try and catch up
//				//		iRefClock += (40 * 10000);
//
//				//		iDiff = 0;
//					}
//					else
//					if( iDiff > 100 )
//					{
//						OutputDebugString( _T("FRAME WAS VERY EARLY\n"));
//						// our reference clock is too far ahead of the frame clock, try to slow down a bit
//	//					iRefClock += (40 * 10000);
//					}
//					else
					{
						// Everything seems to be in order, so try and catch up a bit, depending on the size of the video buffer
				//		OutputDebugString(_T("----OK\n"));

						unsigned int uiMaxLen, uiCurLen;
						if( m_pVideoSource->GetBufferLen( &uiMaxLen, &uiCurLen ) )
						{
							// Figure out how full the buffer is
							float fFullPct = (float)uiCurLen/(float)uiMaxLen;

							unsigned int uiMsInBuffer = uiCurLen * (unsigned int)fAvgTimeBetweenFrames;

					//		CString s; s.Format( _T("AVG frame time: %dms. MS in buffer: %d\n"), (int)fAvgTimeBetweenFrames, uiMsInBuffer );
					//		OutputDebugString(s);


							// Try to maintain at least 500ms in the buffer
							if( uiCurLen > (uiMaxLen/2) )//uiMsInBuffer >= 500 && uiCurLen > 1)
							{
								iRefClock -= ( 10000);
					//			OutputDebugString(_T("SPEED UP\n"));
							}
							else
							if( uiCurLen < (uiMaxLen/2))//uiMsInBuffer < 200 && uiCurLen > 1 )
							{
								iRefClock += 10000;
								// Slow down a bit, getting a bit close to the edge
					//			OutputDebugString(_T("SLOW DOWN\n"));
							}
						}
						else
						{
							// Video source needs to provide this info to use adaptive streaming
							ASSERT( FALSE );
						}
					}

					// Update waiting time
					uiSleepTime = max( 0, min( 1000, iDiff ));

					// Wait for this long
				//	CString s; s.Format(_T("WAIT:%d\n"), uiSleepTime );
				//	OutputDebugString(s);
					switch( WaitForSingleObject( m_hEndEvent, uiSleepTime ) )
					{
						case WAIT_OBJECT_0:
						{
							bRun = FALSE;
						}
						break;

						case WAIT_TIMEOUT:
						{
							// Deliver the frame
							CheckFormatChange();

							OnNewFrame( pData, &m_bih, (VCA5_TIMESTAMP *)&timestamp );

							m_pVideoSource->ReleaseRawImage();
						}
						break;
					}
				}
			}
			break;
			
		}
	}

	m_ulStatus = VCA_ENGINE_READY;

	m_pVideoSource->Stop();

	TRACE("End of Engine [%d] \n", m_ulEngineId);
	CoUninitialize();
}

/*
// static
UINT ConfigEngine::SyncThreadStub(LPVOID lpThis)
{
	((ConfigEngine *)lpThis)->SyncThread();

	return 0;
}

void ConfigEngine::SyncThread()
{
	// Do the shizzle
	BOOL bRun = TRUE;

	CheckFormatChange();

	DWORD dwWaitTime = 30;	// TBD - put correct rate in here based on FPS

	// Start it up
	m_pVideoSource->Start();

	HANDLE hEvents[2];
	hEvents[0]	= m_hStopThreadEvent;
	hEvents[1]	= m_pVideoSource->GetEvent();

	while( bRun )
	{
		switch( WaitForMultipleObjects( sizeof( hEvents ) / sizeof( hEvents[0]), hEvents, FALSE, dwWaitTime ) )
		{
			case WAIT_OBJECT_0:
			{
				// Die die die
				bRun = FALSE;
			}
			break;

			case WAIT_OBJECT_0+1:
			case WAIT_TIMEOUT:
			{
				BYTE		*pImage = NULL;
				char		*pszMeta = NULL;
				int			iMetaLen = 0;
				FILETIME	vidTs, metaTs;
				DWORD		bEof;
				BOOL		bNewMeta = FALSE;

				// Timed out... display the frame
				if( m_pVideoSource->GetRawImage( &pImage, &vidTs, &bEof ) )
				{
					// Check for change in image size
					CheckFormatChange();

					// See if there's any metadata
					IVCAMediaSample *pSamp = NULL;
					
					m_csMetaLock.Lock();
					// TBD: do proper sync of meta and video based on TS

					// Empty out all but the last one
					while( m_metaSamples.size() > 1 )
					{
						// Another meta frame has been added, scoot up to that one
						bNewMeta = TRUE;

						pSamp = m_metaSamples.front();
						pSamp->Release();
						pSamp = NULL;

						m_metaSamples.pop( );
					}

					if( !m_metaSamples.empty() )
					{
						pSamp = m_metaSamples.front();

						if( pSamp )
						{
							pSamp->GetDataPtr( (void **) &pszMeta, iMetaLen );
						}
					}

					m_csMetaLock.Unlock();

				//	if( bNewMeta )
		//			{
		//				// Parse it
		//				m_pMetaLib->ParseMetaData( (unsigned char *)pszMeta, iMetaLen );
		//			}


					// Now display - pass to each sink
					std::vector< IVCAEventSink *>::iterator sinkIt;
					for( sinkIt = m_eventSinks.begin(); sinkIt != m_eventSinks.end(); sinkIt++ )
					{
						(*sinkIt)->ProcessVCAData( 0, pImage, &m_bih, (BYTE *)pszMeta, iMetaLen, m_pMetaLib );
					}

					m_pVideoSource->ReleaseRawImage();

					if( pSamp )
					{
					//	pSamp->Release();
					}

				}
				else
				{
					if( bEof )
					{
						bRun = FALSE;
					}
					else
					{
						continue;
					}
				}
			}
			break;

			default:
			{
				// Erk??
				ASSERT( FALSE );
			}
			break;
		}
	}

	m_pVideoSource->Stop();

	SetEvent( m_hThreadStoppedEvent );
}
*/