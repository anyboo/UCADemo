#include "stdafx.h"
#include "Engine.h"
#include "VideoSource/VCAVideoSource.h"
#include "../Common/VCAEventSink.h"
#include <mmsystem.h>

static BOOL waitWithMessageLoop(HANDLE hEvent, DWORD dwTimeout)
{
  DWORD dwRet;
  MSG msg;
    
  while(true)
  {
    dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwTimeout, QS_ALLINPUT);
    if (dwRet == WAIT_OBJECT_0)
       return TRUE;
    if (dwRet != WAIT_OBJECT_0 + 1)
       break;
    while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
         return TRUE;
    }
  }
  return FALSE;
}


CEngine::CEngine( ULONG ulEngineId )
{
	m_ulEngineId =	ulEngineId;
	m_pVideoSource	= NULL;
	m_ulStatus		= VCA_ENGINE_FREE;

	m_hEndEvent		= CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hThread		= NULL;

	m_ulImgSize		= 0;
	m_ulColorFmt	= 0;

	memset( &m_bih, 0, sizeof( BITMAPINFOHEADER ) );
	m_ulFunction	= 0;
}

CEngine::~CEngine(void)
{
	CloseHandle( m_hEndEvent );
}


BOOL CEngine::SetVideoSource( IVCAVideoSource *pSource )
{
	m_pVideoSource	= pSource;
	return TRUE;
}

BOOL CEngine::RegisterVCAEventSink(IVCAEventSink *pVCAEventSink)
{
	if (pVCAEventSink)
	{
		m_csSinkLock.Lock();
#ifdef _DEBUG
		std::vector< IVCAEventSink * >::iterator it;
		for( it = m_EventSinks.begin(); it != m_EventSinks.end(); it++ )
		{
			if( *it == pVCAEventSink )
			{
				// Already registered!
				ASSERT( FALSE );
				break;
			}
		}
#endif // _DEBUG

		m_EventSinks.push_back( pVCAEventSink );

		m_csSinkLock.Unlock();
	}
	return FALSE;
}

BOOL CEngine::UnregisterVCAEventSink(IVCAEventSink *pVCAEventSink)
{
	BOOL bRet = FALSE;
	m_csSinkLock.Lock();

	std::vector< IVCAEventSink *>::iterator it;
	for( it = m_EventSinks.begin(); it != m_EventSinks.end(); it++ )
	{
		if( *it == pVCAEventSink )
		{
			m_EventSinks.erase( it );
			bRet = TRUE;
			break;
		}
	}

	m_csSinkLock.Unlock();

	return bRet;
}

DWORD CEngine::Open()
{
	if (VCA_ENGINE_FREE != m_ulStatus)
	{
		return FALSE;
	}
	m_ulStatus	= VCA_ENGINE_READY;
	return TRUE;
}

void CEngine::Close()
{
	if (VCA_ENGINE_WORK > m_ulStatus) {
		return;
	}

	Stop(); 
	m_ulStatus = VCA_ENGINE_FREE;
}


BOOL CEngine::Run()
{
	if (VCA_ENGINE_READY != m_ulStatus)
	{
		return FALSE;
	}

	CWinThread *pThread;
	VERIFY( pThread = AfxBeginThread( ThreadStub, (LPVOID)this) );
	
	m_hThread	= pThread->m_hThread;
	m_ulStatus	= VCA_ENGINE_WORK;
	return TRUE;
}

void CEngine::Stop()
{
	if (VCA_ENGINE_WORK > m_ulStatus) {
		return;
	}

	TRACE(_T("CVCAEngine::Stop Engie Id[%d] \n"), m_ulEngineId);
	SetEvent( m_hEndEvent );

	if(WaitForSingleObject( m_hThread, 5000 ) != WAIT_OBJECT_0){
		TRACE(_T("Timeout :: Terminate thread Engie Id[%d] \n"), m_ulEngineId);
		TerminateThread(m_hThread, 0);

	}
	
	m_ulStatus = VCA_ENGINE_READY;
}

BOOL CEngine::CheckFormatChange()
{
	BITMAPINFOHEADER	bm;
	memset( &bm, 0, sizeof( BITMAPINFOHEADER ) );
	bm.biSize = sizeof( BITMAPINFOHEADER );

	unsigned long ulImgSize;
	unsigned long ulColorFmt;

	m_pVideoSource->Control( IVCAVideoSource::CMD_GET_IMGSIZE, (DWORD) &ulImgSize, 0 );
	m_pVideoSource->Control( IVCAVideoSource::CMD_GET_COLORFORMAT, (DWORD) &ulColorFmt, 0 );

	if( (ulImgSize != m_ulImgSize) || (ulColorFmt != m_ulColorFmt) )
	{
		bm.biWidth	= VCA5_GETIMGWIDTH(ulImgSize);
		bm.biHeight	= VCA5_GETIMGHEIGHT(ulImgSize);

		switch (ulColorFmt)
		{
		case VCA5_COLOR_FORMAT_YUY2:
			bm.biCompression = mmioFOURCC('Y','U','Y','2');
			break;
		case VCA5_COLOR_FORMAT_YV12:
			bm.biCompression = mmioFOURCC('Y','V','1','2');
			break;
		case VCA5_COLOR_FORMAT_UYVY:
			bm.biCompression = mmioFOURCC('U','Y','V','Y');
			break;
		case VCA5_COLOR_FORMAT_RGB16:
			bm.biCompression = BI_RGB;
			bm.biBitCount = 16;
			break;
		case VCA5_COLOR_FORMAT_RGB24:
			bm.biCompression = BI_RGB;
			bm.biBitCount = 24;
			break;
		default:
			ASSERT(0);
		}

		// Sort myself and any derived classes out first
		OnFormatChange( &bm );

		// Now tell all the listeners
		m_csSinkLock.Lock();
		std::vector< IVCAEventSink * >::iterator sinkIt;
		for( sinkIt = m_EventSinks.begin(); sinkIt != m_EventSinks.end(); sinkIt++ )
		{
			(*sinkIt)->ChangeVCASourceInfo( m_ulEngineId, &bm );
		}
		m_csSinkLock.Unlock();

		m_ulColorFmt = ulColorFmt;
		m_ulImgSize  = ulImgSize;

		m_bih = bm;

		return TRUE;
	}

	return FALSE;
}

// static
UINT CEngine::ThreadStub(LPVOID lpThis)
{
	((CEngine *)lpThis)->Thread();
	return 0;
}

void CEngine::Thread()
{
	DWORD				bEOF;
	DWORD				uTimeOut;
	BYTE				*pData;
	FILETIME			timestamp;
	BOOL				bRun = TRUE;
	DWORD				dwFrameRate = 0;

	CoInitialize(NULL);

	m_pVideoSource->Start();

	VERIFY( m_pVideoSource->Control( IVCAVideoSource::CMD_GET_FRAMERATE, (DWORD)&dwFrameRate, 0 ) );

//	CheckFormatChange();

	//ASSERT( dwFrameRate > 0 );
	// Prevent divide by 0 error
	if( !dwFrameRate ) dwFrameRate = 30;

	HANDLE	hEvents[2];
	hEvents[0] =	m_hEndEvent;
	hEvents[1] =	m_pVideoSource->GetEvent();
	DWORD dwNumHandles;

	if( hEvents[1] )
	{
		// Scheduling will be done by video source - they tell us when frame is ready by firing event
		uTimeOut = 1000;//INFINITE;

		dwNumHandles = 2;
	}
	else
	{
		// We do the scheduling and check back at regular intervals to retrieve the next frame
		uTimeOut = (1000/(dwFrameRate));

		dwNumHandles = 1;
	}

	int iFrames = 0;
	while( bRun ) 
	{
		DWORD dwResult = WaitForMultipleObjects(dwNumHandles, hEvents, FALSE, uTimeOut);
		switch( dwResult )
		{
			case WAIT_OBJECT_0:
			{
				// Die die die
				bRun = FALSE;
				TRACE(_T("CEngine::Thread DIE DIE DIE Engine Id[%d] \n"), m_ulEngineId);
			}
			break;

			case WAIT_OBJECT_0+1:
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

		}
	}

	m_pVideoSource->Stop();
	

	TRACE("End of Engine [%d] \n", m_ulEngineId);
	CoUninitialize();
}