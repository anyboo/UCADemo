// StartVCA5.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlfile.h>

#define _MEMDUMPERVALIDATOR_H
#include <initguid.h>
#include "Cap5BoardLibEx.h"
#include "SimpleVCA.h"
#include "VCAMetaLib.h"
#include "GDIViewer.h"
#include "VideoSource/VCAVideoSource.h"
#include "VCAConfigure.h"

extern CGDIViewer		g_GDIViewer;
extern IVCAVideoSource	*g_pVideoSource;

#define DbgMsg TRACE

#define	_V(x) \
	do { \
		if (x==0) { \
			TRACE("%s return 0\n", #x); \
			return FALSE; \
		} \
	} while (0)


static	IVCA5*				g_VCAApi;
static	VCA5_SYSTEM_INFO	g_VCA5SysInfo;
static	DWORD				g_nVCAEngine = 0;
static	CVCAMetaLib*		g_pVCAMetaDateParser = 0;
static	VCA5_ENGINE_PARAMS	g_EngineParam;
static	VCA5_LICENSE_INFO	g_LicenseInfo;
static	int g_iWidth, g_iHeight;


DWORD	ConvertGDI2VCAColorType(DWORD VCA5ColorType)
{
	if(VCA5_COLOR_FORMAT_RGB24 == VCA5ColorType ) return CGDIViewer::COLORTYPE_RGB24;
	else if(VCA5_COLOR_FORMAT_RGB16 == VCA5ColorType ) return CGDIViewer::COLORTYPE_RGB16;
	else if(VCA5_COLOR_FORMAT_YUY2 == VCA5ColorType ) return CGDIViewer::COLORTYPE_YUY2;
	else if(VCA5_COLOR_FORMAT_UYVY == VCA5ColorType ) return CGDIViewer::COLORTYPE_UYVY;
	else if(VCA5_COLOR_FORMAT_YV12 == VCA5ColorType ) return CGDIViewer::COLORTYPE_YV12;

	return CGDIViewer::COLORTYPE_UNKNOWN;
}

static BOOL Uda5CreateInstance(HMODULE hLib,REFIID riid,void ** ppInterface)
{
	if (hLib) {

		BOOL rs;
		IUnknown* pUnknown;
		BOOL (FAR WINAPI*_CreateInstance)(IUnknown ** ppInterface);
		FARPROC test_proc=GetProcAddress(hLib,"VCA5CreateInstance");
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

BOOL InitVCAApi(TCHAR* szDllName, TCHAR* szDrvDllPath, char* szLicense, char* szUSN)
{
	HMODULE hLib=NULL;
	USES_CONVERSION;

	if (szDllName) {
		_V((hLib=LoadLibrary(szDllName)));
	}

	_V(Uda5CreateInstance(hLib,IID_IVCA5,(void**)&g_VCAApi));
		
	//Set Version
	BOOL (FAR WINAPI*_VCA5Init)(ULONG ulVersion);
	FARPROC test_proc=GetProcAddress(hLib,"VCA5Init");
	if (test_proc) {
		*(FARPROC*)&_VCA5Init=test_proc;
		(*_VCA5Init)(VCA5_VERSION);
	}

	g_LicenseInfo.szUSN			= szUSN;
	g_LicenseInfo.szLicense		= szLicense;
	g_LicenseInfo.szDrvDllPath	= T2A(szDrvDllPath);
	
	_V((g_VCAApi->VCA5Activate(1,  (VCA5_LICENSE_INFO*)&g_LicenseInfo)));
	_V((g_VCAApi->VCA5GetSystemInfo(&g_VCA5SysInfo)));

	DbgMsg("Licesne[%d] EngineCnt [%d] Function [0x%08X] Decription [%s] \n", 
		g_LicenseInfo.nLicenseID, g_LicenseInfo.ulNumOfEngine, g_LicenseInfo.ulFunction, g_LicenseInfo.szLicenseDesc);

	//Just One Engine Run
	g_nVCAEngine	= 1;

	return TRUE;
}


BOOL	ReadLicense(LPCTSTR szFIleName, LPSTR szLincens, DWORD nBufferLen, PDWORD pdwBytesRead)
{
	CAtlFile fLicense;
	ULONGLONG nFileSize = 0;
	DWORD dwBytesRead = 0;

	fLicense.Create(szFIleName, FILE_GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING );
	fLicense.GetSize( nFileSize );
	fLicense.Read( szLincens, nBufferLen, dwBytesRead );

	if ( pdwBytesRead )
		*pdwBytesRead = dwBytesRead;

	if ( nFileSize == dwBytesRead )
		return TRUE;

	DbgMsg("Can not Open file [%s]\n", szFIleName);

	return FALSE;
}


typedef BOOL (WINAPI* LPFCAP5QUERYINFO)(ULONG , ULONG uIn, void *);
BOOL	VCASetup(TCHAR* szDrvDll, TCHAR* szLicenseFile)
{
	TCHAR	*szVCADll=_T("VCA5Lib.dll");
	char	szLicense[1024];
	char	szUSN[16];
	TCHAR	szLicenseFilePath[255];
	
	HMODULE hCAP5Lib	= NULL;
	LPFCAP5QUERYINFO	pCap5QueryInfo;

	memset(szLicense, 0, _countof(szLicense));
	memset(szUSN,0,_countof(szUSN));

	//Get USN Number
	if (szDrvDll) {
		_V((hCAP5Lib=LoadLibrary(szDrvDll)));
	}

	if(hCAP5Lib){
		pCap5QueryInfo = (LPFCAP5QUERYINFO)GetProcAddress(hCAP5Lib, "Cap5QueryInfo");
		_V(pCap5QueryInfo(CAP5_QR_GET_USN, 16, szUSN));
		FreeLibrary(hCAP5Lib);
	}

	//Default License 
#ifdef VCA_PRESENCE_TEST
	_V((InitVCAApi(szVCADll, szDrvDll, NULL, szUSN)));
#else
	if(!szLicenseFile){
		_stprintf_s(szLicenseFilePath, _T("license_%c%c%c%c%c%c%c%c%c.lic"),
			szUSN[0],szUSN[1],szUSN[2],szUSN[3],szUSN[4],szUSN[5],szUSN[6],szUSN[7],szUSN[8]);
	}else{
		_tcscpy(szLicenseFilePath, szLicenseFile);
	}

	DWORD dwLicenseLength = 0;
	_V((ReadLicense(szLicenseFilePath, szLicense, _countof(szLicense), &dwLicenseLength)));
	szLicense[dwLicenseLength] = 0;

	_V((InitVCAApi(szVCADll, szDrvDll, szLicense, szUSN)));
#endif

	g_pVCAMetaDateParser = CreateVCAMetaLib(CVCAMetaLib::MSXML_PARSER);
	return TRUE;
}


void	VCAEndup()
{
	if(!g_nVCAEngine){
		return;
	}

	for(DWORD i=0;i<g_nVCAEngine;i++) {
		g_VCAApi->VCA5Close(i);
	}

	g_VCAApi->Release();

	if(g_pVCAMetaDateParser)
	{
		delete g_pVCAMetaDateParser;
		g_pVCAMetaDateParser = NULL;
	}
}
	


BOOL	VCAOpen(HWND hWnd, VCA5_ENGINE_PARAMS* pEngineParam)
{
	if(!g_VCAApi){
		DbgMsg("Open VCA Fail VCA5 instance does not create before\n");
		return FALSE;
	}
	VCAClose();
	
	// Close GDI viewer
	g_GDIViewer.Endup();
	DWORD Width, Height;

	Width	= VCA5_GETIMGWIDTH(pEngineParam->ulImageSize);
	Height	= VCA5_GETIMGHEIGHT(pEngineParam->ulImageSize);

	int Rotate = CGDIViewer::ROTATE_0;
	pEngineParam->ulImageSize = VCA5_SETIMAGEROTATE(pEngineParam->ulImageSize, Rotate);

	if(CGDIViewer::ROTATE_90 == Rotate || CGDIViewer::ROTATE_270 == Rotate){
		g_iHeight	= Width;
		g_iWidth	= Height;
	}else{
		g_iHeight	= Height;
		g_iWidth	= Width;
	}

	// Re-open GDI viewer
	int gdiColorType = ConvertGDI2VCAColorType(pEngineParam->ulColorFormat);
	g_GDIViewer.Setup( hWnd, Width, Height, gdiColorType, Rotate);

	// Re-open the VCA engines
	for( unsigned int i = 0; i < g_nVCAEngine; i++ ){
		_V( g_VCAApi->VCA5Open( i, pEngineParam) );
	}

	
	// Resize window
	SetWindowPos( hWnd, 0, 0, 0, g_iWidth, g_iHeight+40, SWP_NOZORDER | SWP_NOMOVE ); // +40 because of menu & frame
	return TRUE;
}


void	VCAClose()
{
	if(!g_nVCAEngine){
		return;
	}
	
	for(DWORD i=0;i<g_nVCAEngine;i++) {
		g_VCAApi->VCA5Close(i);
	}
}

typedef struct{
	VCA5_PACKET_OBJECT	object;
	BOOL		bAlarm;		
}OBJECT_STATUS;


#define			MAX_TARGET_NUM	32
__declspec( align(16) )
DWORD			g_ObjectNum;
OBJECT_STATUS	g_ObjectStatus[MAX_TARGET_NUM];


#ifndef PIXELTOPERCENT
#define PIXELTOPERCENT( normalizeValue, pixelValue, maxValue ) \
	normalizeValue = (unsigned short) ( (pixelValue * NORMALISATOR) / ( maxValue -1) ); \
	PERCENTASSERT( normalizeValue );
#endif

#ifndef PERCENTTOPIXEL
#define PERCENTTOPIXEL( pixelValue, normalizeValue, maxValue ) \
	pixelValue = (( maxValue -1 ) * normalizeValue) / NORMALISATOR;
#endif


#define NORMALISATOR	65535	
#define PERCENTTOPIXEL_W(normalizeValue) \
	(( g_iWidth - 1 ) * normalizeValue) / NORMALISATOR

#define PERCENTTOPIXEL_H(normalizeValue) \
	(( g_iHeight - 1 ) * normalizeValue) / NORMALISATOR


inline	void	ConvertPercent2PixelRect(int x, int y, int w, int usH, RECT& rc)
{
	rc.left		= (LONG)(PERCENTTOPIXEL_W((x - w * 0.5)));
	rc.top		= (LONG)(PERCENTTOPIXEL_H((y - usH * 0.5)));
	rc.right	= (LONG)(PERCENTTOPIXEL_W((x + w * 0.5)));
	rc.bottom	= (LONG)(PERCENTTOPIXEL_H((y + usH * 0.5)));

	rc.left		= max(0, rc.left); 
	rc.top		= max(0, rc.top);
	rc.right	= min(MAX_IMAGE_WIDTH  - 1, rc.right);
	rc.bottom	= min(MAX_IMAGE_HEIGHT - 1, rc.bottom);
}

BOOL	AnalysisResult(CVCAMetaLib*	pVCAMetaDateParser, DWORD *ObjectNum, OBJECT_STATUS *pObjectStatus)
{
	VCA5_PACKET_HEADER *pHeader = pVCAMetaDateParser->GetHeader();
	if(VCA5_PACKET_STATUS_ERROR == pHeader->ulVCAStatus) return FALSE;
	
	VCA5_PACKET_OBJECTS *pObjects = pVCAMetaDateParser->GetObjects();
	ULONG i, j;
	*ObjectNum = pObjects->ulTotalObject;
	for(i = 0; i < pObjects->ulTotalObject ; i++){
		pObjectStatus[i].object = pObjects->Objects[i];
		pObjectStatus[i].bAlarm	= FALSE;
	}

	VCA5_PACKET_EVENTS *pEvents = pVCAMetaDateParser->GetEvents();
	
	for(i = 0; i < pEvents->ulTotalEvents ; i++){
		for(j = 0; j < pObjects->ulTotalObject ; j++){
			if(pEvents->Events[i].ulObjId == pObjects->Objects[j].ulId){
					pObjectStatus[j].bAlarm = TRUE;
					break;
			}
		}
	}
	
	return TRUE;
}


void	DrawZones()
{
	VCA5_APP_ENGINE_CONFIG *pConfig = CVCAConfigure::Instance()->GetEngineConf(0);
	if(!pConfig) return;

	VCA5_APP_ZONES* pZones = &(pConfig->Zones);
	VCA5_ZONE* pZone;
	POINT	Points[VCA5_MAX_NUM_ZONE_POINTS];

	ULONG	i,j;

	for(i = 0 ; i < pZones->ulTotalZones ; i++){
		pZone = &(pZones->pZones[i]);

		for(j = 0 ; j < pZone->ulTotalPoints ; j++){
			Points[j].x = PERCENTTOPIXEL_W(pZone->pPoints[j].x);
			Points[j].y = PERCENTTOPIXEL_H(pZone->pPoints[j].y);
		}

		g_GDIViewer.DrawZone(pZone->ulTotalPoints, Points, RGB(25,255,0));
	}
}


void	DrawObject(DWORD ObjectNum, OBJECT_STATUS *pObjectStatus)
{
	ULONG	i;
	RECT	ObjectRect;
	RECT	ObjectText;
	COLORREF color;
	TCHAR	id[128];

	for(i = 0 ; i < ObjectNum ; i++){
		VCA5_PACKET_OBJECT Object = pObjectStatus[i].object;
		ConvertPercent2PixelRect(Object.bBox.x, Object.bBox.y, Object.bBox.w, Object.bBox.h, ObjectRect); 

		ObjectText.top = ObjectRect.top;
		ObjectText.left = ObjectRect.left;
		ObjectText.right = ObjectText.left + 20;
		ObjectText.bottom = ObjectText.top + 20;

		if(pObjectStatus[i].bAlarm){
			color = RGB( 255, 0, 0 );
//			g_GDIViewer.DrawRect(ObjectRect, RGB(255,0,0));
		}else{
			color = RGB(255, 255, 0 );

//			g_GDIViewer.DrawRect(ObjectRect, RGB(255,255,0));
		}

		g_GDIViewer.DrawRect( ObjectRect, color );
		_stprintf_s( id, _countof( id ), _T("%d"), Object.ulId );

		g_GDIViewer.DrawText( id, ObjectText, color );
	}
}

void	DrawBlobs(  )
{
	VCA5_PACKET_BLOBMAP *pBlobMap = g_pVCAMetaDateParser->GetBlobMap();
	g_GDIViewer.DrawPixelMap( pBlobMap->pBlobMap, pBlobMap->ulWidth, pBlobMap->ulHeight, RGB( 0x00, 0xff, 0xff ) );
}



BOOL	VCASetConf()
{
	ULONG EngId	= 0;
	ULONG	i;
	VCA5_APP_ENGINE_CONFIG *pConfig;

	CVCAConfigure *pCfg = CVCAConfigure::Instance();
	if(pCfg->LoadEngine(0, _T("VCAConf.xml"))){
		return FALSE;
	}

	pConfig	= pCfg->GetEngineConf(0);
	if(!pConfig) return FALSE;

	VCA5_APP_ZONES*		pZones = &(pConfig->Zones);
	VCA5_APP_RULES*		pRules = &(pConfig->Rules);	
	VCA5_APP_COUNTERS*	pCounters = &(pConfig->Counters);

	g_VCAApi->VCA5Control(EngId, VCA5_CMD_CLEARZONE, VCA5_ID_ALL);
	g_VCAApi->VCA5Control(EngId, VCA5_CMD_CLEARRULE, VCA5_ID_ALL);
	g_VCAApi->VCA5Control(EngId, VCA5_CMD_CLEARCOUNTER, VCA5_ID_ALL);


	for(i = 0 ; i < pZones->ulTotalZones ; i++)
		g_VCAApi->VCA5Control(EngId, VCA5_CMD_SETZONE, (ULONG)(&(pZones->pZones[i])));

	for(i = 0 ; i < pRules->ulTotalRules ; i++)
		g_VCAApi->VCA5Control(EngId, VCA5_CMD_SETRULE, (ULONG)(&(pRules->pRules[i])));

	for(i = 0 ; i < pCounters->ulTotalCounters ; i++)
		g_VCAApi->VCA5Control(EngId, VCA5_CMD_SETCOUNTER, (ULONG)(&pCounters->pCounters[i]));

	g_VCAApi->VCA5Control( EngId, VCA5_CMD_SETTRACKERPARAMS, (ULONG) &(pConfig->AdvInfo.TrackerParams));

	return TRUE;
}


BOOL	VCASetConfFile()
{

	ULONG EngId	= 0;
	VCA5_XMLCFG_PARAMS params;

	params.ulMedia		= VCA5_XMLCFG_FILE;
	params.ulCfgFlags	= VCA5_CFGFLAG_ALL;
	params.pszBufOrFilename = "VCAConf.xml";
	params.ulBufLen		= 0;

	//show configuration
	CVCAConfigure *pCfg = CVCAConfigure::Instance();
	if(pCfg->LoadEngine(0, _T("VCAConf.xml"))){
		return FALSE;
	}


	return g_VCAApi->VCA5Control( EngId, VCA5_CMD_LOADCFGXML, (ULONG) &params);
}



BOOL	VCAPlay(HWND hWnd)
{
	if(!g_pVideoSource){
		return FALSE;
	}

	BYTE *pRawData;
	FILETIME timestamp; 
	DWORD bEOF;

	BYTE	pResult[VCA5_MAX_OUTPUT_BUF_SIZE];
	DWORD	nResult = VCA5_MAX_OUTPUT_BUF_SIZE;
	

	if(g_pVideoSource->GetRawImage(&pRawData, &timestamp, &bEOF)){

		g_GDIViewer.DrawImage(pRawData);
		DrawZones();

		if(g_VCAApi->VCA5Process(0, pRawData, (VCA5_TIMESTAMP*)&timestamp, &nResult, pResult)){

			g_pVCAMetaDateParser->ParseMetaData(pResult, nResult);
			AnalysisResult(g_pVCAMetaDateParser, &g_ObjectNum, g_ObjectStatus);
			DrawObject(g_ObjectNum, g_ObjectStatus);

			// Uncomment the following to draw the "blobs"
			//DrawBlobs();
		}

		HDC hDC = GetDC(hWnd);
		g_GDIViewer.Update(hDC, 0,0);
		ReleaseDC(hWnd, hDC);

		g_pVideoSource->ReleaseRawImage();
	}

	return TRUE;
}


