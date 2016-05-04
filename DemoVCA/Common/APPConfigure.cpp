#include "stdafx.h"
#include "APPConfigure.h"
#include "VideoSource/VCAVideoSource.h"
#include "XMLUtils.h"

#ifdef _DEBUG
#pragma comment( lib, "comsuppwd" )
#else
#pragma comment( lib, "comsuppw" )
#endif

CAPPConfigure *CAPPConfigure::m_pInstance = NULL;

BOOL	ReadLicenseSet(IXMLDOMNodeListPtr pRootXMLList, DWORD *pLicenseCnt, VCA_APP_LICENSE_INFO *pLicenseInfo)
{
	IXMLDOMElementPtr pRootXML, pLicenseXML;
	IXMLDOMNodePtr pNode;
	int i;
	BOOL bRet = TRUE;
	
	try{
		pRootXML	= pRootXMLList->item[0];
		*pLicenseCnt= GetAttrValueNum(pRootXML, _T("count"));
		
		i = 0;
		for_ChildNodeEnum(pRootXML, pLicenseXML){
			GetAttrValueString(pLicenseXML, _T("USN"), pLicenseInfo[i].szUSN, MAX_USN_SIZE);
			GetAttrValueString(pLicenseXML, _T("Path"), pLicenseInfo[i].szLicensePath, MAX_PATH);
			i++;
		}
	}catch(...){
		bRet = false;
	}
    
	*pLicenseCnt = i;
	return bRet;
}

BOOL	WriteLicenseSet(IXMLDOMDocument2Ptr pXMLDoc, IXMLDOMElementPtr pParenetXML, DWORD licenseCnt, VCA_APP_LICENSE_INFO *pLicenseInfo)
{
	IXMLDOMElementPtr pLicenseXML;
	ULONG i;

	BSTR bstr_wsnttt= SysAllocString(L"\n\t\t\t");

	try{		
		for ( i = 0; i < licenseCnt; i++){
			if(LICENSE_STATUS_ACTIVATE != pLicenseInfo[i].Status) continue;

			AddWhiteSpaceToNode(pXMLDoc, bstr_wsnttt, pParenetXML);
			pLicenseXML = pXMLDoc->createElement(_T("License"));
			
			SetAttrValueString(pLicenseXML, _T("USN"), pLicenseInfo[i].szUSN);
			SetAttrValueString(pLicenseXML, _T("Path"), pLicenseInfo[i].szLicensePath);
			
			pParenetXML->appendChild(pLicenseXML);
		}
	}catch(...){
		return FALSE;
	}
    
	SysFreeString(bstr_wsnttt);
	return TRUE;
}



BOOL	ReadEngineSet(IXMLDOMNodeListPtr pRootXMLList, DWORD *pEngineCnt, VCA5_APP_ENGINE_INFO* pAPPEngineInfo, DWORD* appEngineDisplayFlag)
{
	IXMLDOMElementPtr pRootXML, pEngineXML;
	IXMLDOMNodePtr pNode;
	int i;
	BOOL bRet = TRUE;
	DWORD Width, Height;
	TCHAR	strTempLicense[128];
	

	try{
		pRootXML	= pRootXMLList->item[0];
		*pEngineCnt	= GetAttrValueNum(pRootXML, _T("count"));
		
		i = 0;
		for_ChildNodeEnum(pRootXML, pEngineXML){
			pAPPEngineInfo[i].tSourceData.SourceType	= GetAttrValueNum(pEngineXML, _T("SourceType"));
			
			//Get License ID array
			if(GetAttrValueString(pEngineXML, _T("LicenseId"),strTempLicense, 128)){
				int count = 0;
				TCHAR *token = _tcstok(strTempLicense, _T(","));
				while(token){
					pAPPEngineInfo[i].ucLicenseId[count] = _tstoi(token);
					count++;
					token = _tcstok(NULL, _T(","));
				}
				pAPPEngineInfo[i].ulLicenseCnt = count;
			}

			//If not set VCAConfPath, then assign default name.
			memset(pAPPEngineInfo[i].szConfPath, 0, sizeof(pAPPEngineInfo[i].szConfPath));
			if(!GetAttrValueString( pEngineXML, _T("VCAConfPath"), pAPPEngineInfo[i].szConfPath, VCA_MAX_SOURCE_PATH )){
				_stprintf_s(pAPPEngineInfo[i].szConfPath, _T("VCAConf_%02d.xml"), i); 	
			}

			if(pAPPEngineInfo[i].szConfPath[0] == 0){
				_stprintf_s(pAPPEngineInfo[i].szConfPath, _T("VCAConf_%02d.xml"), i);
			}

			GetAttrValueString( pEngineXML, _T("SourcePath"), pAPPEngineInfo[i].tSourceData.szRawPath, VCA_MAX_SOURCE_PATH );
			GetAttrValueString( pEngineXML, _T("VLCPath"), pAPPEngineInfo[i].tSourceData.szVlcPath, VCA_MAX_SOURCE_PATH );
			pAPPEngineInfo[i].tSourceData.ulDShowDeviceId = GetAttrValueNum( pEngineXML, _T("DShowDevId") );
			pAPPEngineInfo[i].tSourceData.Bd	= GetAttrValueNum(pEngineXML, _T("Bd"));
			DWORD Ch = GetAttrValueNum(pEngineXML, _T("Ch"));
			pAPPEngineInfo[i].tSourceData.Ch	= Ch;
			pAPPEngineInfo[i].tSourceData.ulVideoFormat= GetAttrValueNum(pEngineXML, _T("VideoFormat"));
			pAPPEngineInfo[i].tSourceData.ulColorFormat= GetAttrValueHex(pEngineXML, _T("ColorFormat"));
			pAPPEngineInfo[i].tSourceData.ulRotate= GetAttrValueHex(pEngineXML, _T("Rotate"));
			pAPPEngineInfo[i].tSourceData.ulFrameType= GetAttrValueHex(pEngineXML, _T("FrameType"));

			Width	= GetAttrValueNum(pEngineXML, _T("ImageSizeW"));
			Height	= GetAttrValueNum(pEngineXML, _T("ImageSizeH"));
			pAPPEngineInfo[i].tSourceData.ulImageSize	= CMN5_MAKEIMGSIZE(Width, Height);

			pAPPEngineInfo[i].tSourceData.rcROI.x	= (USHORT)GetAttrValueNum(pEngineXML, _T("ROI_X"));
			pAPPEngineInfo[i].tSourceData.rcROI.y	= (USHORT)GetAttrValueNum(pEngineXML, _T("ROI_Y"));	
			pAPPEngineInfo[i].tSourceData.rcROI.w	= (USHORT)GetAttrValueNum(pEngineXML, _T("ROI_W"));
			pAPPEngineInfo[i].tSourceData.rcROI.h	= (USHORT)GetAttrValueNum(pEngineXML, _T("ROI_H"));

			pAPPEngineInfo[i].tSourceData.ulFrameRate	= GetAttrValueNum(pEngineXML, _T("FrameRate"));
			appEngineDisplayFlag[i] = GetAttrValueHex(pEngineXML, _T("DisplayFlag"));
			
			i++;
		}
	}catch(...){
		bRet = FALSE;
	}
    
	*pEngineCnt = i;
	return bRet;

}

BOOL	WriteEngineSet(IXMLDOMDocument2Ptr pXMLDoc, IXMLDOMElementPtr pParenetXML, DWORD engineCnt, VCA5_APP_ENGINE_INFO* pAPPEngineInfo, DWORD* appEngineDisplayFlag)
{
	IXMLDOMElementPtr pEngineXML;
	ULONG i;
	DWORD Width, Height;
	BSTR bstr_wsntt= SysAllocString(L"\n\t\t");
	BSTR bstr_wsnttt= SysAllocString(L"\n\t\t\t");
	TCHAR	strTempLicense[128];
	

	try	{	
		for ( i = 0; i < engineCnt; i++){
			AddWhiteSpaceToNode(pXMLDoc, bstr_wsnttt, pParenetXML);
			pEngineXML = pXMLDoc->createElement(_T("Engine"));

			//Set License Id array
			TCHAR	buff[32];	
			memset(strTempLicense, 0, sizeof(strTempLicense));
			for(ULONG j = 0 ; j < pAPPEngineInfo[i].ulLicenseCnt ; j++){
				_itot(pAPPEngineInfo[i].ucLicenseId[j] ,buff, 10);
				_tcscat(strTempLicense, buff);
				if(pAPPEngineInfo[i].ulLicenseCnt > j+1) _tcscat(strTempLicense, _T(","));
			}
			SetAttrValueString(pEngineXML, _T("LicenseId"), strTempLicense);


			SetAttrValueString( pEngineXML, _T("VCAConfPath"), pAPPEngineInfo[i].szConfPath );
			SetAttrValueNum(pEngineXML, _T("SourceType"), pAPPEngineInfo[i].tSourceData.SourceType);
			
//			if (IVCAVideoSource::IsFileSource(pAPPEngineInfo[i].SourceType)) {
				SetAttrValueString(pEngineXML, _T("SourcePath"), pAPPEngineInfo[i].tSourceData.szRawPath);
				SetAttrValueString( pEngineXML, _T("VLCPath"), pAPPEngineInfo[i].tSourceData.szVlcPath);
				SetAttrValueNum( pEngineXML, _T("DShowDevId"), pAPPEngineInfo[i].tSourceData.ulDShowDeviceId );
//				SetAttrValueNum(pEngineXML, _T("FrameRate"), pAPPEngineInfo[i].ulFrameRate);
//			}else if(IVCAVideoSource::CAP5SOURCE == pAPPEngineInfo[i].SourceType) {
				SetAttrValueNum(pEngineXML, _T("Bd"), pAPPEngineInfo[i].tSourceData.Bd);
				SetAttrValueNum(pEngineXML, _T("Ch"), pAPPEngineInfo[i].tSourceData.Ch);
				SetAttrValueNum(pEngineXML, _T("VideoFormat"), pAPPEngineInfo[i].tSourceData.ulVideoFormat);
				SetAttrValueHex(pEngineXML, _T("ColorFormat"), pAPPEngineInfo[i].tSourceData.ulColorFormat);
				
				SetAttrValueNum(pEngineXML, _T("Rotate"), pAPPEngineInfo[i].tSourceData.ulRotate);
				SetAttrValueHex(pEngineXML, _T("FrameType"), pAPPEngineInfo[i].tSourceData.ulFrameType);

				Width	= CMN5_GETIMGWIDTH(pAPPEngineInfo[i].tSourceData.ulImageSize);
				Height	= CMN5_GETIMGHEIGHT(pAPPEngineInfo[i].tSourceData.ulImageSize);
				SetAttrValueNum(pEngineXML, _T("ImageSizeW"), Width);
				SetAttrValueNum(pEngineXML, _T("ImageSizeH"), Height);

				SetAttrValueNum(pEngineXML, _T("ROI_X"), pAPPEngineInfo[i].tSourceData.rcROI.x);
				SetAttrValueNum(pEngineXML, _T("ROI_Y"), pAPPEngineInfo[i].tSourceData.rcROI.y);	
				SetAttrValueNum(pEngineXML, _T("ROI_W"), pAPPEngineInfo[i].tSourceData.rcROI.w);
				SetAttrValueNum(pEngineXML, _T("ROI_H"), pAPPEngineInfo[i].tSourceData.rcROI.h);
			
				SetAttrValueNum(pEngineXML, _T("FrameRate"), pAPPEngineInfo[i].tSourceData.ulFrameRate);
//			}
			SetAttrValueHex(pEngineXML, _T("DisplayFlag"), appEngineDisplayFlag[i]);
			pParenetXML->appendChild(pEngineXML);
		}
		AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pParenetXML);
	}
	catch(...){
		return FALSE;	
	}

	SysFreeString(bstr_wsntt);
	SysFreeString(bstr_wsnttt);
	return TRUE;
}



//
CAPPConfigure::CAPPConfigure(void)
{
	m_bLoad			= FALSE;
	memset(m_szCap5DllPath, 0 ,sizeof(m_szCap5DllPath));
	memset(m_szVCA5DllPath, 0 ,sizeof(m_szVCA5DllPath));
	memset(m_szCAP5ModelInfoPath, 0 ,sizeof(m_szCAP5ModelInfoPath));

	m_EngineCnt		= 0;
	memset(m_AppEngineInfo, 0 ,sizeof(VCA5_APP_ENGINE_INFO)*VCA5_MAX_NUM_ENGINE);

	m_nCapBoards	= 0;
	m_pCapBoardInfo = NULL;

	m_bAlarmSaveEnabled = 0;
	m_bEventExportEnabled = 0;
}


CAPPConfigure::~CAPPConfigure(void)
{

}

//static
CAPPConfigure *CAPPConfigure::Instance()
{
	if( !m_pInstance )
	{
		m_pInstance = new CAPPConfigure();
	}

	return m_pInstance;
}


HRESULT		CAPPConfigure::Load(TCHAR * szFilename)
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

	IXMLDOMNodeListPtr	pRootXMLList, pChildListXML;
	IXMLDOMElementPtr	pRootXML, pChildXML;
	bool bRet = true;

	DWORD					nLicenseCnt = 0;
	VCA_APP_LICENSE_INFO	LicenseInfo[VCA5_MAX_NUM_LICENSE];
	
	try{
		pRootXMLList	= pXMLDoc->documentElement->getElementsByTagName(_T("APPSetting"));
		pRootXML		= pRootXMLList->item[0];		

		GetAttrValueString(pRootXML, _T("CAP5DllPath"), m_szCap5DllPath, MAX_PATH);
		GetAttrValueString(pRootXML, _T("CAP5ModelXMLPath"), m_szCAP5ModelInfoPath, MAX_PATH);
		GetAttrValueString(pRootXML, _T("VCA5DllPath"), m_szVCA5DllPath, MAX_PATH);
	
		pChildListXML = pRootXML->getElementsByTagName(_T("LicenseSetting"));
		
		ReadLicenseSet(pChildListXML, &nLicenseCnt, LicenseInfo);
		m_LicenseMgr.SetLicenseInfo(nLicenseCnt, LicenseInfo);

		pChildListXML = pRootXML->getElementsByTagName(_T("EngineSetting"));
		ReadEngineSet(pChildListXML, &m_EngineCnt, m_AppEngineInfo, m_AppEngineDisplayFlag);


		pRootXMLList	= pXMLDoc->documentElement->getElementsByTagName(_T("RECSetting"));
		if (pRootXMLList->length) {
			pRootXML		= pRootXMLList->item[0];

			m_bAlarmSaveEnabled = GetAttrValueNum(pRootXML, _T("Enable"));
			GetAttrValueString(pRootXML, _T("Path"), m_szEventLogSavePath, MAX_PATH);
			m_AlarmSavePeriod = GetAttrValueNum(pRootXML, _T("Period"));
			m_SizeOfPreAlarm = GetAttrValueNum(pRootXML, _T("SizeOfPreAlarm"));
			if(m_SizeOfPreAlarm == 0) m_SizeOfPreAlarm = 2;
			m_SizeOfPreAlarmBuffer = GetAttrValueNum(pRootXML, _T("SizeOfPreAlarmBuffer"));
			if(m_SizeOfPreAlarmBuffer == 0) m_SizeOfPreAlarmBuffer = 4;
		} else {
			m_bAlarmSaveEnabled = FALSE;
			m_AlarmSavePeriod = 1;
			m_SizeOfPreAlarm  = 2;
			m_SizeOfPreAlarmBuffer = 4;
			_stprintf_s(&m_szEventLogSavePath[0], _countof(m_szEventLogSavePath), _T("%s"), _T("C:\\DemoVCA_Log"));
		}

		pRootXMLList	= pXMLDoc->documentElement->getElementsByTagName(_T("EventExport"));
		if (pRootXMLList->length) {
			pRootXML		= pRootXMLList->item[0];
			m_bEventExportEnabled = GetAttrValueNum(pRootXML, _T("Enable"));
			GetAttrValueString(pRootXML, _T("EventFilterFile"), m_szEventFilterFile, MAX_PATH);
		} else {
			m_bEventExportEnabled = FALSE;
			_stprintf_s(m_szEventFilterFile, _countof(m_szEventFilterFile), _T("%s"), _T("EventFilter.xml"));
		}

	} catch(...) {
		bRet = false;
	}

	_tcsncpy_s(&m_szAppConfPath[0], MAX_PATH, szFilename, _tcslen(szFilename));

	if(bRet){
		m_bLoad		= TRUE;
	}

	return bRet?S_OK:S_FALSE;	
}


HRESULT		CAPPConfigure::Save(TCHAR * szFilename)
{
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


	pLicenseSetXML = pXMLDoc->createElement(_T("LicenseSetting"));
	SetAttrValueNum(pLicenseSetXML, _T("count"), m_LicenseMgr.GetLicenseCnt());
	WriteLicenseSet(pXMLDoc, pLicenseSetXML, m_LicenseMgr.GetLicenseCnt(), m_LicenseMgr.GetLicenseInfo());
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pLicenseSetXML);


	pEngineSetXML = pXMLDoc->createElement(_T("EngineSetting"));
	SetAttrValueNum(pEngineSetXML, _T("count"), m_EngineCnt);
	WriteEngineSet(pXMLDoc, pEngineSetXML, m_EngineCnt, m_AppEngineInfo, m_AppEngineDisplayFlag);
	
	pAPPSetXML->appendChild(pLicenseSetXML);
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsntt, pAPPSetXML);
	pAPPSetXML->appendChild(pEngineSetXML);
	pParenetXML->appendChild(pAPPSetXML);
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pAPPSetXML);
	
	pRECSetXML	= pXMLDoc->createElement(_T("RECSetting"));

	SetAttrValueNum(pRECSetXML, _T("Enable"), m_bAlarmSaveEnabled);
	SetAttrValueString(pRECSetXML, _T("Path"), m_szEventLogSavePath);
	SetAttrValueNum(pRECSetXML, _T("Period"), m_AlarmSavePeriod);
	SetAttrValueNum(pRECSetXML, _T("SizeOfPreAlarm"), m_SizeOfPreAlarm);
	SetAttrValueNum(pRECSetXML, _T("SizeOfPreAlarmBuffer"), m_SizeOfPreAlarmBuffer);

	pEventExportXML	= pXMLDoc->createElement(_T("EventExport"));

	SetAttrValueNum(pEventExportXML, _T("Enable"), m_bEventExportEnabled);
	SetAttrValueString(pEventExportXML, _T("EventFilterFile"), m_szEventFilterFile);

	AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pParenetXML);
	pParenetXML->appendChild(pRECSetXML);
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsnt, pParenetXML);
	pParenetXML->appendChild(pEventExportXML);
	AddWhiteSpaceToNode(pXMLDoc, bstr_wsn, pParenetXML);
	pXMLDoc->appendChild(pParenetXML);
	
	TCHAR *pszPath;
	if (NULL == szFilename) {
		pszPath = &m_szAppConfPath[0];
	} else {
		pszPath = szFilename;
	}

	try{
	hr = pXMLDoc->save(pszPath);
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

	return S_OK;
}



TCHAR	*CAPPConfigure::GetEngineDesc(DWORD EngId)
{
	if(EngId>=m_EngineCnt) return NULL;
	static TCHAR szEngineDesc[MAX_PATH];

	VCA5_APP_ENGINE_INFO *pEngine = GetAPPEngineInfo(EngId);
	if (IVCAVideoSource::IsFileSource(pEngine->tSourceData.SourceType)) {
		_stprintf_s(szEngineDesc, _countof(szEngineDesc), _T("Engine %d\n"), EngId );	// BW - this is a frig - not sure if this code is really used??
	} else {
		_stprintf_s(szEngineDesc, _countof(szEngineDesc), _T("bd%02d_ch%02d"), 
			pEngine->tSourceData.Bd, pEngine->tSourceData.Bd);
	}

	return szEngineDesc;
}

