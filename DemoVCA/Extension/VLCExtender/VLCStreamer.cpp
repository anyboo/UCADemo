#include "StdAfx.h"
#include "VLCStreamer.h"
#include <dshow.h>
#include <aviriff.h>


#define VLC_PATH _T("C:\\Program Files\\VideoLAN\\VLC\\vlc.exe")
#define VLC_ARGS _T("--sout=#transcode{vcodec=YUY2}:duplicate{dst=display,dst=std{access=file,mux=avi,dst=-}}")

VLCStreamer::VLCStreamer(void)
{
	//--------------------------------------
	// Me
	m_bGotMainHeader	= FALSE;
	m_bOpen				= FALSE;
	m_bRun				= FALSE;
	m_hChildStdOut_R	= NULL;
	m_hChildStdOut_W	= NULL;

	m_iTimestamp		= 0;

	memset( &m_piStreamer, 0, sizeof( PROCESS_INFORMATION ) );

	SetCallback( this );

	m_pAvimh	= new AVIMAINHEADER;
	m_pAvish	= new AVISTREAMHEADER;
	m_pAvibih	= new BITMAPINFOHEADER;

	// Set some sensible defaults
	memset( m_pAvimh, 0, sizeof( AVIMAINHEADER ) );
	memset( m_pAvish, 0, sizeof( AVISTREAMHEADER ) );
	memset( m_pAvibih, 0, sizeof( BITMAPINFOHEADER ) );

	m_hThreadEndedEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	m_pCallback = NULL;
}

VLCStreamer::~VLCStreamer(void)
{
}


//---------------------------------------------------------------------------------------------

HRESULT VLCStreamer::Open( IVideoStreamCallback *pCallback )
{
	HRESULT hr = E_FAIL;

	if( m_bOpen )
	{
		TRACE( _T("VLCPlayer already opened!\n" ));
		Close();
	}

	m_pCallback = pCallback;
	m_iTimestamp = 0;

	CString sCmdLine;
	sCmdLine.Format( _T("%s %s"), VLC_PATH, VLC_ARGS );

	SECURITY_ATTRIBUTES sa;
	memset( &sa, 0, sizeof( SECURITY_ATTRIBUTES ) );

	sa.nLength				= sizeof( SECURITY_ATTRIBUTES );
	sa.bInheritHandle		= TRUE;
	sa.lpSecurityDescriptor	= NULL;

	do
	{
		// Create a pipe to capture child process STDOUT
		if( !CreatePipe( &m_hChildStdOut_R, &m_hChildStdOut_W, &sa, 0 ) )
		{
			TRACE( _T("Unable to create pipe to child\n") );
			break;
		}

		// Ensure read handle to the pipe from STDOUT is not inherited
		if( !SetHandleInformation( m_hChildStdOut_R, HANDLE_FLAG_INHERIT, 0 ) )
		{
			TRACE( _T("Unable to adjust read handle inerit flags\n") );
			break;
		}

		// Start the child process
		STARTUPINFO si;
		memset( &si, 0, sizeof( STARTUPINFO ) );
		si.hStdOutput	= m_hChildStdOut_W;
		si.dwFlags		= STARTF_USESTDHANDLES;

		if( !CreateProcess( NULL, sCmdLine.GetBuffer(0), NULL, NULL, TRUE,
								NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &m_piStreamer ) )
		{
			TRACE( "CStreamVideoSource: Unable to create process: %s\n", sCmdLine );
			break;
		}

		m_bOpen = TRUE;

		// Now cue the stream up
		m_bGotMainHeader = FALSE;

		hr = StartVLCThread();

	}
	while( 0 );

	if( FAILED( hr ) )
	{
		// Clean up
		Close();
	}


	return hr;
}

//-----------------------------------------------------------------------------

HRESULT VLCStreamer::Close()
{

	if( m_piStreamer.hProcess )
	{
		TerminateProcess( m_piStreamer.hProcess, 0 );

		// Clean up the mess from the kids...
		CloseHandle( m_piStreamer.hProcess );
		CloseHandle( m_piStreamer.hThread );
		CloseHandle( m_hChildStdOut_W );
		CloseHandle( m_hChildStdOut_R );

		m_hChildStdOut_R = 0;
		m_hChildStdOut_W = 0;
		
		memset( &m_piStreamer, 0, sizeof( PROCESS_INFORMATION ) );

	}

	StopVLCThread();

	m_pCallback = NULL;

//	CloseHandle( m_hEvent );
//	m_hEvent = NULL;

	m_bOpen = FALSE;

	return S_OK;
}

//----------------------------------------------------------------------------

HRESULT VLCStreamer::StartVLCThread()
{
	m_bGotVideoFrame = FALSE;
	if( !m_bRun )
	{
		m_bRun = TRUE;
		AfxBeginThread( StreamThreadStub, (LPVOID) this );
	}

	return S_OK;
}

//----------------------------------------------------------------------------

HRESULT VLCStreamer::StopVLCThread()
{
	if( m_bRun )
	{
		m_bRun = FALSE;
		// Wait for stop
		VERIFY( WaitForSingleObject( m_hThreadEndedEvent, 10000 ) == WAIT_OBJECT_0 );
	}

	return S_OK;
}

//----------------------------------------------------------------------------

// static
UINT VLCStreamer::StreamThreadStub(LPVOID lpThis)
{
	((VLCStreamer *)lpThis)->StreamThread();
	return 0;
}

void VLCStreamer::StreamThread()
{
	while( m_bRun )
	{
		DWORD dwBytesRead, dwBytesToRead;

		unsigned char *pBuf = GetWritePtr();
		unsigned int uiBufLen = GetBufferSize();

		{
			dwBytesToRead = 10 * 1024;

			if( !ReadFile( m_hChildStdOut_R, pBuf, dwBytesToRead, &dwBytesRead, NULL ) )
			{
				TRACE( _T(" Unable to ReadFile\n") );
				break;
			}

			// Process the data
			DataAdded( dwBytesRead, NULL );
		}
	}

	SetEvent( m_hThreadEndedEvent );
}

//----------------------------------------------------------------------------
// AviDemuxCallback

void VLCStreamer::OnMainHeader( AVIMAINHEADER &avimh, void *pUserData )
{
	*m_pAvimh = avimh;

	// Sanity check
//	if( m_pAvimh->dwMicroSecPerFrame == 0 )
	{
		// Put a sensible default
		m_pAvimh->dwMicroSecPerFrame = 33333;
	}

	m_bGotMainHeader = TRUE;
}

void VLCStreamer::OnStreamHeader( AVISTREAMHEADER &avish, void *pUserData )
{
	m_bVideoFmtComing = FALSE;

	if( streamtypeVIDEO == avish.fccType )
	{
		*m_pAvish = avish;

		m_bVideoFmtComing = TRUE;
	}
}

void VLCStreamer::OnStreamFormat( unsigned char *pFmtBuf, unsigned int uiLen, void *pUserData )
{
	if( m_bVideoFmtComing )
	{
		memcpy( m_pAvibih, pFmtBuf, sizeof( BITMAPINFOHEADER ) );
	}
}

void VLCStreamer::OnNewFrame( DWORD dwFourCC, unsigned char *pBuf, unsigned int uiLen, void *pUserData )
{
	// See if it's a video stream

	if( (FCC('xxdc') & 0xFFFF0000) == (dwFourCC & 0xFFFF0000))
	{
		ASSERT( m_pCallback );
		m_pCallback->OnNewFrame( m_pAvibih, pBuf, uiLen, m_iTimestamp );


		// Timestamp in 100ns blocks
		m_iTimestamp += (m_pAvimh->dwMicroSecPerFrame * 10);
	}
}

void VLCStreamer::OnStreamEnd( void *pUserData)
{
	ASSERT( m_pCallback );
	m_pCallback->OnStreamEnd();
}

