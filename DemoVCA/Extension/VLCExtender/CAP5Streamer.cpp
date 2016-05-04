#include "StdAfx.h"
#include "CAP5Streamer.h"

#define	_V(x) \
	do { \
		if (x==0) { \
			TRACE("%s return 0\n", #x); \
			return FALSE; \
		} \
	} while (0)


#define MAX_MODEL	8
DVRAUTHINFO g_ModelAuthInfo[MAX_MODEL] = {
		{0x01AC,{0x7f, 0x29, 0x6d, 0x41, 0xd9, 0x14, 0x8f, 0x8a, 0xa0, 0x1b, 0xfb, 0x3c, 0x5f, 0x5f, 0x81, 0xfb}},	//ECPR480-16
		{0x01AB,{0xb6, 0x79, 0xb2, 0x37, 0x85, 0x43, 0xda, 0x14, 0x32, 0x05, 0x8b, 0x84, 0xb1, 0x12, 0x5f, 0x27}},	//ECPR240-16
		{0x01AA,{0xcb, 0xab, 0xac, 0x73, 0xbd, 0xaa, 0xcb, 0x5b, 0x3b, 0x8a, 0xa0, 0xa9, 0x80, 0x28, 0x25, 0xa5}},	//ECPR120-4
		{0x01A6,{0xd6, 0x5f, 0xb2, 0x80, 0x2c, 0xd1, 0xc7, 0x96, 0xc5, 0xfc, 0x4b, 0x64, 0x04, 0x46, 0xc3, 0x8d}},	//MP2000
		{0x01A7,{0x3b, 0x79, 0xd5, 0x7c, 0x51, 0xe1, 0x18, 0xca, 0xd9, 0x9f, 0xad, 0x7e, 0xc4, 0xbc, 0x40, 0x25}},	//MP3000
		{0x01B6,{0xe0, 0x93, 0x3e, 0x4b, 0xd7, 0x9d, 0x11, 0x8f, 0xe6, 0x85, 0xae, 0xb2, 0xe7, 0x8c, 0xcb, 0xc2}},	//MP2000L
		{0x01B7,{0x28, 0x6d, 0x52, 0xe4, 0x2e, 0x9e, 0x68, 0x3f, 0x22, 0xdd, 0x43, 0x24, 0x09, 0x63, 0x66, 0x68}},	//MP3000L
		{0x01A8,{0x2e, 0x79, 0xfb, 0x57, 0x72, 0x08, 0x09, 0x76, 0x52, 0x68, 0x3e, 0xaf, 0x66, 0x5d, 0x76, 0x9b}},	//MP4000L
};

#define DLLNAME _T("ECPSV.dll")
#define MAX_NUM_CHANNEL 8


#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)


CAP5Streamer::CAP5Streamer( CString sVideoData )
{
	BOOL bOk = FALSE;
	m_pCallback = NULL;

	m_hStopThreadEvent	= CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hThreadStoppedEvent	= CreateEvent( NULL, FALSE, FALSE, NULL );

	m_pCapBoardInfo = NULL;
	m_nCapBoards = 0;

	// Parse input data
	int iStart = 0;
	int iToken = 0;
	CString tokens[10];
	while( iStart > -1 )
	{
		tokens[iToken++] = sVideoData.Tokenize( _T(":"), iStart );
	}

	// Now figure it all out
	m_bd = _ttoi( tokens[0].GetBuffer(0) );
	m_ch = _ttoi( tokens[1].GetBuffer(0) );
	if( 0 == tokens[2].Compare( _T("pal") ) )
	{
		m_videoStd = CMN5_VIDEO_FORMAT_PAL_B;
		m_iVideoWidth = 720;
		m_iVideoHeight = 576;
	}
	else
	{
		m_videoStd = CMN5_VIDEO_FORMAT_NTSC_M;
		m_iVideoWidth = 720;
		m_iVideoHeight = 480;
	}

	m_iFrameRate = _ttoi( tokens[3].GetBuffer(0) );


	bOk = InitCapApi( DLLNAME, g_ModelAuthInfo, MAX_MODEL );

	if( !bOk )
	{
		AfxMessageBox( _T("Unable to initialize CAP5 API.\nAre all the DLLs in the right place?") );
	}
}

CAP5Streamer::~CAP5Streamer(void)
{
	CloseHandle( m_hStopThreadEvent );
	CloseHandle( m_hThreadStoppedEvent );

}

HRESULT CAP5Streamer::Open( IVideoStreamCallback *pCallback )
{
	m_pCallback = pCallback;

	if( !m_pCapApi ) return E_FAIL;

	_V(m_pCapApi->Cap5Setup());
	_V(m_pCapApi->Cap5SetCaptureMethod(CMN5_CAPTURE_TYPE_ON_DATA, CMN5_CAPTURE_METHOD_QUERY));

	_V(SetProperty());

	_V(m_pCapApi->Cap5Run());
	
	_V(SetAdjust());

	// Run the streaming thread
	AfxBeginThread( MediaThreadStub, (LPVOID) this );

	return S_OK;
}

HRESULT CAP5Streamer::Close()
{
	SetEvent( m_hStopThreadEvent );

	VERIFY( WaitForSingleObject( m_hThreadStoppedEvent, 5000 ) == WAIT_OBJECT_0 );

	return S_OK;
}

BOOL CAP5Streamer::Uda5CreateInstance(HMODULE hLib,REFIID riid,void ** ppInterface)
{
	if (hLib)
	{
		BOOL rs;
		IUnknown* pUnknown;
		BOOL (FAR WINAPI*_CreateInstance)(IUnknown ** ppInterface);
		FARPROC test_proc=GetProcAddress(hLib,"Cmn5CreateInstance");
		if (test_proc) {
			*(FARPROC*)&_CreateInstance=test_proc;
			
			
			rs=(*_CreateInstance)(&pUnknown);
			
			if (rs) {
				HRESULT hr;
				hr=pUnknown->QueryInterface(riid,ppInterface);
				pUnknown->Release();
				if (SUCCEEDED(hr))
					return TRUE;
			}
		}
	}
	
	return FALSE;
}


BOOL CAP5Streamer::InitCapApi(LPCTSTR lpszDllName, DVRAUTHINFO AuthCode[], int nAuthCount)
{
	HMODULE hLib=NULL;
	
	if (lpszDllName) {
		_V((hLib=LoadLibrary(lpszDllName)));
	}
	if (Uda5CreateInstance(hLib,IID_ICap5,(void**)&m_pCapApi)) {
		CMN5_SYSTEM_INFO sysInfo;
		_V(m_pCapApi->Cap5GetSystemInfo(&sysInfo));

		m_nCapBoards = sysInfo.uNumOfBoard;
		m_pCapBoardInfo=new CAP5_BOARD_INFO[m_nCapBoards];
		CMN5_BOARD_INFO_DESC desc={sizeof(CAP5_BOARD_INFO),1,0,};

		BOOL bActivate=FALSE;
		for(int i=0;i<m_nCapBoards;i++) {
			_V(m_pCapApi->Cap5GetBoardInfo(i,&desc,&m_pCapBoardInfo[i]));

			for (int j=0; j<nAuthCount; j++) {
				if (AuthCode[j].hwId == m_pCapBoardInfo[i].uModelID) {
					_V(m_pCapApi->Cap5Activate(i, AuthCode[j].activationCode));
					bActivate = TRUE;
					break;
				}
			}
			if (!bActivate) {
				TRACE("ModelID is not found!!\n");
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CAP5Streamer::SetProperty(void)
{
	int		bd, ch;

	for(bd=0;bd<m_nCapBoards;bd++) {
		for(ch=0;ch<(int)m_pCapBoardInfo[bd].uMaxChannel;ch++) {
			_V(m_pCapApi->Cap5VideoEnable(bd,ch,FALSE));
		}
	}
	
	// Cap5	
	for(bd=0;bd<m_nCapBoards;bd++) {
		_V(m_pCapApi->Cap5SetVideoFormat(bd,m_videoStd));
		_V(m_pCapApi->Cap5SetColorFormat(bd,CAP5_COLOR_FORMAT_YUY2));
		//for(ch=0;ch<(int)m_pCapBoardInfo[bd].uMaxChannel;ch++) {
		for(ch=0;ch<MAX_NUM_CHANNEL;ch++) {
			_V(m_pCapApi->Cap5SetImageSize(bd,ch,CMN5_MAKEIMGSIZE(m_iVideoWidth,m_iVideoHeight)));
			
			_V(m_pCapApi->Cap5VideoEnable(bd,ch,TRUE));
			
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_BRIGHTNESS,128,0,0,0));
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_CONTRAST,128,0,0,0));
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_HUE,128,0,0,0));
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_SATURATION_U,128,0,0,0));
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_SATURATION_V,128,0,0,0));
		}

		ULONG	uDoData=0xffffffff;
		_V(m_pCapApi->Cap5SetDO(bd,16,&uDoData));
	}

	return TRUE;
}

BOOL CAP5Streamer::SetAdjust(void)
{
	int		bd, ch;
	int		use_userseq=0;

	// Cap5
	for(bd=0;bd<m_nCapBoards;bd++) {
		if (use_userseq) {
			BYTE framerate[MAX_NUM_CHANNEL]={10,20,10,20,10,20,10,20};
			_V(m_pCapApi->Cap5SetFrameRate(bd,CAP5_FRC_USER_FIXED,framerate,0));
		}else{
			_V(m_pCapApi->Cap5SetFrameRate(bd,CAP5_FRC_AUTO_EVEN,NULL,0));
		}

		for(ch=0;ch<MAX_NUM_CHANNEL;ch++) {
			_V(m_pCapApi->Cap5VideoEnable(bd,ch,TRUE));
			
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_BRIGHTNESS,128,0,0,0));
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_CONTRAST,128,0,0,0));
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_HUE,128,0,0,0));
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_SATURATION_U,128,0,0,0));
			_V(m_pCapApi->Cap5SetVideoAdjust(bd, ch, CAP5_VAC_SATURATION_V,128,0,0,0));
		}
	}
		
	return TRUE;
}


//static
UINT CAP5Streamer::MediaThreadStub( LPVOID lpThis )
{
	((CAP5Streamer *)lpThis)->MediaThread();
	return 0;
}

void CAP5Streamer::MediaThread()
{
	HANDLE pEvents[6];

	int idxVid=0xFF;
	int idxDI=0xFF;
	int idxVst=0xFF;
	int idxAud=0xFF;
	int idxCod=0xFF;
	int idx=0;
	pEvents[idx++] = m_hStopThreadEvent;
	HANDLE hEvent;

	// CapApi
	if (m_pCapApi->Cap5GetEventHandle(CMN5_DT_VIDEO,&hEvent)) {
		idxVid=idx;
		pEvents[idx++]=hEvent;
	}
	

	BOOL rs;
	while (TRUE) {
		DWORD waitObj=WaitForMultipleObjects(idx,pEvents,FALSE,INFINITE);
		if (waitObj==WAIT_OBJECT_0+0) {
			// breaking
			break;
		}

		else if (waitObj==WAIT_OBJECT_0+idxVid) {
			CAP5_DATA_INFO capData;
			do {
				ZeroMemory(&capData, sizeof(capData));
				rs = m_pCapApi->Cap5GetEventData(CMN5_DT_VIDEO,&capData);
				if (rs) {
					rs = ProcessCapData(&capData);
				}
			} while (rs && capData.uHasNextData);
		}
		else if (waitObj==WAIT_FAILED) {
			TRACE("WaitFailed!!\n");
		} 
	}

	SetEvent( m_hThreadStoppedEvent );
}

BOOL CAP5Streamer::ProcessCapData(CAP5_DATA_INFO* pCap5ImageInfo)
{
	if (pCap5ImageInfo->uErrCode != CMN5_EC_NO_ERROR) {
		_V(m_pCapApi->Cap5ReleaseData(pCap5ImageInfo));
		return FALSE;
	}
//	if (g_bShowProcMsg) {
//		TRACE("ProcessCapData() bd=%d ch=%d res=%dx%d\n",
//			pCap5ImageInfo->uBoardNum,
//			pCap5ImageInfo->uChannelNum,
//			CMN5_GETIMGWIDTH(pCap5ImageInfo->uImageSize),
//			CMN5_GETIMGHEIGHT(pCap5ImageInfo->uImageSize));
//	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Process the data.
	//////////////////////////////////////////////////////////////////////////////////////
	FILETIME timestamp;
	if(pCap5ImageInfo->uChannelNum == 0){
		FileTimeToLocalFileTime((FILETIME *)&(pCap5ImageInfo->TimeTag.QuadPart), (FILETIME *)&timestamp);
		// Do the callback
		BITMAPINFOHEADER bih;
		memset( &bih, 0, sizeof( BITMAPINFOHEADER ) );


		__int64 ts = timestamp.dwHighDateTime;
		ts <<= 32;
		ts |= timestamp.dwLowDateTime;

		int iWidth, iHeight;
		iWidth = CMN5_GETIMGWIDTH( pCap5ImageInfo->uImageSize );
		iHeight = CMN5_GETIMGHEIGHT( pCap5ImageInfo->uImageSize );

		bih.biHeight	= iHeight;
		bih.biWidth		= iWidth;
		bih.biCompression	= MAKEFOURCC( 'Y','U','Y','2' );
		m_pCallback->OnNewFrame( &bih, pCap5ImageInfo->pDataBuffer, iWidth * iHeight * 2, ts );
		//VCA5Process(pCap5ImageInfo->uChannelNum, pCap5ImageInfo->pDataBuffer, &timestamp);
	}

	_V(m_pCapApi->Cap5ReleaseData(pCap5ImageInfo));

	return TRUE;
}
