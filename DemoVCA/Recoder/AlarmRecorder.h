#pragma once

#include <map>
#include <queue>
#include <list>
using namespace std;

#include "VCAEventSink.h"
#include "../VCADialog/VCAConfigureObserver.h"
#include "JpegCodec.h"
#include "VCAConfigure.h"
#include "GDIViewer.h"

#define MAX_NUM_ITEMS		1000
#define MAX_NUM_SUB_ALARM	(30*60*5)	// 5Min

#define GET_ENG_BITS(engId)					((engId) & 0xFF)
#define GET_ALM_BITS(alarmId)				((alarmId) & 0xFFFFFF)
#define MAKE_ENG_ALM_KEY(engId, alarmId)	((GET_ENG_BITS(engId) << 24) | GET_ALM_BITS(alarmId))

#define ONE_SEC_IN_100_USEC	(10000000LL)

struct ALARM_t
{
	DWORD		dwEngId;
	int			iAlarmId;
	DWORD		dwRuleType;
	int			iZoneId;
	int			iObjectId;
	__int64		i64StartTime;
	__int64		i64StopTime;
	DWORD		dwNumSubAlarms;
};

class CEventRecord {
public:
	CEventRecord();

	CEventRecord(const CEventRecord &Other);

	CEventRecord &operator =(const CEventRecord &Other);

	ALARM_t				AlarmInfo;
	BITMAPINFOHEADER	bm;
	FILETIME			curTime;
	VCA5_RULE			Rule;
	VCA5_APP_ZONE		Zone;
	VCA5_PACKET_EVENT	Event;
	VCA5_PACKET_OBJECT	EventObject;
};

class CImageData {
public:
	CImageData();
	CImageData(int imageSizeIn);
	virtual ~CImageData();
	CImageData(const CImageData &Other);
	CImageData(BYTE* pDataIn, FILETIME TimeStampIn, int imageSizeIn);
	CImageData &operator =(const CImageData &Other);

	BYTE*		pData;
	FILETIME	TimeStamp;
	int			imageSize;
};

class CVCADataMgr;

class CAlarmRecorder :	public IVCAEventSink, 
						public IVCAConfigureObserver
{
public:
	CAlarmRecorder(void);
	~CAlarmRecorder(void);

	BOOL SetOption(BOOL bEnable, LPCTSTR pszPath, ULONG uMinutePeriod);
	void SetVCADataMgr(ULONG index, CVCADataMgr* pVCADataMgr);

	// IVCAEventSink implementation
	virtual BOOL	ChangeVCASourceInfo(DWORD EngId, BITMAPINFOHEADER *pbm);
	virtual BOOL	ProcessVCAData(DWORD EngId, BYTE *pImage, BITMAPINFOHEADER *pbm, VCA5_TIMESTAMP *pTimestamp, 
		BYTE *pMetaData, int iMataDataLen, CVCAMetaLib *pVCAMetaLib);

	virtual void	FireOnEvent(DWORD uiEvent, DWORD uiContext);

	static	DWORD	WINAPI ProcessThreadWrapper(LPVOID pParam);

private:
	CRITICAL_SECTION	m_cs;
	CVCADataMgr*		m_pVCADataMgr[VCA5_MAX_NUM_ENGINE];
	BOOL				m_bSetup;
	BITMAPINFOHEADER	m_bih;
	CGDIViewer			m_GDIViewer;

	ULONGLONG			m_timePrev;

	std::map<int, ALARM_t> m_Alarms;

	std::list<CImageData*> m_ImageQueue;
	CRITICAL_SECTION	m_csImageQueue;

	std::queue<CEventRecord> m_Events;

	HANDLE	m_hThread;
	HANDLE	m_hKillEvent;
	HANDLE	m_hEvent;
	
	TCHAR	m_pRootPath[MAX_PATH];
	TCHAR	m_pSubFolderName[MAX_PATH];
	TCHAR	m_pPath[MAX_PATH];

	__int64 m_i64LastDirCreateTimeStamp;
	__int64	m_i64DirCreatePeriod;
	CJpegCodec	m_JpegCodec;
	

	BOOL SaveSnapShot(ALARM_t Notification, BYTE *pImage, BITMAPINFOHEADER *pbm, VCA5_APP_ZONE *pZone, 
		VCA5_RULE *pRule, VCA5_PACKET_OBJECT *pObject);

	BOOL SaveXML(BOOL bErase);


	UINT ProcessThread();

	void ProcessEventRecord();

	CString MakeEventName(unsigned short usRuleType, CString strObjectName, int zoneId);

	void DrawBIA(ALARM_t Notification, BYTE *pImage, FILETIME timeStamp, VCA5_PACKET_OBJECT *pEventObject, VCA5_PACKET_EVENT *pEvent, VCA5_RULE *pRule);

	void DeleteOldImagedata(FILETIME curTime);

	size_t GetImageSizeFromBitmapInfo(BITMAPINFOHEADER *pbm);

	void DrawString(CString targetString, CPoint pos);
};
