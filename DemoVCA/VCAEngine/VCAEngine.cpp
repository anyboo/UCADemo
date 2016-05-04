#include "StdAfx.h"

#include "VCAEngine.h"
#include "VCAEventSink.h"
#include "../VCASystem.h"
#include "VideoSource/VCAVideoSource.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include "../Render/VCARender.h"

#define CHECK_ENGINE_READY()\
	if(m_ulStatus < VCA_ENGINE_READY){\
		TRACE("Engine is not ready for Work, Current Status [%d]\n",m_ulStatus);\
		return FALSE;\
	}\


CVCAEngine::CVCAEngine(ULONG ulEngineId, CVCASystem *pVCASystem)
	: CEngine( ulEngineId )
{

	m_pVCA5System	= pVCASystem;
	m_pVCA5API		= pVCASystem->GetVCA5Lib();
	m_pVCAMetaLib	= NULL;

	m_ulEngId = 0;
	memset(&m_EngineParams, 0, sizeof(m_EngineParams));
}


CVCAEngine::~CVCAEngine()
{
	Endup();
}


BOOL	CVCAEngine::Setup(ULONG ulEngId)
{
	if (VCA_ENGINE_WORK == m_ulStatus) {
		return FALSE;
	}

	m_ulEngId = ulEngId;

	// Don't open the engine until we know what the stream looks like

	m_pVCAMetaLib	= CreateVCAMetaLib(CVCAMetaLib::MSXML_PARSER);
	return TRUE;
}


void	CVCAEngine::Endup()
{
	// Close down
	Close();
	
	if(m_pVCAMetaLib){
		delete m_pVCAMetaLib;
		m_pVCAMetaLib = NULL;
	}
}


DWORD	CVCAEngine::Open()
{
	//Check 
	if (VCA_ENGINE_FREE != m_ulStatus){
		Close();
	}

	DWORD bOpen = TRUE;
	// Now open with the new details
	unsigned long ulImgSize;
	unsigned long ulColorFmt;
	unsigned long ulFrameRate;

	m_pVideoSource->Control( IVCAVideoSource::CMD_GET_IMGSIZE, (DWORD) &ulImgSize, 0 );
	m_pVideoSource->Control( IVCAVideoSource::CMD_GET_COLORFORMAT, (DWORD) &ulColorFmt, 0 );
	m_pVideoSource->Control( IVCAVideoSource::CMD_GET_FRAMERATE, (DWORD) &ulFrameRate, 0 );

	VCA5_APP_ENGINE_INFO* pEngineInfo = CAPPConfigure::Instance()->GetAPPEngineInfo(m_ulEngId);
	

	memset( &m_EngineParams, 0, sizeof( m_EngineParams ) );
	m_EngineParams.ulImageSize		= ulImgSize;
	m_EngineParams.ulImageSize = VCA5_SETIMAGEROTATE(m_EngineParams.ulImageSize, (pEngineInfo->tSourceData.ulRotate));
	m_EngineParams.ulImageSize = VCA5_SETIMAGEFRAMETYPE(m_EngineParams.ulImageSize, (pEngineInfo->tSourceData.ulFrameType));
	m_EngineParams.ulColorFormat	= ulColorFmt;
	m_EngineParams.ulVideoFormat	= VCA5_VIDEO_FORMAT_PAL_B;
	m_EngineParams.ulFrameRate100	= ulFrameRate ? ulFrameRate :25 * 100;

	
	m_EngineParams.imageROI			= pEngineInfo->tSourceData.rcROI;

	m_EngineParams.ulLicenseCnt		= pEngineInfo->ulLicenseCnt;
	memcpy(m_EngineParams.ucLicenseId, pEngineInfo->ucLicenseId, pEngineInfo->ulLicenseCnt);

	if( m_EngineParams.ulLicenseCnt && m_pVCA5API->VCA5Open( m_ulEngId, &m_EngineParams) )
	{
		// Sync the time
		VCA5_TIME_STRUCT vcaTs;

		_timeb ts;
		_ftime_s( &ts);

		vcaTs.ulSec = (ULONG)ts.time;
		vcaTs.ulMSec = 0;

		m_pVCA5API->VCA5Control(m_ulEngId, VCA5_CMD_SETSYNCTIME, (ULONG)&vcaTs);

		ULONG MetaFmt;
		if(m_pVCA5API->VCA5Control(m_ulEngId, VCA5_CMD_GETMETAFMT, (ULONG)&MetaFmt)){
			DWORD	DisplayOption = CAPPConfigure::Instance()->GetAppEngineDisplayFlag(m_ulEngId);
			if(DisplayOption&IVCARender::DISPLAY_BLOBS) {
				MetaFmt = MetaFmt|VCA5_METAFMT_BLOB;
				MetaFmt = MetaFmt|VCA5_METAFMT_SMOKEFIRE;
			} else {
				MetaFmt = MetaFmt&(~VCA5_METAFMT_BLOB);
				MetaFmt = MetaFmt&(~VCA5_METAFMT_SMOKEFIRE);
			}

			m_pVCA5API->VCA5Control(m_ulEngId, VCA5_CMD_SETMETAFMT,  MetaFmt|VCA5_METAFMT_TAMPER|VCA5_METAFMT_SCENECHANGE|VCA5_METAFMT_COLORSIG);
		}

		//Set Engine Function.
		m_ulFunction = 0;
		CLicenseMgr* pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();
		ULONG		ulLicenseCnt;
		UCHAR		ucLicenseId[VCA5_MAX_NUM_LICENSE];

		ulLicenseCnt = 0;
		for(ULONG i = 0 ; i < m_EngineParams.ulLicenseCnt ; i++){
			VCA_APP_LICENSE_INFO *pInfo = pLicenseMgr->GetLicenseInfo(m_EngineParams.ucLicenseId[i]);

			if(pInfo){ 
				m_ulFunction |= pInfo->VCA5LicenseInfo.ulFunction;
				pInfo->UsedCount++; 
				ulLicenseCnt++;
				ucLicenseId[i]	= m_EngineParams.ucLicenseId[i];
			}
		}

		//Update 
		pEngineInfo->ulLicenseCnt	= ulLicenseCnt;
		memcpy(pEngineInfo->ucLicenseId, ucLicenseId, ulLicenseCnt);

		m_EngineParams.ulLicenseCnt = ulLicenseCnt;
		memcpy(m_EngineParams.ucLicenseId, ucLicenseId, ulLicenseCnt);

		pEngineInfo->ulFunction		= m_ulFunction;
	}
	else
	{
		//Update 
		pEngineInfo->ulLicenseCnt	= 0;
		m_EngineParams.ulLicenseCnt = 0;
		pEngineInfo->ulFunction		= 0;

		USES_CONVERSION;
		TRACE("Can not Setup VCA5 instance\n");

		VCA5_ERROR_CODE_ITEM err;
		CString sReason = _T("Unknown reason.");
		CString sMsg;
		if( m_pVCA5API->VCA5GetLastErrorCode( &err ) )
		{
			sReason = CString( A2T( err.AuxMsg ) );
		}
		
		if(m_EngineParams.ulLicenseCnt){
			sMsg.Format( _T("Unable to open VCA Engine [%d].\n%s"), m_ulEngId, sReason);
		}else{
			sMsg.Format( _T("Unable to open VCA Engine [%d].\n License is not assigned"), m_ulEngId);
		}
		AfxMessageBox( sMsg );

		bOpen = VCA5OPEN_FAIL;
	}
	
	CEngine::Open();
	return bOpen;
}


void	CVCAEngine::Close()
{
	if(VCA_ENGINE_WORK > m_ulStatus){
		m_pVCA5API->VCA5Close( m_ulEngId );

		VCA5_APP_ENGINE_INFO* pEngineInfo = CAPPConfigure::Instance()->GetAPPEngineInfo(m_ulEngId);
		CLicenseMgr* pLicenseMgr = CAPPConfigure::Instance()->GetLicenseMgr();
		for(ULONG i = 0 ; i < m_EngineParams.ulLicenseCnt ; i++){
			VCA_APP_LICENSE_INFO *pInfo = pLicenseMgr->GetLicenseInfo(m_EngineParams.ucLicenseId[i]);
			if(pInfo){
				m_ulFunction |= pInfo->VCA5LicenseInfo.ulFunction;
				pInfo->UsedCount--; 
			}
		}
		memset(&m_EngineParams, 0, sizeof(m_EngineParams));

		CEngine::Close();
	}
}


void CVCAEngine::OnFormatChange(BITMAPINFOHEADER *pBih)
{
	//need 
	if(m_ulStatus < VCA_ENGINE_READY){
		TRACE("Engine is not ready for Work, Current Status [%d]\n",m_ulStatus);
		return ;
	}

	unsigned long ulImgSize;
	unsigned long ulColorFmt;
	unsigned long ulFrameRate;

	m_pVideoSource->Control( IVCAVideoSource::CMD_GET_IMGSIZE, (DWORD) &ulImgSize, 0 );
	m_pVideoSource->Control( IVCAVideoSource::CMD_GET_COLORFORMAT, (DWORD) &ulColorFmt, 0 );
	m_pVideoSource->Control( IVCAVideoSource::CMD_GET_FRAMERATE, (DWORD) &ulFrameRate, 0 );

	VCA5_APP_ENGINE_INFO* pEngineInfo = CAPPConfigure::Instance()->GetAPPEngineInfo(m_ulEngId);
	
	m_EngineParams.ulImageSize		= ulImgSize;
	m_EngineParams.ulImageSize		= VCA5_SETIMAGEROTATE(m_EngineParams.ulImageSize, (pEngineInfo->tSourceData.ulRotate));
	m_EngineParams.ulImageSize		= VCA5_SETIMAGEFRAMETYPE(m_EngineParams.ulImageSize, (pEngineInfo->tSourceData.ulFrameType));
	m_EngineParams.ulColorFormat	= ulColorFmt;
	m_EngineParams.ulVideoFormat	= VCA5_VIDEO_FORMAT_PAL_B;
	m_EngineParams.ulFrameRate100	= ulFrameRate ? ulFrameRate :25 * 100;

	m_EngineParams.imageROI			= pEngineInfo->tSourceData.rcROI;
	//Notice does not tocch licensecount and licesne id
	
	m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_SETENGINEPARAMS, (ULONG)&m_EngineParams);
}


BOOL	CVCAEngine::SetConfig(VCA5_APP_ZONES *pZones, VCA5_APP_RULES *pRules, VCA5_APP_COUNTERS *pCounters)
{
	CHECK_ENGINE_READY();

	if (pZones) {
		m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_CLEARZONE, VCA5_ID_ALL);
	}
	if (pRules) {
		m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_CLEARRULE, VCA5_ID_ALL);
	}
	if (pCounters) {
//		m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_SETCOUNTERRES, VCA5_ID_ALL);
		m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_CLEARCOUNTER, VCA5_ID_ALL);
	}

	// update
	if (pZones) {	
		for(ULONG i = 0 ; i < pZones->ulTotalZones ; ++i)
			CHECK_FUNC(m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_SETZONE, (ULONG)(&(pZones->pZones[i]))),
				TEXT("fail to update zones"));
	}
	if (pRules) {	
		for(ULONG i = 0 ; i < pRules->ulTotalRules ; ++i)
			CHECK_FUNC(m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_SETRULE, (ULONG)(&(pRules->pRules[i]))),
				TEXT("fail to update rules"));
	}
	if (pCounters) {
		for(ULONG i = 0 ; i < pCounters->ulTotalCounters ; ++i){
			CHECK_FUNC(m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_SETCOUNTER, (ULONG)(&pCounters->pCounters[i])),
				TEXT("fail to update counters"));

			if(pCounters->pCounters[i].uiStatus&VCA5_APP_COUNTER_STATUS_NOT_USED){
				m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_SETCOUNTERRES, pCounters->pCounters[i].usCounterId, 0);
			}
		}
	}
	
	return TRUE;
}

BOOL	CVCAEngine::SetClibInfo(VCA5_APP_CALIB_INFO *pCalibInfo, DWORD inputWidth, DWORD inputHeight)
{
	CHECK_ENGINE_READY();
	CHECK_FUNC_AND_RETURN_FALSE(m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_SETCALIBPARAMS, (ULONG)(pCalibInfo),
		inputWidth, inputHeight),
				TEXT("fail to update calibration information"));
	return TRUE;
}

BOOL	CVCAEngine::SetClassObjs(VCA5_APP_CLSOBJECTS *pClassObjs)
{
	CHECK_ENGINE_READY();
	if(pClassObjs){
		m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_CLEAROBJECT, VCA5_ID_ALL);
		for(ULONG i = 0 ; i < pClassObjs->ulTotalClsObjects ; ++i){
			if(pClassObjs->pClsObjects[i].bEnable){
				CHECK_FUNC_AND_RETURN_FALSE(m_pVCA5API->VCA5Control(m_ulEngineId, VCA5_CMD_SETOBJECT, (ULONG)(&pClassObjs->pClsObjects[i])),
					TEXT("fail to update classification object"));
			}
		}
	}
	return TRUE;
}

BOOL	CVCAEngine::SetControl(ULONG ulVCA5Cmd, ULONG ulParam0, ULONG ulParam1, ULONG ulParam2, ULONG ulParam3)
{
	CHECK_ENGINE_READY();
	CHECK_FUNC_AND_RETURN_FALSE(m_pVCA5API->VCA5Control(m_ulEngineId, ulVCA5Cmd, ulParam0, ulParam1, ulParam2, ulParam3), 
		TEXT("fail to control object"));
	return TRUE;
}


void CVCAEngine::OnNewFrame(unsigned char *pData, BITMAPINFOHEADER *pBih, VCA5_TIMESTAMP *pTimestamp )
{
	ULONG uLength = 0;
	BYTE				Result[VCA5_MAX_OUTPUT_BUF_SIZE];

	uLength			= VCA5_MAX_OUTPUT_BUF_SIZE;
	if(!m_pVCA5API->VCA5Process(m_ulEngineId, pData, pTimestamp, &uLength, Result))
	{
		TRACE(TEXT("Fail to VA5 Process \n"));
		uLength		= 0;
	}

	if(uLength) m_pVCAMetaLib->ParseMetaData(Result, uLength);
	

	std::vector< IVCAEventSink *>::iterator it;

	m_csSinkLock.Lock();
	for( it = m_EventSinks.begin(); it != m_EventSinks.end(); it++ )
	{
		(*it)->ProcessVCAData(m_ulEngineId, pData, pBih, pTimestamp, Result, uLength, m_pVCAMetaLib);
	}

	//Reset countline event to display trace (ORGINAL CODE in DrawCountingLinesCounting)
	m_pVCAMetaLib->ClearCountLineEvents();
	m_csSinkLock.Unlock();
}

