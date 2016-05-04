// StartVCA5.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlfile.h>
#include <initguid.h>
#include "VCA5CoreLib.h"
#include "Cap5BoardLibEx.h"

#include "VCAMetaLib.h"
#include "VideoSource/CompressedVideoSource.h"

#define VCA5_DLL_PATH			TEXT("VCA5Lib.dll")
#define TEST_FILE_PATH			TEXT("sample.wmv")
#define DEFAULT_LICENSE_NAME	TEXT("license.lic");


void DbgMsg(LPCSTR szFmt, ...)
{
	#define DEBUG_STRING_LENGTH		255
    char szOutBuff [DEBUG_STRING_LENGTH] ;

    va_list  args ;
    va_start (args , szFmt) ;
	

    _vsnprintf( szOutBuff , DEBUG_STRING_LENGTH-1, szFmt , args ) ;

    va_end ( args ) ;
	szOutBuff[DEBUG_STRING_LENGTH-1] = NULL;
	fprintf(stderr, szOutBuff);
}

#define	_V(x) \
	do { \
		if (x==0) { \
			printf("%s return 0\n", #x); \
			return FALSE; \
		} \
	} while (0)


static	IVCA5*		g_VCAApi;
static	VCA5_SYSTEM_INFO g_VCA5SysInfo;
static	DWORD		g_nVCAEngine;
IVCAVideoSource*	g_pVideoSource;


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


char* ShowFunction(ULONG function)
{
	static char strTemp[1024];
	memset(strTemp, 0, sizeof(strTemp));

	if(function&VCA5_FEATURE_PRESENCE) strcat(strTemp,	"PRESENCE|");
	if(function&VCA5_FEATURE_ENTER) strcat(strTemp,		"ENTER|");
	if(function&VCA5_FEATURE_EXIT) strcat(strTemp,		"EXIT|");
	if(function&VCA5_FEATURE_APPEAR) strcat(strTemp,	"APPEAR|");
	if(function&VCA5_FEATURE_DISAPPEAR) strcat(strTemp, "DISAPPEAR|");
	if(function&VCA5_FEATURE_STOPPED) strcat(strTemp,	"STOPPED|");
	if(function&VCA5_FEATURE_DWELL) strcat(strTemp,		"DWELL|");
	if(function&VCA5_FEATURE_DIRECTION) strcat(strTemp, "DIRECTION|");
	if(function&VCA5_FEATURE_SPEED) strcat(strTemp,		"SPEED|");
	if(function&VCA5_FEATURE_ABOBJ) strcat(strTemp,		"ABOBJ|");
	if(function&VCA5_FEATURE_TAILGATING) strcat(strTemp, "TAILGATING|");
	if(function&VCA5_FEATURE_LINECOUNTER) strcat(strTemp,"LINECOUNTER|");
	if(function&VCA5_FEATURE_PEOPLETRACKING) strcat(strTemp,"PEOPLETRACKING|");
	if(function&VCA5_FEATURE_COUNTING) strcat(strTemp,		"COUNTING|");
	if(function&VCA5_FEATURE_CALIBRATION) strcat(strTemp,	"CALIBRATION|");
	if(function&VCA5_FEATURE_METADATA) strcat(strTemp,		"METADATA|");

	return strTemp;
}


VCA5_ENGINE_PARAMS	g_EngineParam;
VCA5_LICENSE_INFO	g_LicenseInfo;

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
	
	g_nVCAEngine	= g_LicenseInfo.ulNumOfEngine;

	DbgMsg("License[%d] EngineCnt [%d] Function [0x%08X] Description [%s] \n", 
		g_LicenseInfo.nLicenseID, g_LicenseInfo.ulNumOfEngine, g_LicenseInfo.ulFunction, g_LicenseInfo.szLicenseDesc);
	DbgMsg("Function Detail [%s] \n", ShowFunction(g_LicenseInfo.ulFunction));

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
BOOL	SetupVCA(TCHAR* szDrvDll, TCHAR* szLicenseFile)
{
	TCHAR	*szVCADll=VCA5_DLL_PATH;
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
		_stprintf(szLicenseFilePath, _T("license_%c%c%c%c%c%c%c%c%c.lic"),
			szUSN[0],szUSN[1],szUSN[2],szUSN[3],szUSN[4],szUSN[5],szUSN[6],szUSN[7],szUSN[8]);
	}else{
		_tcscpy(szLicenseFilePath, szLicenseFile);
	}

	DWORD dwLicenseLength = 0;
	_V((ReadLicense(szLicenseFilePath, szLicense, _countof(szLicense), &dwLicenseLength)));
	szLicense[dwLicenseLength] = 0;

	_V((InitVCAApi(szVCADll, szDrvDll, szLicense, szUSN)));
#endif
	return TRUE;
}


void	EndupVCA()
{
	if(!g_VCAApi){
		return;
	}
	
	g_nVCAEngine= 0;
	g_VCAApi->Release();
	g_VCAApi = NULL;
}


BOOL	OpenVCA()
{
	if(!g_VCAApi){
		DbgMsg("Open VCA Fail VCA5 instance does not create before\n");
		return FALSE;
	}
	DWORD i;

	g_pVideoSource->Control(IVCAVideoSource::CMD_GET_IMGSIZE, (DWORD_PTR)&(g_EngineParam.ulImageSize), 0);
	g_pVideoSource->Control(IVCAVideoSource::CMD_GET_COLORFORMAT, (DWORD_PTR)&(g_EngineParam.ulColorFormat), 0);
	g_pVideoSource->Control(IVCAVideoSource::CMD_GET_VIDEOFORMAT, (DWORD_PTR)&(g_EngineParam.ulVideoFormat), 0);
	g_pVideoSource->Control(IVCAVideoSource::CMD_GET_FRAMERATE, (DWORD_PTR)&(g_EngineParam.ulFrameRate100), 0);

	for(i=0;i<g_nVCAEngine;i++) {
		
		g_EngineParam.ulLicenseCnt	= 1;
		g_EngineParam.ucLicenseId[0]= g_LicenseInfo.nLicenseID;

		_V(g_VCAApi->VCA5Open(i,&g_EngineParam));
	}
	return TRUE;
}


void	CloseVCA()
{
	if(!g_nVCAEngine){
		return;
	}
	
	for(DWORD i=0;i<g_nVCAEngine;i++) {
		g_VCAApi->VCA5Close(i);
	}
}


void	ShowVCAHeader(VCA5_PACKET_HEADER *pHeader)
{
	DbgMsg("Packet Header  VCAStatus : %X, FrameId : %d\n",pHeader->ulVCAStatus, pHeader->ulFrameId);
}

void	ShowVCAObject(VCA5_PACKET_OBJECTS *pObjects)
{
	DWORD i,j;
	VCA5_PACKET_OBJECT *pObject;
	VCA5_POINT trailPoint;
	for( i = 0; i <pObjects->ulTotalObject ; i++ ){
		pObject = &(pObjects->Objects[i]);
		DbgMsg("OBJECT ID:%d, Flag [0x%X] POS:[%d,%d:%d,%d] ClassificationId[%d], CalibHeight[%d], CalibSpeed[%d], CalibArea[%d] \n", 
			pObject->ulId, pObject->ulFlags, pObject->bBox.x, pObject->bBox.y, pObject->bBox.w, pObject->bBox.h,
			pObject->iClassificationId, pObject->ulCalibHeight, pObject->ulCalibSpeed, pObject->ulCalibArea);

		DbgMsg("\tTRAIL[%d] :", pObject->trail.usNumTrailPoints);
		
		for(j = 0 ; j < pObject->trail.usNumTrailPoints ; j++){
			trailPoint = pObject->trail.trailPoints[j];
			DbgMsg("[%d:%d] ",trailPoint.x, trailPoint.y);
		}
		DbgMsg("\n");
	}
}


void	ShowEventPacket(VCA5_PACKET_EVENTS *pEvents)
{
	VCA5_PACKET_EVENT *pEvent;
	DWORD i;

	for( i = 0 ; i < pEvents->ulTotalEvents ; i++ ){
		pEvent = &(pEvents->Events[i]);
		DbgMsg("Event ID:%d, RuleId[0x%X] Status[0x%X] ZoneId[%d] ObjectId[%d] StartTime[%d:%d], EndTime[%d:%d], Box[%d,%d:%d,%d]\n", 
			pEvent->ulId, pEvent->ulRuleId, pEvent->ulStatus, pEvent->ulZoneId, pEvent->ulObjId,
			pEvent->tStartTime.ulSec, pEvent->tStartTime.ulMSec, 
			pEvent->tStopTime.ulSec, pEvent->tStopTime.ulMSec, 
			pEvent->bBox.x, pEvent->bBox.y, pEvent->bBox.w, pEvent->bBox.h);
	}
}


void	ShowCounterPacket(VCA5_PACKET_COUNTS *pCounters)
{
	DWORD i;
	for( i = 0 ; i < pCounters->ulTotalCounter ; i++ ){
		DbgMsg("Counter Id :%d, Value :%d\n", pCounters[i].Counters->ulId, pCounters[i].Counters->iVal);
	}
}


void ShowResult(CVCAMetaLib* pVCAMetaDateParser)
{
	ShowVCAHeader(pVCAMetaDateParser->GetHeader());
	ShowVCAObject(pVCAMetaDateParser->GetObjects());
	ShowEventPacket(pVCAMetaDateParser->GetEvents());
	ShowCounterPacket(pVCAMetaDateParser->GetCounts());
}


int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR *szDvrDllPath		= NULL;
	TCHAR *szLIcensePath	= DEFAULT_LICENSE_NAME;
	TCHAR *szTestFile		= TEST_FILE_PATH;

	//parse by file name for PC VCA and License
	for(int i = 0 ; i < argc; i++){
		TCHAR *szParam = _tcslwr(argv[i]);
		if(_tcsstr(szParam, _T(".lic"))){
			szLIcensePath	= argv[i];
		}else if(_tcsstr(szParam, _T(".dll"))){
			szDvrDllPath	= argv[i];
			szLIcensePath	= NULL;
		}
	}

	g_pVideoSource		= (IVCAVideoSource*)new CCompressedFileVideoSource();
	CVCAMetaLib*		pVCAMetaDateParser	= CreateVCAMetaLib(CVCAMetaLib::MSXML_PARSER);

	_V(g_pVideoSource->Open(szTestFile,  FALSE));

	_V(SetupVCA(szDvrDllPath, szLIcensePath));
	_V((OpenVCA()));
		
	DWORD	eng = 0;
	BYTE	pResult[VCA5_MAX_OUTPUT_BUF_SIZE];
	DWORD	nResult = VCA5_MAX_OUTPUT_BUF_SIZE;
	
	BYTE *pRawData;
	FILETIME timestamp; 
	DWORD bEOF;

	g_pVideoSource->Start();
	for(DWORD i = 0 ; i < 1000 ; i++){
		pRawData = NULL;
		Sleep((1000*1000)/g_EngineParam.ulFrameRate100);	//for wait data incoming

		if(g_pVideoSource->GetRawImage(&pRawData, &timestamp, &bEOF)){
			nResult = VCA5_MAX_OUTPUT_BUF_SIZE;
			if((g_VCAApi->VCA5Process(eng, pRawData, (VCA5_TIMESTAMP*)&timestamp,&nResult, pResult)) && (nResult != 0)){
				pVCAMetaDateParser->ParseMetaData(pResult, nResult);
				ShowResult(pVCAMetaDateParser);		
			}
			g_pVideoSource->ReleaseRawImage();
		}
	}
	g_pVideoSource->Stop();
	CloseVCA();

	EndupVCA();

	delete pVCAMetaDateParser;
	delete (CCompressedFileVideoSource *)g_pVideoSource;
	return 0;
}

