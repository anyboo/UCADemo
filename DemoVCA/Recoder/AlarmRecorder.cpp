#include "stdafx.h"
#include "AlarmRecorder.h"
#include "XMLUtils.h"
#include "VCAConfigure.h"
#include "VCADataMgr.h"
#include "VideoSource/VCAVideoSource.h"
#include "../Common/EventFilter.h"
#include <mmsystem.h>

#ifdef _DEBUG
#pragma comment( lib, "comsuppwd" )
#else
#pragma comment( lib, "comsuppw" )
#endif

//--------------------------------------------------------------------------------
CEventRecord::CEventRecord()
{
	ZeroMemory(&AlarmInfo, sizeof(ALARM_t));
	ZeroMemory(&bm, sizeof(BITMAPINFOHEADER));
	ZeroMemory(&curTime, sizeof(FILETIME));
	ZeroMemory(&Rule, sizeof(VCA5_RULE));
	ZeroMemory(&Zone, sizeof(VCA5_APP_ZONE));
	ZeroMemory(&Event, sizeof(VCA5_PACKET_EVENT));
	ZeroMemory(&EventObject, sizeof(VCA5_PACKET_OBJECT));
}

CEventRecord::CEventRecord(const CEventRecord &Other)
{
	AlarmInfo = Other.AlarmInfo;
	bm = Other.bm;
	curTime = Other.curTime;
	memcpy(&Rule, &Other.Rule, sizeof(VCA5_RULE));
	memcpy(&Zone, &Other.Zone, sizeof(VCA5_APP_ZONE));
	memcpy(&Event, &Other.Event, sizeof(VCA5_PACKET_EVENT));
	memcpy(&EventObject, &Other.EventObject, sizeof(VCA5_PACKET_OBJECT));
}

CEventRecord &CEventRecord::operator =(const CEventRecord &Other)
{
	if (this != &Other) {
		AlarmInfo = Other.AlarmInfo;
		bm = Other.bm;
		curTime = Other.curTime;
		memcpy(&Rule, &Other.Rule, sizeof(VCA5_RULE));
		memcpy(&Zone, &Other.Zone, sizeof(VCA5_APP_ZONE));
		memcpy(&Event, &Other.Event, sizeof(VCA5_PACKET_EVENT));
		memcpy(&EventObject, &Other.EventObject, sizeof(VCA5_PACKET_OBJECT));
	}
	return *this;
}

//--------------------------------------------------------------------------------
CImageData::CImageData() : pData(NULL), imageSize(0)
{
	ZeroMemory(&TimeStamp, sizeof(FILETIME));
}

CImageData::CImageData(int imageSizeIn)
{
	pData = new BYTE[imageSizeIn+16];
	imageSize = imageSizeIn;
}

CImageData::~CImageData()
{
	if(pData) delete [] pData;
}

CImageData::CImageData(const CImageData &Other)
{
	TimeStamp = Other.TimeStamp;
	imageSize = Other.imageSize;
	pData = new BYTE[Other.imageSize+16];
	memcpy(pData, Other.pData, sizeof(BYTE)*imageSize);
}

CImageData::CImageData(BYTE* pDataIn, FILETIME TimeStampIn, int imageSizeIn)
{
	TimeStamp = TimeStampIn;
	imageSize = imageSizeIn;
	pData = new BYTE[imageSize+16];
	memcpy(pData, pDataIn, sizeof(BYTE)*imageSize);
}

CImageData &CImageData::operator =(const CImageData &Other)
{
	if (this != &Other) {
		TimeStamp = Other.TimeStamp;
		imageSize = Other.imageSize;
		if(pData) delete [] pData;
		pData = new BYTE[Other.imageSize+16];
		memcpy(pData, Other.pData, sizeof(BYTE)*imageSize);
	}
	return *this;
}

//--------------------------------------------------------------------------------
CAlarmRecorder::CAlarmRecorder(void)
{
	InitializeCriticalSection( &m_cs );
	InitializeCriticalSection( &m_csImageQueue );
	m_bSetup = FALSE;
	m_i64LastDirCreateTimeStamp = 0;
	memset(m_pVCADataMgr, 0, sizeof(m_pVCADataMgr));
	memset(&m_bih, 0, sizeof(m_bih));
	m_timePrev	= 0ULL;
	m_hThread	= NULL;
	m_hKillEvent= NULL;	

}

CAlarmRecorder::~CAlarmRecorder(void)
{
	m_bSetup = FALSE;

	if(m_hThread){
		SetEvent(m_hKillEvent);
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		CloseHandle(m_hKillEvent);
	}

	list< CImageData* >::iterator itor;
	for( itor = m_ImageQueue.begin(); itor != m_ImageQueue.end(); itor++) {
		if(*itor) delete *itor;
	}

	m_GDIViewer.Endup();
	DeleteCriticalSection( &m_csImageQueue );
	DeleteCriticalSection( &m_cs );
}

BOOL CAlarmRecorder::SetOption(BOOL bEnable, LPCTSTR pszPath, ULONG uMinutePeriod)
{
	EnterCriticalSection(&m_cs);

	BOOL bRet = TRUE;
	if (bEnable) {
		_tcsncpy_s(&m_pRootPath[0], sizeof(m_pRootPath), pszPath, _tcslen(pszPath));
		m_i64DirCreatePeriod = (__int64) uMinutePeriod * 60;
		m_i64LastDirCreateTimeStamp = 0;
	}
	m_bSetup = bEnable;
	m_pSubFolderName[0] = '\0';

	m_hKillEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == m_hKillEvent) {
		bRet = FALSE;
		goto EXIT;
	}
	DWORD threadID;
	m_hThread = CreateThread(NULL,0,ProcessThreadWrapper,this,NULL,&threadID);
	if (NULL == m_hThread) {
		bRet = FALSE;
		goto EXIT;
	}

	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == m_hEvent) {
		bRet = FALSE;
		goto EXIT;
	}

EXIT:
	if(!bRet) {
		if(m_hKillEvent) CloseHandle(m_hKillEvent);
		if(m_hEvent) CloseHandle(m_hEvent);
	}
	LeaveCriticalSection(&m_cs);
	return bRet;
}

void CAlarmRecorder::SetVCADataMgr(ULONG index, CVCADataMgr* pVCADataMgr)
{
	if( m_pVCADataMgr[index] ) m_pVCADataMgr[index]->UnregisterObserver( this );

	m_pVCADataMgr[index] = pVCADataMgr;
	pVCADataMgr->RegisterObserver(this);
}

BOOL CAlarmRecorder::ChangeVCASourceInfo(DWORD EngId, BITMAPINFOHEADER *pbm)
{
	m_bih = *pbm;

	CAPPConfigure *pApp = CAPPConfigure::Instance();
	if(pApp->IsEventExportEnabled()) {
		m_GDIViewer.Endup();
		BOOL bRet = m_GDIViewer.Setup( NULL, m_bih.biWidth, m_bih.biHeight, CGDIViewer::COLORTYPE_YUY2, CGDIViewer::COLORTYPE_RGB24 );
	}

	return TRUE;
}

BOOL CAlarmRecorder::ProcessVCAData(DWORD EngId, BYTE *pImage, BITMAPINFOHEADER *pbm, VCA5_TIMESTAMP *pTimestamp, BYTE *pMetaData, int iMataDataLen, CVCAMetaLib *pVCAMetaLib)
{
	if(FALSE == m_bSetup) return FALSE;

	USES_CONVERSION;
	BOOL bRet = FALSE;
	VCA5_PACKET_EVENT	*pEvent;
	VCA5_APP_ZONE		*pZone;
	VCA5_RULE			*pRule = NULL;
	VCA5_PACKET_OBJECT *pEventObject = NULL;
	VCA5_PACKET_EVENTS *pPacketEvents = pVCAMetaLib->GetEvents();
	VCA5_PACKET_OBJECTS *pPacketObjects = pVCAMetaLib->GetObjects();

	EnterCriticalSection(&m_cs);

	CImageData* image = new CImageData(pImage, *((FILETIME*)pTimestamp), GetImageSizeFromBitmapInfo(pbm));
	EnterCriticalSection(&m_csImageQueue);
	m_ImageQueue.push_back(image);
	LeaveCriticalSection(&m_csImageQueue);

	if(m_pVCADataMgr[EngId] && pPacketEvents->ulTotalEvents){

		for (DWORD i = 0; i < pPacketEvents->ulTotalEvents ; i++ )		{
			pEvent = &pPacketEvents->Events[i];
			pRule = m_pVCADataMgr[EngId]->GetRuleByRealId(pEvent->ulRuleId);
			if (!pRule) continue;

			for(DWORD j = 0 ; j < pPacketObjects->ulTotalObject ; j++){
				if(pEvent->ulObjId == pPacketObjects->Objects[j].ulId){
					pEventObject = &(pPacketObjects->Objects[j]);
					break;
				}
			}

			ALARM_t Notification;
			ZeroMemory(&Notification, sizeof(Notification));
			Notification.dwEngId		= EngId;
			Notification.iAlarmId		= pEvent->ulId;
			Notification.iZoneId		= pEvent->ulZoneId;
			Notification.i64StartTime	= pEvent->tStartTime.ulSec;
			Notification.i64StopTime	= pEvent->tStopTime.ulSec;
			Notification.dwRuleType		= pRule->usRuleType;
			Notification.iObjectId		= pEvent->ulObjId;

			pZone = m_pVCADataMgr[Notification.dwEngId]->GetZoneById(Notification.iZoneId);
			if (!pZone) continue;

			CEventRecord er;
			er.AlarmInfo	= Notification;
			er.bm			= *pbm;
			er.curTime		= *(FILETIME*)pTimestamp;
			if(pRule) memcpy(&er.Rule, pRule, sizeof(VCA5_RULE));
			if(pZone) memcpy(&er.Zone, pZone, sizeof(VCA5_APP_ZONE));
			if(pEvent) memcpy(&er.Event, pEvent, sizeof(VCA5_PACKET_EVENT));
			if(pEventObject) memcpy(&er.EventObject, pEventObject, sizeof(VCA5_PACKET_OBJECT));

			// Push Event Record
			m_Events.push(er);
			SetEvent(m_hEvent);
		}

		bRet = TRUE;
	}

	if(m_Events.size() == 0) {
		DeleteOldImagedata(*(FILETIME*)pTimestamp);
	}

	LONGLONG timeCur = (LONGLONG)pTimestamp->ulHighDateTime;
	timeCur <<= 32;
	timeCur |= pTimestamp->ulLowDateTime;

	LONGLONG timeDiff = (LONGLONG)(m_timePrev-timeCur);
	if(timeDiff < 0LL) timeDiff *= -1LL;
	if(timeDiff > 10000000LL) {
		EnterCriticalSection(&m_csImageQueue);
		int cnt = 0;
		list< CImageData* >::iterator itor;
		for( itor = m_ImageQueue.begin(); itor != m_ImageQueue.end(); itor++) {
			cnt ++;
		}
		TRACE(_T("-- Count [%d]\n"), cnt);
		LeaveCriticalSection(&m_csImageQueue);
		m_timePrev = timeCur;
	}
	
	LeaveCriticalSection(&m_cs);
	return bRet;
}

BOOL CAlarmRecorder::SaveSnapShot(ALARM_t Notification, BYTE *pImage, BITMAPINFOHEADER *pbm,
						 VCA5_APP_ZONE *pZone, VCA5_RULE *pRule, VCA5_PACKET_OBJECT *pObject)
{
	CTime cTime = CTime( (time_t) Notification.i64StartTime );
	struct tm	osTime;
	cTime.GetLocalTm( &osTime );

	// Create a root folder
	CreateDirectory(&m_pRootPath[0], NULL);

	// Create a sub folder
	_sntprintf_s(&m_pSubFolderName[0], _countof(m_pSubFolderName), MAX_PATH, _T("%04d%02d%02d_%02d%02d"),
		osTime.tm_year+1900, osTime.tm_mon+1, osTime.tm_mday, osTime.tm_hour, osTime.tm_min);
	_sntprintf_s(&m_pPath[0], _countof(m_pPath), MAX_PATH, _T("%s\\%s"), &m_pRootPath[0], &m_pSubFolderName[0]);
	CreateDirectory(&m_pPath[0], NULL);

	_sntprintf_s(&m_pPath[0], _countof(m_pPath), MAX_PATH, _T("%s\\%s\\%03d_%03d_%04d.JPG"),
		&m_pRootPath[0], &m_pSubFolderName[0], Notification.dwEngId, Notification.iAlarmId, Notification.dwNumSubAlarms);


	if(pZone || pRule || pObject) {
		if (!m_JpegCodec.Encode(&m_pPath[0], pImage, pbm, Notification.i64StopTime, pZone, pRule, pObject)) {
			ASSERT(TRUE);
			return FALSE;
		}
	} else {
		if (!m_JpegCodec.Encode(&m_pPath[0], pImage, pbm, 0, NULL, NULL, NULL)) {
			ASSERT(TRUE);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CAlarmRecorder::SaveXML(BOOL bErase)
{
	IXMLDOMDocument2Ptr pXMLDoc;
	IXMLDOMElementPtr	pParentXML, pEngineXML, pAlarmsXML, pAlarmXML;
	HRESULT hr;
	BOOL pEngines[128];

	if (_tcsclen(&m_pSubFolderName[0]) == 0) {
		return FALSE;
	}

	ZeroMemory(&pEngines[0], sizeof(pEngines));
	hr = pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if (FAILED(hr)) {
		AfxMessageBox(_T("Cannot create XMLDOMDocumenosTime. Install MSXML6.0"));
		return FALSE;
	}
	pXMLDoc->resolveExternals = VARIANT_TRUE;
	pXMLDoc->async = VARIANT_FALSE;
	pXMLDoc->validateOnParse = VARIANT_TRUE;

	pParentXML = pXMLDoc->createElement(_T("root"));
	pXMLDoc->appendChild(pParentXML);


	map<int, ALARM_t>::iterator i = m_Alarms.begin();
	map<int, ALARM_t>::iterator deleted;
	while (i != m_Alarms.end()) {
		ALARM_t &Notification = i->second;
		if (0 == Notification.dwNumSubAlarms) {
			i++;
			continue;
		}

		// Create an "Engine" element
		if (!pEngines[Notification.dwEngId]) {
			pEngineXML	= pXMLDoc->createElement(_T("Engine"));
			// ID 
			SetAttrValueNum(pEngineXML, _T("ID"), (long) Notification.dwEngId);
			TCHAR *szChDesc = CAPPConfigure::Instance()->GetEngineDesc(Notification.dwEngId);
			SetAttrValueString(pEngineXML, _T("Desc"), szChDesc);

			pParentXML->appendChild(pEngineXML);
			pEngines[Notification.dwEngId] = TRUE;
		}

		// Create an "Engine\Alarms" element
		pAlarmsXML = pXMLDoc->createElement(_T("Alarms"));
		SetAttrValueNum(pAlarmsXML, _T("ID"), (long) Notification.iAlarmId);
		SetAttrValueNum(pAlarmsXML, _T("Type"), (long) Notification.dwRuleType);
		SetAttrValueNum(pAlarmsXML, _T("ZoneID"), (long) Notification.iZoneId);
		pEngineXML->appendChild(pAlarmsXML);

		Notification.dwNumSubAlarms = 0;

		deleted = i;
		i++;
		m_Alarms.erase(deleted);
	}


	_sntprintf_s(&m_pPath[0], _countof(m_pPath), MAX_PATH, _T("%s\\%s\\Alarms.xml"), &m_pRootPath[0], &m_pSubFolderName[0]);
	try {
		pXMLDoc->save(&m_pPath[0]);
	}
	catch (_com_error &e) {
		TRACE("Error at IXMLDOMDocument2Ptr::save\n");
		TRACE("Code = %08lx\n", e.Error());
		TRACE("Message = %s\n", e.ErrorMessage());
		TRACE("Source = %s\n", (LPCSTR) e.Source());
		TRACE("Description = %s\n", (LPCSTR) e.Description());

		OutputDebugString( e.ErrorMessage() );
	}

	pXMLDoc = NULL;
	return TRUE;
}

void CAlarmRecorder::FireOnEvent(DWORD uiEvent, DWORD uiContext)
{
	if (uiEvent == VCA_SAVE) {
		//SaveXML(TRUE);
	}
}

DWORD	CAlarmRecorder::ProcessThreadWrapper(LPVOID pParam)
{
	return ((CAlarmRecorder *) pParam)->ProcessThread();
}

UINT	CAlarmRecorder::ProcessThread()
{
	HANDLE pEvents[2];

	int idx = 0;
	pEvents[idx++] = m_hKillEvent;
	pEvents[idx++] = m_hEvent;

	while (TRUE) {
		DWORD waitObj = WaitForMultipleObjects(idx, pEvents, FALSE, INFINITE);
		if (waitObj == WAIT_OBJECT_0+1) {

			ProcessEventRecord();

		} else if (waitObj == WAIT_OBJECT_0+0) {
			break;
		} else if (waitObj == WAIT_FAILED) {
			TRACE("WaitFailed!!\n");
		} else if (waitObj == WAIT_TIMEOUT) {
			TRACE("WaitTimeout!!\n");
		}
	}

	TRACE("End of CAlarmRecorder::ProcessThread() \n");
	return 0;
}

void CAlarmRecorder::ProcessEventRecord()
{
	CAPPConfigure *pApp = CAPPConfigure::Instance();
	LONGLONG ullPreAlarmTimeInSec = pApp->GetSizeOfPreAlarm() * ONE_SEC_IN_100_USEC;
	while(m_Events.size()) {
		CEventRecord eventRec = m_Events.front();

		ALARM_t Notification = eventRec.AlarmInfo;
		int key = MAKE_ENG_ALM_KEY(Notification.dwEngId, Notification.iAlarmId);
		map<int, ALARM_t>::iterator it = m_Alarms.find( key );
		if (it == m_Alarms.end()) {
			// New Event
			list< CImageData* >::iterator itor;
			for( itor = m_ImageQueue.begin(); itor != m_ImageQueue.end(); itor++ ) {
				LONGLONG imageTS = (LONGLONG)(*itor)->TimeStamp.dwHighDateTime;
				imageTS <<= 32;
				imageTS |= (*itor)->TimeStamp.dwLowDateTime;

				LONGLONG eventTS = (LONGLONG)eventRec.curTime.dwHighDateTime;
				eventTS <<= 32;
				eventTS |= eventRec.curTime.dwLowDateTime;

				if(pApp->IsEventExportEnabled()) {

					if(imageTS < (eventTS-ullPreAlarmTimeInSec)) {
						continue;
					} else if(imageTS < eventTS) {
						Notification.i64StopTime = 0;
						DrawBIA(Notification, (*itor)->pData, (*itor)->TimeStamp, NULL, NULL, NULL);
					} else if(imageTS == eventTS) {
						DrawBIA(Notification, (*itor)->pData, (*itor)->TimeStamp, &eventRec.EventObject, &eventRec.Event, &eventRec.Rule);
					} else {
						break;
					}

					BITMAPINFOHEADER bih = m_bih;
					bih.biCompression	= BI_RGB;
					bih.biBitCount		= 24;

					SaveSnapShot(Notification, m_GDIViewer.GetImageBuffer(), &bih, NULL, NULL, NULL);
					Notification.dwNumSubAlarms++;

				} else {
					if(imageTS < (eventTS-ullPreAlarmTimeInSec)) {
						continue;
					} else if(imageTS < eventTS) {
						SaveSnapShot(Notification, (*itor)->pData, &eventRec.bm, NULL, NULL, NULL);
						Notification.dwNumSubAlarms++;
					} else if(imageTS == eventTS) {
						SaveSnapShot(Notification, (*itor)->pData, &eventRec.bm, &eventRec.Zone, &eventRec.Rule, &eventRec.EventObject);
						Notification.dwNumSubAlarms++;
					} else {	
						break;
					}
				}
			}

		} else {
			// Existing Event
			list< CImageData* >::iterator itor;
			for( itor = m_ImageQueue.begin(); itor != m_ImageQueue.end(); itor++ ) {
				LONGLONG imageTS = (LONGLONG)(*itor)->TimeStamp.dwHighDateTime;
				imageTS <<= 32;
				imageTS |= (*itor)->TimeStamp.dwLowDateTime;

				LONGLONG eventTS = (LONGLONG)eventRec.curTime.dwHighDateTime;
				eventTS <<= 32;
				eventTS |= eventRec.curTime.dwLowDateTime;

				if(pApp->IsEventExportEnabled()) {
					if(imageTS < eventTS) {
						continue;
					} else if(imageTS == eventTS) {
						Notification.dwNumSubAlarms = it->second.dwNumSubAlarms;
						DrawBIA(Notification, (*itor)->pData, (*itor)->TimeStamp, &eventRec.EventObject, &eventRec.Event, &eventRec.Rule);
					} else {
						break;
					}

					BITMAPINFOHEADER bih = m_bih;
					bih.biCompression	= BI_RGB;
					bih.biBitCount		= 24;

					SaveSnapShot(Notification, m_GDIViewer.GetImageBuffer(), &bih, NULL, NULL, NULL);
					Notification.dwNumSubAlarms++;

				} else {
					if(imageTS < eventTS) {
						continue;
					} else if(imageTS == eventTS) {
						Notification.dwNumSubAlarms = it->second.dwNumSubAlarms;
						SaveSnapShot(Notification, (*itor)->pData, &eventRec.bm, &eventRec.Zone, &eventRec.Rule, &eventRec.EventObject);
						Notification.dwNumSubAlarms++;
					} else {	
						break;
					}
				}
			}
		}


		// Check overflow of AlarmListMap
		while( m_Alarms.size() > MAX_NUM_ITEMS ) {
			m_Alarms.erase( m_Alarms.end() );
		}

		// Insert or Update AlarmListMap
		{
			int key = MAKE_ENG_ALM_KEY(Notification.dwEngId, Notification.iAlarmId);
			map<int, ALARM_t>::iterator it = m_Alarms.find( key );
			if (it == m_Alarms.end()) {
				m_Alarms.insert( std::pair< int, ALARM_t >(key, Notification) );
			} else {
				it->second.dwNumSubAlarms++;

				it->second.i64StartTime	= Notification.i64StartTime;
				it->second.i64StopTime	= Notification.i64StopTime;
				it->second.iAlarmId		= Notification.iAlarmId;
				it->second.iZoneId		= Notification.iZoneId;
				it->second.iObjectId	= Notification.iObjectId;
			}
		}
		m_Events.pop();
		DeleteOldImagedata(eventRec.curTime);
	}
}


CString CAlarmRecorder::MakeEventName(unsigned short usRuleType, CString strObjectName, int zoneId)
{
	CEventFilter *pEventFilter = CEventFilter::Instance();
	return pEventFilter->GetEventName(usRuleType, strObjectName, zoneId);
}

void	CAlarmRecorder::DrawBIA(ALARM_t Notification, BYTE *pImage, FILETIME timeStamp, VCA5_PACKET_OBJECT *pEventObject, VCA5_PACKET_EVENT *pEvent, VCA5_RULE *pRule)
{
	USES_CONVERSION;

	if(pImage) {
		m_GDIViewer.DrawImage(pImage);
	}

	if(pEventObject) {
		VCA5_RECT	rcROIEng = CAPPConfigure::Instance()->GetAPPEngineInfo( Notification.dwEngId )->tSourceData.rcROI;
		CRect		rcVCAROI;
		rcVCAROI.left	= rcROIEng.x;
		rcVCAROI.top	= rcROIEng.y;
		rcVCAROI.right	= rcROIEng.x + rcROIEng.w;
		rcVCAROI.bottom	= rcROIEng.y + rcROIEng.h;

		if( rcVCAROI.Size() == CSize(0, 0) ) {
			rcVCAROI.SetRect(0, 0, m_bih.biWidth, m_bih.biHeight);
		}

		VCA5_RECT bBox = pEventObject->bBox;
		RECT ObjectRect;
		LONG Xmin, Ymin, Xmax, Ymax;

		PERCENTTOPIXEL( Xmin, (bBox.x - bBox.w/2), rcVCAROI.Width());
		PERCENTTOPIXEL( Ymin, (bBox.y - bBox.h/2), rcVCAROI.Height());
		PERCENTTOPIXEL( Xmax, (bBox.x + bBox.w/2), rcVCAROI.Width());
		PERCENTTOPIXEL( Ymax, (bBox.y + bBox.h/2), rcVCAROI.Height());
		Xmin = max(0, Xmin);
		Ymin = max(0, Ymin);
		Xmax = min(rcVCAROI.Width(), Xmax);
		Ymax = min(rcVCAROI.Height(), Ymax);

		Xmin += rcVCAROI.left;
		Ymin += rcVCAROI.top;
		Xmax += rcVCAROI.left;
		Ymax += rcVCAROI.top;

		ObjectRect.left		= Xmin;
		ObjectRect.top		= Ymin;
		ObjectRect.right	= Xmax;
		ObjectRect.bottom	= Ymax;

		m_GDIViewer.DrawRect( ObjectRect, RGB(0, 0, 255) );
	}

	{
		CTime cTime = CTime( timeStamp, 0 );
		struct tm	osTime;
		cTime.GetGmtTm( &osTime );
		CString strTime;
		strTime.Format(_T("Video TS: %04d-%02d-%02d %02d:%02d:%02d"),
			osTime.tm_year+1900, osTime.tm_mon+1, osTime.tm_mday, osTime.tm_hour, osTime.tm_min, osTime.tm_sec);
		DrawString(strTime, CPoint(10, 10));
	}

	if(Notification.i64StopTime > 0)
	{
		CTime cTime = CTime( (time_t) Notification.i64StopTime );
		struct tm	osTime;
		cTime.GetLocalTm( &osTime );
		CString strTime;
		strTime.Format(_T("Event TS: %04d-%02d-%02d %02d:%02d:%02d"),
			osTime.tm_year+1900, osTime.tm_mon+1, osTime.tm_mday, osTime.tm_hour, osTime.tm_min, osTime.tm_sec);
		DrawString(strTime, CPoint(10, 30));
	}

	if(pEvent) {
		VCA5_APP_CLSOBJECT *pClsObj = NULL;
		CString strClsObjName, strEventName;
		if( VCA5_EVENT_TYPE_TAMPER != pEvent->ulEventType ) {

			if(pEventObject) {
				pClsObj = m_pVCADataMgr[Notification.dwEngId]->GetClsObject(pEventObject->iClassificationId);
				if(pClsObj) strClsObjName = CA2T(pClsObj->szClsobjectName, CP_UTF8);
			}

			strEventName = MakeEventName(pRule->usRuleType, strClsObjName, Notification.iZoneId);
			if(strEventName.GetLength() > 0) {
				DrawString(strEventName, CPoint(10, 50));
			}
		}
	}
}

void CAlarmRecorder::DeleteOldImagedata(FILETIME curTime)
{
	CAPPConfigure *pApp = CAPPConfigure::Instance();
	LONGLONG ullPreAlarmBufferTimeInSec = pApp->GetSizeOfPreAlarmBuffer() * ONE_SEC_IN_100_USEC;
	list< CImageData* >::iterator itor;
	for( itor = m_ImageQueue.begin(); itor != m_ImageQueue.end(); ) {
		LONGLONG imageTS = (LONGLONG)(*itor)->TimeStamp.dwHighDateTime;
		imageTS <<= 32;
		imageTS |= (*itor)->TimeStamp.dwLowDateTime;

		LONGLONG eventTS = (LONGLONG)curTime.dwHighDateTime;
		eventTS <<= 32;
		eventTS |= curTime.dwLowDateTime;

		LONGLONG timeDiff = (LONGLONG)(eventTS - imageTS);
		if(timeDiff < 0LL) timeDiff *= -1LL;
		if(timeDiff > ullPreAlarmBufferTimeInSec) {
			if(*itor) delete *itor;
			itor++;
			m_ImageQueue.pop_front();
			continue;
		} else {
			break;
		}
	}
}

size_t CAlarmRecorder::GetImageSizeFromBitmapInfo(BITMAPINFOHEADER *pbm)
{
	float bpp = 4.0;

	if (pbm->biCompression == BI_RGB) {
		if(16 == pbm->biBitCount) bpp = 2.0;
		if(24 == pbm->biBitCount) bpp = 3.0;
	} else {
		if(mmioFOURCC('Y','U','Y','2') == pbm->biCompression) bpp = 2.0;
		else if(mmioFOURCC('U','Y','V','Y') == pbm->biCompression) bpp = 2.0;
		else if(mmioFOURCC('Y','V','1','2') == pbm->biCompression) bpp = 1.5;
	}

	return (size_t)(pbm->biWidth * pbm->biHeight * bpp);
}

void CAlarmRecorder::DrawString(CString targetString, CPoint pos)
{
	TCHAR szEventName[128] = {0};
	_stprintf_s( szEventName, _countof( szEventName ), _T("%s"),  targetString);
	const int FONT_HEIGHT = 20;
	const int THICK_OFFSET = 3;
	RECT rcText;
	rcText.left = pos.x;
	rcText.top = pos.y - THICK_OFFSET;
	rcText.right = m_bih.biWidth;
	rcText.bottom = m_bih.biHeight;
	m_GDIViewer.DrawText( targetString.GetBuffer(), rcText, RGB(255, 0, 0), FONT_HEIGHT );

	rcText.left = pos.x - THICK_OFFSET;
	rcText.top = pos.y;
	rcText.right = m_bih.biWidth;
	rcText.bottom = m_bih.biHeight;
	m_GDIViewer.DrawText( targetString.GetBuffer(), rcText, RGB(255, 0, 0), FONT_HEIGHT );

	rcText.left = pos.x;
	rcText.top = pos.y + THICK_OFFSET;
	rcText.right = m_bih.biWidth;
	rcText.bottom = m_bih.biHeight;
	m_GDIViewer.DrawText( targetString.GetBuffer(), rcText, RGB(255, 0, 0), FONT_HEIGHT );

	rcText.left = pos.x + THICK_OFFSET;
	rcText.top = pos.y;
	rcText.right = m_bih.biWidth;
	rcText.bottom = m_bih.biHeight;
	m_GDIViewer.DrawText( targetString.GetBuffer(), rcText, RGB(255, 0, 0), FONT_HEIGHT );

	rcText.left = pos.x;
	rcText.top = pos.y;
	rcText.right = m_bih.biWidth;
	rcText.bottom = m_bih.biHeight;
	m_GDIViewer.DrawText( targetString.GetBuffer(), rcText, RGB(0, 255, 255), FONT_HEIGHT );
}