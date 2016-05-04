#include "stdafx.h"
#include "EventFilter.h"
#include "XMLUtils.h"
#include "VCA5CoreLib.h"

#ifdef _DEBUG
#pragma comment( lib, "comsuppwd" )
#else
#pragma comment( lib, "comsuppw" )
#endif

CEventFilter *CEventFilter::m_pInstance = NULL;

CEventFilter::CEventFilter(void)
{
	m_bLoad			= FALSE;
	memset(&m_eventFilters, 0 ,sizeof(m_eventFilters));

	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_PRESENCE]		= _T("presence");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_ENTER]			= _T("enter");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_EXIT]			= _T("exit");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_APPEAR]			= _T("appear");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_DISAPPEAR]		= _T("disappear");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_STOP]			= _T("stopped");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_DWELL]			= _T("dwell");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_DIRECTION]		= _T("direction filter");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_TAILGATING]		= _T("tailgating");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_ABOBJ]			= _T("abandoned/remove object");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_LINECOUNTER_A]	= _T("counting line A");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_LINECOUNTER_B]	= _T("counting line B");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_RMOBJ]			= _T("abandoned/remove object");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_SMOKE]			= _T("smoke");
	m_RuleIntRuleStrMap[VCA5_RULE_TYPE_FIRE]			= _T("fire");

}

CEventFilter::~CEventFilter(void)
{

}

//static
CEventFilter *CEventFilter::Instance()
{
	if( !m_pInstance )
	{
		m_pInstance = new CEventFilter();
	}

	return m_pInstance;
}


HRESULT		CEventFilter::Load(TCHAR * szFilename)
{
	IXMLDOMDocument2Ptr pXMLDoc;
	HRESULT hr;
	hr = pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if(S_OK != hr){
		AfxMessageBox(_T("Can not Create XMLDOMDocument Install MSXML6.0"));
		return hr;
	}
	
	pXMLDoc->resolveExternals = VARIANT_TRUE;
	pXMLDoc->async = VARIANT_FALSE;
	pXMLDoc->validateOnParse = VARIANT_TRUE;


	_variant_t varXml(szFilename);
	_variant_t varOut((bool)TRUE);

	varOut = pXMLDoc->load(szFilename);
	if ((bool)varOut == FALSE){
		TCHAR strTemp[100];
		_stprintf_s(strTemp, _countof(strTemp), _T("Can not Read XML File %s\n"), szFilename);
		AfxMessageBox(strTemp);
		return S_FALSE;
	}

	IXMLDOMNodeListPtr	pFilterNodeList, pCondNodeList, pZoneIdNodeList, pRulesNodeList, pRuleNodeList, pObjectNodeList;
	IXMLDOMElementPtr	pFilterNode, pCondNode, pZoneIdNode, pRulesNode, pRuleNode, pObjectNode;
	bool bRet = true;

	try{
		pFilterNodeList	= pXMLDoc->documentElement->getElementsByTagName(_T("event"));
		long filterCount = pFilterNodeList->Getlength();

		for(int i=0; i<filterCount; i++) {
			pFilterNode = pFilterNodeList->Getitem(i);
			GetAttrValueString(pFilterNode, _T("name"), m_eventFilters.eventFilter[i].szFilterName, MAX_PATH);

			pCondNodeList = pFilterNode->getElementsByTagName(_T("condition"));
			long conditionCount = pCondNodeList->Getlength();
			for(int j=0; j<conditionCount; j++) {
				pCondNode = pCondNodeList->Getitem(j);
				EVENT_FILTER_CONDITION& condition = m_eventFilters.eventFilter[i].eventFilterConds.condition[j];

				pZoneIdNodeList = pCondNode->getElementsByTagName(_T("zone_id"));
				pZoneIdNode = pZoneIdNodeList->Getitem(0);
				if(pZoneIdNode) {
					condition.nZoneId = _ttoi((TCHAR*)(_bstr_t(pZoneIdNode->Gettext())));
				} else {
					condition.nZoneId = -1;
				}

				pRulesNodeList = pCondNode->getElementsByTagName(_T("rules"));
				pRulesNode = pRulesNodeList->Getitem(0);
				if(pRulesNode == NULL) {
					CString strErr;
					strErr.Format(_T("Please check %s file.\nReason: filter[%s] doesn't have any %s Node.")
						, szFilename
						,m_eventFilters.eventFilter[i].szFilterName
						, _T("rules"));
					AfxMessageBox(strErr);
					return FALSE;
				}
				pRuleNodeList = pRulesNode->getElementsByTagName(_T("rule"));
				long ruleCount = pRuleNodeList->Getlength();
				for(int k=0; k<ruleCount; k++) {
					pRuleNode = pRuleNodeList->Getitem(k);
					if(pRuleNode) {
						_tcscpy_s(condition.rules.rule[k], _countof(condition.rules.rule[k]), (TCHAR*)(_bstr_t(pRuleNode->Gettext())));
					}
				}
				condition.rules.nCount = ruleCount;

				pObjectNodeList = pCondNode->getElementsByTagName(_T("object"));
				pObjectNode = pObjectNodeList->Getitem(0);
				if(pObjectNode) {
					_tcscpy_s(condition.szObjectName, _countof(condition.szObjectName), (TCHAR*)(_bstr_t(pObjectNode->Gettext())));
				}
			}
			m_eventFilters.eventFilter[i].eventFilterConds.nCount = conditionCount;
		}
		m_eventFilters.nCount = filterCount;

	} catch(...) {
		bRet = false;
	}

//	_tcsncpy_s(&m_szAppConfPath[0], MAX_PATH, szFilename, _tcslen(szFilename));

	if(bRet){
		m_bLoad		= TRUE;
	}

	return bRet?S_OK:S_FALSE;	
}


HRESULT		CEventFilter::Save(TCHAR * szFilename)
{
/*
	IXMLDOMDocument2Ptr pXMLDoc;
	IXMLDOMElementPtr	pParenetXML, pAPPSetXML, pLicenseSetXML, pEngineSetXML, pRECSetXML, pEventExportXML;
	HRESULT hr;
	BSTR bstr_wsn = SysAllocString(L"\n");
    BSTR bstr_wsnt= SysAllocString(L"\n\t");
	BSTR bstr_wsntt= SysAllocString(L"\n\t\t");

	hr = pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if(S_OK != hr){
		AfxMessageBox(_T("Can not Create XMLDOMDocument Install MSXML6.0"));
		return hr;
	}

	pXMLDoc->resolveExternals = VARIANT_TRUE;
	pXMLDoc->async = VARIANT_FALSE;
	pXMLDoc->validateOnParse = VARIANT_TRUE;

	pParenetXML = pXMLDoc->createElement(_T("root"));
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pParenetXML);
	pAPPSetXML	= pXMLDoc->createElement(_T("APPSetting"));
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pAPPSetXML);

	SetAttrValueString(pAPPSetXML, _T("VCA5DllPath"), m_szVCA5DllPath);
	if(m_szCap5DllPath[0] != 0)SetAttrValueString(pAPPSetXML, _T("CAP5DllPath"), m_szCap5DllPath);
	if(m_szCAP5ModelInfoPath[0] != 0)SetAttrValueString(pAPPSetXML, _T("CAP5ModelXMLPath"), m_szCAP5ModelInfoPath);

	pAPPSetXML->appendChild(pLicenseSetXML);
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pAPPSetXML);
	pAPPSetXML->appendChild(pEngineSetXML);
	pParenetXML->appendChild(pAPPSetXML);
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pAPPSetXML);
	
	pRECSetXML	= pXMLDoc->createElement(_T("RECSetting"));

	SetAttrValueNum(pRECSetXML, _T("Enable"), m_bAlarmSaveEnabled);
	SetAttrValueString(pRECSetXML, _T("Path"), m_szEventLogSavePath);
	SetAttrValueNum(pRECSetXML, _T("Period"), m_AlarmSavePeriod);

	pEventExportXML	= pXMLDoc->createElement(_T("EventExport"));

	SetAttrValueNum(pEventExportXML, _T("Enable"), m_bEventExportEnabled);
	SetAttrValueString(pEventExportXML, _T("EventFilterFile"), m_szEventFilterFile);

	AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pParenetXML);
	pParenetXML->appendChild(pRECSetXML);
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pParenetXML);
	pParenetXML->appendChild(pEventExportXML);
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pParenetXML);
	pXMLDoc->appendChild(pParenetXML);
	

	try{
	hr = pXMLDoc->save(szFilename);
	if(S_OK != hr){
		TCHAR strTemp[100];
		_stprintf_s(strTemp, _countof(strTemp), _T("Can not Read XML File %s\n"), szFilename);
		AfxMessageBox(strTemp);
			
		}
	}catch(...)
	{
		
		TCHAR strTemp[100];
		_stprintf_s(strTemp, _countof(strTemp), _T("Can not Read XML File %s\n"), szFilename);
		AfxMessageBox(strTemp);
	}

	SysFreeString(bstr_wsn);
	SysFreeString(bstr_wsnt);
	SysFreeString(bstr_wsntt);
*/
	return S_OK;
}

TCHAR*	CEventFilter::GetEventName(unsigned short usRuleType, CString strObjectName, int zoneId)
{
	for(int i=0; i<m_eventFilters.nCount; i++) {
		int nCondCnt = m_eventFilters.eventFilter[i].eventFilterConds.nCount;
		for(int j=0; j<nCondCnt; j++) {
			EVENT_FILTER_CONDITION& cond = m_eventFilters.eventFilter[i].eventFilterConds.condition[j];
			CString strObjNameEventFilter(cond.szObjectName);

			BOOL bRuleExist = FALSE;
			for(int k=0; k<cond.rules.nCount; k++) {
				if( _tcscmp(m_RuleIntRuleStrMap[usRuleType], cond.rules.rule[k]) == 0 ) {
					bRuleExist = TRUE;
					break;
				}
			}

			BOOL bCorrect = bRuleExist;
			if(cond.nZoneId != -1) {
				bCorrect = bCorrect && (cond.nZoneId == zoneId);
			}
			if(strObjNameEventFilter != _T("")) {
				bCorrect = bCorrect && (strObjNameEventFilter == strObjectName);
			}

			if(bCorrect) return m_eventFilters.eventFilter[i].szFilterName;
		}
	}
	return _T("");
}