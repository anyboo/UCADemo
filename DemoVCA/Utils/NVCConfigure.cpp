// CNVCConfigure.cpp: implementation of the CNVCConfigure class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NVCConfigure.h"
#include "XMLUtils.h"

#include "./common/vca_commondef.h"
#include "./common/vca_alarmdef.h"
#include "./common/vca_zonedef.h"
#include "./common/vca_eventdef.h"
#include "./common/vca_countingdef.h"
#include "./common/vca_calibdef.h"

#include "NVCUtils.h"


//////////////////////////////////////////////////////////////////////
// Implement
//////////////////////////////////////////////////////////////////////

BOOL WriteZoneDef(IXMLDOMDocument2Ptr pXMLDoc, IXMLDOMElementPtr pParenetXML, VCA_ZONES_T *zonedef)
{
	IXMLDOMElementPtr pRootXML, pZoneXML, pPointerXML;
	VCA_ZONE_T* pZone;	

	DWORD i,j, nRealZoneCnt = 0;
	BSTR bstr = NULL;
	BSTR bstr_wsn = SysAllocString(L"\n");
    BSTR bstr_wsnt= SysAllocString(L"\n\t");
	BSTR bstr_wsntt= SysAllocString(L"\n\t\t");
    
	try{
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pParenetXML);
		pRootXML = pXMLDoc->createElement(_T("ZoneSettings"));
				
		for ( i = 0; i < MAX_NUM_ZONES; i++){
			
			pZone = &zonedef->pZones[i];
			// check whether this zone is used or not
			if ( !IsZoneUsed(pZone) ) continue;

			nRealZoneCnt++;
			AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pRootXML);
			pZoneXML = pXMLDoc->createElement(_T("Zone"));
			SetAttrValueNum(pZoneXML, _T("Id"), pZone->usZoneId);
			SetAttrValueString(pZoneXML, _T("Name"), pZone->strZoneName);
			SetAttrValueNum(pZoneXML, _T("Type"), pZone->usZoneType);
			SetAttrValueNum(pZoneXML, _T("Style"), pZone->usZoneStyle);
			SetAttrValueNum(pZoneXML, _T("Color"), pZone->uiColor);
			SetAttrValueNum(pZoneXML, _T("Display"), pZone->ucDisplay);
			SetAttrValueNum(pZoneXML, _T("Enable"), pZone->ucEnable);
			SetAttrValueNum(pZoneXML, _T("TotalPoints"), pZone->uiTotalPoints);
			
			for (j = 0; j < pZone->uiTotalPoints; j++){
				AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pZoneXML);
				pPointerXML = pXMLDoc->createElement(_T("Pointer"));	
				SetAttrValueNum(pPointerXML, _T("X"), pZone->pPoints[j].x);
				SetAttrValueNum(pPointerXML, _T("Y"), pZone->pPoints[j].y);
				pZoneXML->appendChild(pPointerXML);
			}
			
			AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pZoneXML);
			pRootXML->appendChild(pZoneXML);
		}

		SetAttrValueNum(pRootXML, _T("TotalZones"), nRealZoneCnt);
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pRootXML);
		pParenetXML->appendChild(pRootXML);
	}catch(...){
		return false;	
	}
	return true;
}


BOOL ReadZoneDef(IXMLDOMNodeListPtr pRootXMLList, VCA_ZONES_T *zonedef)
{
	IXMLDOMElementPtr pRootXML, pZoneXML, pPointerXML;
	IXMLDOMNodePtr pNode;
	int i, j, zoneid;
	bool bRet = true;
	VCA_ZONE_T* pZone;
	USES_CONVERSION;
	TCHAR tcBuf[1024];
	
	try{
		pRootXML	= pRootXMLList->item[0];
		memset(zonedef, 0, sizeof(VCA_ZONES_T));
		zonedef->uiTotalZones = GetAttrValueNum(pRootXML, _T("TotalZones"));

		i = 0;
		for_ChildNodeEnum(pRootXML, pZoneXML){
			zoneid = (unsigned short)GetAttrValueNum(pZoneXML, _T("Id"));
			pZone	= &(zonedef->pZones[zoneid]);
			pZone->usZoneId = zoneid;
			GetAttrValueString(pZoneXML, _T("Name"), tcBuf, 1024);
			memcpy( pZone->strZoneName, T2A(tcBuf), min( sizeof( pZone->strZoneName ), _tcslen( tcBuf ) ) );
			
			pZone->usZoneType	= (unsigned short)GetAttrValueNum(pZoneXML, _T("Type"));
			pZone->usZoneStyle	= (unsigned short)GetAttrValueNum(pZoneXML, _T("Style"));
			pZone->uiColor		= GetAttrValueNum(pZoneXML, _T("Color"));
			pZone->ucDisplay	= (unsigned char)GetAttrValueNum(pZoneXML, _T("Display"));
			pZone->ucEnable		= (unsigned char)GetAttrValueNum(pZoneXML, _T("Enable"));
			pZone->uiTotalPoints= GetAttrValueNum(pZoneXML, _T("TotalPoints"));
			pZone->ucUsed		= 1;
			j = 0;
			for_ChildNodeEnum(pZoneXML, pPointerXML){
				pZone->pPoints[j].x = (unsigned short)GetAttrValueNum(pPointerXML, _T("X"));
				pZone->pPoints[j].y = (unsigned short)GetAttrValueNum(pPointerXML, _T("Y"));
				j++;
			}
			i++;
		}
		
	}catch(...){
		bRet = false;
	}
    
	return bRet;
}


BOOL WriteEventDef(IXMLDOMDocument2Ptr pXMLDoc, IXMLDOMElementPtr pParenetXML, VCA_EVENTS_T *eventdef)
{
	
	IXMLDOMElementPtr pRootXML, pEventXML, pRuleXML, pTrkObjXML;
	VCA_EVENT_T* pEvent;	
	VCA_RULE_T*	pRule;

	DWORD i,j,k, nRealEventCnt = 0;
	BSTR bstr = NULL;
	BSTR bstr_wsn = SysAllocString(L"\n");
    BSTR bstr_wsnt= SysAllocString(L"\n\t");
	BSTR bstr_wsntt= SysAllocString(L"\n\t\t");
    
	try	{	
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pParenetXML);
		pRootXML = pXMLDoc->createElement(_T("EventSettings"));
		//SetAttrValueNum(pRootXML, _T("TotalEvents"), eventdef->uiTotalEvents);
		for ( i = 0; i < MAX_NUM_EVENTS; i++){
			
			pEvent = &eventdef->pEvents[i];
			// check whether this event is used/ticked
			if (!IsEventUsed(pEvent) || !IsEventTicked(pEvent) ) continue;

			nRealEventCnt++;
			AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pRootXML);
			pEventXML= pXMLDoc->createElement(_T("Event"));
			SetAttrValueNum(pEventXML, _T("Id"), pEvent->iEventId);
			SetAttrValueString(pEventXML, _T("Name"), pEvent->strEventName);
			SetAttrValueNum(pEventXML, _T("Type"), pEvent->eType);
			SetAttrValueNum(pEventXML, _T("NumRules"), pEvent->iNumRules);
			SetAttrValueNum(pEventXML, _T("Enable"), pEvent->ucEnable);
			
			for (j = 0; j < pEvent->iNumRules; j++){
				pRule = &(pEvent->pRules[j]);
				if(!IsRuleUsed(pRule)) continue;

				AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pEventXML);
				
				pRuleXML = pXMLDoc->createElement(_T("Rule"));	
				SetAttrValueNum(pRuleXML, _T("Logic"), pEvent->pLogics[j]);				
				SetAttrValueNum(pRuleXML, _T("RuleType"), pRule->eRuleType);
				SetAttrValueNum(pRuleXML, _T("iTotalTrkObjs"), pRule->iTotalTrkObjs);

				for (k = 0; k < pRule->iTotalTrkObjs; k++){
					AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pRuleXML);
					pTrkObjXML = pXMLDoc->createElement(_T("TrackingObject"));	
					SetAttrValueNum(pTrkObjXML, _T("ObjectId"), pRule->ucTrkObjs[k]);
					pRuleXML->appendChild(pTrkObjXML);
				}
				SetAttrValueNum(pRuleXML, _T("RuleDataSize"), pRule->uiRuleDataSize);
				SetAttrValueBinData(pRuleXML, _T("RuleData"), pRule->uiRuleDataSize, pRule->pRuleData);
				
				pEventXML->appendChild(pRuleXML);
			}
			
			AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pEventXML);
			pRootXML->appendChild(pEventXML);
			
		}
		SetAttrValueNum(pRootXML, _T("TotalEvents"), nRealEventCnt);
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pRootXML);
		pParenetXML->appendChild(pRootXML);
	}
	catch(...){
		return false;	
	}
	return true;
}


BOOL ReadEventDef(IXMLDOMNodeListPtr pRootXMLList, VCA_EVENTS_T *eventdef)
{
	IXMLDOMElementPtr pRootXML, pEventXML, pRuleXML, pTrkObjXML;
	VCA_EVENT_T* pEvent;
	VCA_RULE_T*	pRule;
	bool bRet = true;
	int i, j, k, ieventid;


	try	{
		pRootXML	= pRootXMLList->item[0];
		memset(eventdef, 0, sizeof(VCA_EVENTS_T));
		eventdef->uiTotalEvents = GetAttrValueNum(pRootXML, _T("TotalEvents"));

		i = 0;
		for_ChildNodeEnum(pRootXML, pEventXML){
			ieventid	= (unsigned short)GetAttrValueNum(pEventXML, _T("Id"));
			pEvent	= &(eventdef->pEvents[ieventid]);
			pEvent->ucUsed		= 1;
			pEvent->iEventId	= ieventid;
			pEvent->eType		= (VCA_EVENTTYPE_T)GetAttrValueNum(pEventXML, _T("Type"));
			pEvent->iNumRules	= (unsigned short)GetAttrValueNum(pEventXML, _T("NumRules"));
			pEvent->ucEnable	= (unsigned char)GetAttrValueNum(pEventXML, _T("Enable"));	

			j = 0;
			for_ChildNodeEnum(pEventXML, pRuleXML){
				pRule = &(pEvent->pRules[j]);
				pEvent->pLogics[j]	= (VCA_LOGIC_T)GetAttrValueNum(pRuleXML, _T("Logic"));
				pRule->ucUsed		= 1;
				pRule->eRuleType	= (VCA_RULE_E)GetAttrValueNum(pRuleXML, _T("RuleType"));
				pRule->iTotalTrkObjs = GetAttrValueNum(pRuleXML, _T("iTotalTrkObjs"));
				k = 0;
				for_ChildNodeEnum(pRuleXML, pTrkObjXML){
					pRule->ucTrkObjs[k] = (unsigned short)GetAttrValueNum(pTrkObjXML, _T("ObjectId"));
					k++;
				}
				pRule->uiRuleDataSize	= GetAttrValueNum(pRuleXML, _T("RuleDataSize"));
				GetAttrValueBinData(pRuleXML, _T("RuleData"), pRule->uiRuleDataSize, pRule->pRuleData);
				j++;
			}
			i++;
		}
	}catch(...)
	{
		bRet = false;
	}
    
	return bRet;
}


BOOL WriteCounterDef(IXMLDOMDocument2Ptr pXMLDoc, IXMLDOMElementPtr	pParenetXML, VCA_COUNTERS_T *counterdef)
{
	
	IXMLDOMElementPtr pRootXML, pCounterXML, pPointerXML, pSubCounterXML;
	VCA_COUNTER_T* pCounter;
	VCA_SUBCOUNTER_T *pSubCounter;

	DWORD i,j, nRealCounterCnt = 0;;
	BSTR bstr = NULL;
	BSTR bstr_wsn = SysAllocString(L"\n");
    BSTR bstr_wsnt= SysAllocString(L"\n\t");
	BSTR bstr_wsntt= SysAllocString(L"\n\t\t");
    
	try{
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pParenetXML);
		pRootXML = pXMLDoc->createElement(_T("CounterSettings"));
		
		
		for ( i = 0; i < MAX_NUM_COUNTERS; i++){
			
			pCounter = &counterdef->pCounters[i];
			// check whether this zone is used or not
			if (!pCounter->ucUsed)	continue;

			nRealCounterCnt++;	
			AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pRootXML);
			pCounterXML = pXMLDoc->createElement(_T("Counter"));
			SetAttrValueNum(pCounterXML, _T("Id"), pCounter->usCounterId);
			SetAttrValueString(pCounterXML, _T("Name"), pCounter->strCounterName);
			SetAttrValueNum(pCounterXML, _T("Display"), pCounter->ucDisplay);
			SetAttrValueNum(pCounterXML, _T("Color"), pCounter->uiColor);
			SetAttrValueNum(pCounterXML, _T("Enable"), pCounter->ucEnable);
			SetAttrValueNum(pCounterXML, _T("NumSubCounters"), pCounter->usNumSubCounters);
				
			for (j = 0; j < 5; j++){
				AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pCounterXML);
				pPointerXML = pXMLDoc->createElement(_T("Pointer"));	
				SetAttrValueNum(pPointerXML, _T("X"), pCounter->pPoints[j].x);
				SetAttrValueNum(pPointerXML, _T("Y"), pCounter->pPoints[j].y);
				pCounterXML->appendChild(pPointerXML);
			}
			
			for (j = 0; j < MAX_NUM_SUBCOUNTERS; j++){
				pSubCounter = &(pCounter->pSubCounters[j]);
				if ( !IsSubCounterUsed( pSubCounter ) || !IsSubCounterTicked( pSubCounter ) ) continue;
				AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pCounterXML);
				pSubCounterXML = pXMLDoc->createElement(_T("SubCounter"));

				SetAttrValueNum(pSubCounterXML, _T("Id"), pSubCounter->usSubCounterId);				
				SetAttrValueNum(pSubCounterXML, _T("Type"), pSubCounter->usSubCounterType);
				SetAttrValueNum(pSubCounterXML, _T("TrigId"), pSubCounter->usTrigId);
				SetAttrValueNum(pSubCounterXML, _T("Enable"), pSubCounter->ucEnable);
				pCounterXML->appendChild(pSubCounterXML);
			}
			
			AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pCounterXML);
			pRootXML->appendChild(pCounterXML);
		}
		SetAttrValueNum(pRootXML, _T("TotalCounters"), nRealCounterCnt);
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pRootXML);
		pParenetXML->appendChild(pRootXML);
	}catch(...){
		return false;	
	}
	return true;
}


BOOL ReadCounterDef(IXMLDOMNodeListPtr pRootXMLList, VCA_COUNTERS_T *counterdef)
{
	IXMLDOMElementPtr pRootXML, pCounterXML, pPointerXML, pSubCounterXML;
	VCA_COUNTER_T* pCounter;
	VCA_SUBCOUNTER_T *pSubCounter;
	int i, j, icounterid, isubcounterid;
	bool bRet = true;
	USES_CONVERSION;

	try{
 		pRootXML	= pRootXMLList->item[0];
		memset(counterdef, 0, sizeof(VCA_COUNTERS_T));
		counterdef->uiTotalCounters = GetAttrValueNum(pRootXML, _T("TotalCounters"));

		i = 0;
		for_ChildNodeEnum(pRootXML, pCounterXML){
			icounterid					= (unsigned short)GetAttrValueNum(pCounterXML, _T("Id"));
			pCounter					= &(counterdef->pCounters[icounterid]);
			pCounter->usCounterId		= icounterid;
			GetAttrValueString(pCounterXML, _T("Name"), A2T(pCounter->strCounterName), MAX_NUM_NAME);
			pCounter->ucDisplay			= (unsigned char)GetAttrValueNum(pCounterXML, _T("Display"));
			pCounter->uiColor			= GetAttrValueNum(pCounterXML, _T("Color"));
			pCounter->ucEnable			= (unsigned char)GetAttrValueNum(pCounterXML, _T("Enable"));
			pCounter->usNumSubCounters	= (unsigned char)GetAttrValueNum(pCounterXML, _T("NumSubCounters"));
			pCounter->ucUsed			= 1;
			j = 0;
			for_ChildNodeEnum(pCounterXML, pPointerXML){
				pCounter->pPoints[j].x	= (unsigned short)GetAttrValueNum(pPointerXML, _T("X"));
				pCounter->pPoints[j].y	= (unsigned short)GetAttrValueNum(pPointerXML, _T("Y"));
				j++;
			}
			j = 0;
			for_ChildNodeEnum(pCounterXML, pSubCounterXML){
				isubcounterid					=  (unsigned short)GetAttrValueNum(pSubCounterXML, _T("Id"));	
				pSubCounter						= &(pCounter->pSubCounters[isubcounterid]);
				pSubCounter->usSubCounterId		= isubcounterid;
				pSubCounter->usSubCounterType	= (unsigned short)GetAttrValueNum(pSubCounterXML, _T("Type"));
				pSubCounter->usTrigId			= (unsigned short)GetAttrValueNum(pSubCounterXML, _T("TrigId"));
				pSubCounter->ucEnable			= (unsigned char)GetAttrValueNum(pSubCounterXML, _T("Enable"));
				pSubCounter->ucTicked			= 1;
				pSubCounter->ucUsed				= 1;
				j++;
			}
			i++;
		}
		
	}catch(...)
	{
		bRet = false;
	}
    
	return bRet;
}


BOOL WriteCalibrationDef(IXMLDOMDocument2Ptr pXMLDoc, IXMLDOMElementPtr	pParenetXML, VCA_CALIB_ALL_INFO *calibdef)
{
	IXMLDOMElementPtr pRootXML, pParamsXML, pMarkerXML;

	DWORD i;
	BSTR bstr = NULL;
	BSTR bstr_wsn = SysAllocString(L"\n");
    BSTR bstr_wsnt= SysAllocString(L"\n\t");
	BSTR bstr_wsntt= SysAllocString(L"\n\t\t");
    
	try{
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pParenetXML);
		pRootXML = pXMLDoc->createElement(_T("CalibrationSettings"));
				
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pRootXML);
		pParamsXML = pXMLDoc->createElement(_T("Calibration"));
		SetAttrValueNum(pParamsXML, _T("CalibMode"), calibdef->calibrationStatus);
		SetAttrValueFloat(pParamsXML, _T("TiltAngle"), calibdef->info.fTilt);
		SetAttrValueFloat(pParamsXML, _T("RollAngle"), calibdef->info.fRoll);			
		SetAttrValueFloat(pParamsXML, _T("CamHeight"), calibdef->info.fHeight);			
		SetAttrValueFloat(pParamsXML, _T("FOV"), calibdef->info.fFOV);			
		SetAttrValueFloat(pParamsXML, _T("MarkerHeight"), calibdef->fMarkerHeight );			
		SetAttrValueNum(pParamsXML, _T("NumMarkers"), calibdef->numMarkers);

		for (i = 0; i < calibdef->numMarkers; i++){
			AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pParamsXML);
			pMarkerXML = pXMLDoc->createElement(_T("Marker"));	
			SetAttrValueFloat(pMarkerXML, _T("X"), calibdef->markers[i].posX);
			SetAttrValueFloat(pMarkerXML, _T("Y"), calibdef->markers[i].posY);
			SetAttrValueFloat(pMarkerXML, _T("H"), calibdef->markers[i].height);
			pParamsXML->appendChild(pMarkerXML);
		}
		
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pParamsXML);
		pRootXML->appendChild(pParamsXML);

		AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pRootXML);
		pParenetXML->appendChild(pRootXML);
	}catch(...){
		return false;	
	}
	return true;
}

BOOL ReadCalibrationDef(IXMLDOMNodeListPtr pRootXMLList, VCA_CALIB_ALL_INFO *calibdef)
{
	IXMLDOMElementPtr pRootXML, pParamsXML, pMarkerXML;
	IXMLDOMNodePtr pNode;
	int i;
	bool bRet = true;
	
	try{
		pRootXML	= pRootXMLList->item[0];
		memset( calibdef, 0, sizeof(VCA_CALIB_ALL_INFO) );
		for_ChildNodeEnum(pRootXML, pParamsXML){
			calibdef->calibrationStatus = (unsigned char)GetAttrValueNum(pParamsXML, _T("CalibMode"));
			calibdef->info.fTilt		= (double)GetAttrValueFloat(pParamsXML, _T("TiltAngle"));
			calibdef->info.fRoll		= (double)GetAttrValueFloat(pParamsXML, _T("RollAngle"));
			calibdef->info.fHeight		= (double)GetAttrValueFloat(pParamsXML, _T("CamHeight"));
			calibdef->info.fFOV			= (double)GetAttrValueFloat(pParamsXML, _T("FOV"));
			calibdef->fMarkerHeight		= (double)GetAttrValueFloat(pParamsXML, _T("MarkerHeight"));
			calibdef->numMarkers		= (unsigned char)GetAttrValueNum(pParamsXML, _T("NumMarkers"));
			i = 0;
			for_ChildNodeEnum(pParamsXML, pMarkerXML){
				calibdef->markers[i].posX	= (unsigned short)GetAttrValueNum(pMarkerXML, _T("X"));
				calibdef->markers[i].posY	= (unsigned short)GetAttrValueNum(pMarkerXML, _T("Y"));
				calibdef->markers[i].height	= (unsigned short)GetAttrValueNum(pMarkerXML, _T("H"));
				i++;
			}
		}
		
	}catch(...){
		bRet = false;
	}
    
	return bRet;
}




CNVCConfigure::CNVCConfigure()
{
	CoInitialize(NULL); 
	m_bLoad	= FALSE;
}

CNVCConfigure::~CNVCConfigure()
{

}

HRESULT	CNVCConfigure::Load(TCHAR * szFilename)
{
	IXMLDOMDocument2Ptr pXMLDoc;
	HRESULT hr;
	hr = pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument40));
	if(S_OK != hr){
		AfxMessageBox( _T("Can not Create XMLDOMDocument Install MSXML4.0"), MB_OK | MB_ICONERROR);
		return hr;
	}
	
	pXMLDoc->resolveExternals = VARIANT_TRUE;
	pXMLDoc->async = VARIANT_FALSE;
	pXMLDoc->validateOnParse = VARIANT_TRUE;

	_variant_t varXml(szFilename);
	_variant_t varOut((bool)TRUE);

	varOut = pXMLDoc->load(szFilename);
	if ((bool)varOut == FALSE){
		CString strTemp;
		strTemp.Format(_T("Can not Read XML File %s\n"), szFilename);
		MessageBox(NULL, strTemp, _T("ERROR"), MB_OK);
		return S_FALSE;
	}

	IXMLDOMNodeListPtr pRootXMLList;

	VCA_ZONES_T			zonedef;
	VCA_EVENTS_T		eventdef;
	VCA_COUNTERS_T		counterdef;
	VCA_CALIB_ALL_INFO	CalibDef;

	pRootXMLList = pXMLDoc->documentElement->getElementsByTagName(_T("ZoneSettings"));
	ReadZoneDef(pRootXMLList, &zonedef);
	
	pRootXMLList = pXMLDoc->documentElement->getElementsByTagName(_T("EventSettings"));
	ReadEventDef(pRootXMLList, &eventdef);
	
	pRootXMLList = pXMLDoc->documentElement->getElementsByTagName(_T("CounterSettings"));
	ReadCounterDef(pRootXMLList, &counterdef);
	
	pRootXMLList = pXMLDoc->documentElement->getElementsByTagName(_T("CalibrationSettings"));
	ReadCalibrationDef(pRootXMLList, &CalibDef);
	
	ULONG i;
	m_Zones.ulTotalZones = zonedef.uiTotalZones;
	for(i = 0 ; i < zonedef.uiTotalZones ; i++)
		ConvertVCAZoneToVCA5(&(zonedef.pZones[i]), &(m_Zones.pZones[i]));

	m_Rules.ulTotalRules = eventdef.uiTotalEvents;
	for(i = 0 ; i < eventdef.uiTotalEvents ; i++)
		ConvertVCAEventToVCA5(&(eventdef.pEvents[i]), &(m_Rules.pRules[i]));

	m_Counters.ulTotalCounters = counterdef.uiTotalCounters;
	for(i = 0 ; i < counterdef.uiTotalCounters ; i++)
		ConvertVCACounterToVCA5(&(counterdef.pCounters[i]), &(m_Counters.pCounters[i]));

	ConvertVCACalibInfoToVCA5(&CalibDef, &m_CalibInfo);
	
	return S_OK;
}


HRESULT	CNVCConfigure::Save(TCHAR * szFilename)
{
	IXMLDOMDocument2Ptr pXMLDoc;
	IXMLDOMElementPtr	pParenetXML;
	HRESULT hr;
	BSTR bstr_wsn = SysAllocString(L"\n");	

	hr = pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument40));
	if(S_OK != hr){
		AfxMessageBox(_T("Can not Create XMLDOMDocument Install MSXML4.0"), MB_OK | MB_ICONERROR);
		return hr;
	}
	
	pXMLDoc->resolveExternals = VARIANT_TRUE;
	pXMLDoc->async = VARIANT_FALSE;
	pXMLDoc->validateOnParse = VARIANT_TRUE;

	pParenetXML = pXMLDoc->createElement(_T("Root"));


	VCA_ZONES_T			zonedef;
	VCA_EVENTS_T		eventdef;
	VCA_COUNTERS_T		counterdef;
	VCA_CALIB_ALL_INFO	CalibDef;

	ULONG i;
	zonedef.uiTotalZones = m_Zones.ulTotalZones;
	for(i = 0 ; i < m_Zones.ulTotalZones ; i++)
		ConvertVCA5ZoneToVCA(&(m_Zones.pZones[i]), &(zonedef.pZones[i]));

	eventdef.uiTotalEvents = m_Rules.ulTotalRules;
	for(i = 0 ; i < m_Rules.ulTotalRules ; i++)
		ConvertVCA5RuleToVCAEvent(&(m_Rules.pRules[i]), &(eventdef.pEvents[i]));

	counterdef.uiTotalCounters = m_Counters.ulTotalCounters;
	for(i = 0 ; i < m_Counters.ulTotalCounters ; i++)
		ConvertVCA5CounterToVCA(&(m_Counters.pCounters[i]), &(counterdef.pCounters[i]));

	ConvertVCA5CalibInfoToVCA(&m_CalibInfo, &CalibDef);
	

	WriteZoneDef(pXMLDoc, pParenetXML, &zonedef);
	WriteEventDef(pXMLDoc, pParenetXML, &eventdef);
	WriteCounterDef(pXMLDoc, pParenetXML, &counterdef);
	WriteCalibrationDef(pXMLDoc, pParenetXML, &CalibDef);

	AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pParenetXML);
	pXMLDoc->appendChild(pParenetXML);
	
	hr = pXMLDoc->save(szFilename);
	if(S_OK != hr){
		CString strTemp;
		strTemp.Format( _T("Can not Read XML File %s\n"), szFilename);
		MessageBox(NULL, strTemp, _T("ERROR"), MB_OK);
		return hr;
	}

	SysFreeString(bstr_wsn);
	return S_OK;

}
