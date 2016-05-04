#pragma once
#include "VCA5CoreLib.h"
#include "CAP5BoardLibEx.h"
#include "..\LicenseMgr.h"

typedef struct
{
	DWORD SourceType;

	struct 
	{
		DWORD	Bd;
		DWORD	Ch;
	};

	TCHAR	szRawPath[VCA_MAX_SOURCE_PATH];
	TCHAR	szVlcPath[VCA_MAX_SOURCE_PATH];
	TCHAR	szDShowURL[VCA_MAX_SOURCE_PATH];
	ULONG	ulDShowDeviceId;

	ULONG 	ulVideoFormat;
	ULONG	ulColorFormat;
	ULONG	ulImageSize;
	ULONG	ulFrameRate;
	ULONG	ulRotate;
	ULONG	ulFrameType;
	VCA5_RECT	rcROI;
}VCA5_APP_VIDEOSRC_INFO;


typedef struct 
{
	ULONG					ulLicenseCnt;
	UCHAR					ucLicenseId[VCA5_MAX_NUM_LICENSE];

	ULONG					ulFunction;
	VCA5_APP_VIDEOSRC_INFO	tSourceData;
	TCHAR					szConfPath[VCA_MAX_SOURCE_PATH];

}VCA5_APP_ENGINE_INFO;


class CAPPConfigure
{
private:
	CAPPConfigure(void);
	~CAPPConfigure(void);

public:
	static CAPPConfigure *Instance();
	void	DestroySelf() {delete this;}

	HRESULT		Load(TCHAR * szFilename);
	HRESULT		Save(TCHAR * szFilename=NULL);

	TCHAR	*GetVCA5DllPath(){return m_szVCA5DllPath;}
	TCHAR	*GetCAP5DllPath(){return m_szCap5DllPath;}
	TCHAR	*GetCAP5ModelInfoPath(){return m_szCAP5ModelInfoPath;}

	CLicenseMgr* GetLicenseMgr(){return &m_LicenseMgr;}
	
	DWORD	GetEngineCnt(){return m_EngineCnt;}
	VCA5_APP_ENGINE_INFO*	GetAPPEngineInfo(DWORD index){
		return (m_EngineCnt > index)?&m_AppEngineInfo[index]:NULL;}

	void	SetCapBoardsInfo(CAP5_BOARD_INFO* cbInfo, BYTE* pUSNs ,DWORD nCapBoards) {
		m_pCapBoardInfo = cbInfo;
		memcpy(m_USN, pUSNs, nCapBoards*MAX_USN_SIZE);
		m_nCapBoards = nCapBoards;
	}

	DWORD	GetBoardCnt() {return m_nCapBoards;}
	BYTE*	GetUSN(DWORD bd){return (m_nCapBoards>bd) ? m_USN[0]: NULL; }
	
	DWORD	GetMaxChannel(DWORD Bd) { return (m_nCapBoards > Bd)?m_pCapBoardInfo[Bd].uMaxChannel:NULL;}
	const CMN5_RESOLUTION_INFO*	GetResolutionInfo(DWORD Bd) 
		{ return (m_nCapBoards > Bd)?m_pCapBoardInfo[Bd].pResInfo:NULL;}

	TCHAR	*GetAlarmSavePath() {return &m_szEventLogSavePath[0];}
	TCHAR	*GetEngineDesc(DWORD EngId);
	DWORD	GetAlarmSavePeriod() {return m_AlarmSavePeriod;}
	DWORD	GetSizeOfPreAlarm() {return m_SizeOfPreAlarm;}
	DWORD	GetSizeOfPreAlarmBuffer() {return m_SizeOfPreAlarmBuffer;}
	DWORD	IsAlarmSaveEnabled() {return m_bAlarmSaveEnabled;}

	DWORD	IsEventExportEnabled() {return m_bEventExportEnabled;}
	TCHAR	*GetEventFilterFile() {return m_szEventFilterFile;}

	void	SetAlarmSavePath(LPCTSTR pPath) {
		_tcscpy_s(&m_szEventLogSavePath[0], MAX_PATH, pPath);
	}
	void	SetAlarmSaveEnable(DWORD bEnable) {m_bAlarmSaveEnabled = bEnable;}
	void	SetAlarmSavePeriod(DWORD Period) {m_AlarmSavePeriod = Period;}

	void	SetAppEngineDisplayFlag(int EngId, DWORD displayFlag) {m_AppEngineDisplayFlag[EngId]=displayFlag;}
	DWORD	GetAppEngineDisplayFlag(int EngId) { return m_AppEngineDisplayFlag[EngId];}
private:

	static		CAPPConfigure	*m_pInstance;


	BOOL		m_bLoad;
	TCHAR		m_szCap5DllPath[MAX_PATH];
	TCHAR		m_szCAP5ModelInfoPath[MAX_PATH];
	TCHAR		m_szVCA5DllPath[MAX_PATH];
	TCHAR		m_szAppConfPath[MAX_PATH];

	CLicenseMgr	m_LicenseMgr;

	TCHAR		m_szEventLogSavePath[MAX_PATH];
	DWORD		m_AlarmSavePeriod;		// in minute
	DWORD		m_bAlarmSaveEnabled;
	DWORD		m_SizeOfPreAlarm;		// in second
	DWORD		m_SizeOfPreAlarmBuffer;	// in second

	DWORD		m_bEventExportEnabled;
	TCHAR		m_szEventFilterFile[MAX_PATH];

	DWORD		m_EngineCnt;
	VCA5_APP_ENGINE_INFO	m_AppEngineInfo[VCA5_MAX_NUM_ENGINE];
	DWORD		m_AppEngineDisplayFlag[VCA5_MAX_NUM_ENGINE];

	DWORD		m_nCapBoards;
	CAP5_BOARD_INFO* m_pCapBoardInfo;
	BYTE		m_USN[CMN5_SYSTEM_MAX_BOARD][MAX_USN_SIZE];

};
