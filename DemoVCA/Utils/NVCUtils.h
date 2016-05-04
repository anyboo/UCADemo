#ifndef __NVC_UTILS_H__
#define __NVC_UTILS_H__

static void	ConvertVCAZoneToVCA5(VCA_ZONE_T* pZone,  VCA5_ZONE *pVCA5Zone)
{
	pVCA5Zone->usZoneId		= pZone->usZoneId;
	pVCA5Zone->usZoneStyle	= pZone->usZoneStyle;
	pVCA5Zone->usZoneType	= pZone->usZoneType;
	pVCA5Zone->ulTotalPoints	= pZone->uiTotalPoints;
	memcpy(pVCA5Zone->pPoints, pZone->pPoints, sizeof(VCA_POINT_T)*(pZone->uiTotalPoints));
}


static void	ConvertVCAEventToVCA5(VCA_EVENT_T* pEvent, VCA5_RULE *pVCA5Rule)
{
	pVCA5Rule->ulRuleId		= pEvent->iEventId;
	pVCA5Rule->usRuleType	= pEvent->pRules[0].eRuleType;
	 		
	switch(pEvent->pRules[0].eRuleType){
	case VCA_RULE_PRESENCE:
	case VCA_RULE_ENTER:
	case VCA_RULE_EXIT:
	case VCA_RULE_APPEAR:
	case VCA_RULE_DISAPPEAR:{
		VCA_RULE_BASIC_T *pRuleBasic = ((VCA_RULE_BASIC_T *)pEvent->pRules[0].pRuleData);
		pVCA5Rule->usZoneId	= pRuleBasic->usZoneId;
		break;}
	case VCA_RULE_STOP:
	case VCA_RULE_LOITERING:{
		VCA_RULE_TIMER_T *pRuleTimer = ((VCA_RULE_TIMER_T *)pEvent->pRules[0].pRuleData);
		pVCA5Rule->usZoneId = pRuleTimer->usZoneId;
		pVCA5Rule->tRuleDataEx.tTimer.ulTimeThreshold	= pRuleTimer->uiTimeThreshold;
		break;}
	
	case VCA_RULE_DIRECTIONFILTER:{
		VCA_RULE_DIRECTIONFILTERS_T* pRuleDirect = ((VCA_RULE_DIRECTIONFILTERS_T*)pEvent->pRules[0].pRuleData);
		pVCA5Rule->usZoneId = pRuleDirect->usZoneId;
		pVCA5Rule->tRuleDataEx.tDirection.startangle = pRuleDirect->pDirections->startangle;
		pVCA5Rule->tRuleDataEx.tDirection.finishangle = pRuleDirect->pDirections->finishangle;
		break;}
	}
}

static void	ConvertVCACounterToVCA5(VCA_COUNTER_T* pCounter, VCA5_COUNTER *pVCA5Counter)
{

	ULONG i;
	pVCA5Counter->usCounterId		= pCounter->usCounterId;
	pVCA5Counter->usNumSubCounters	= pCounter->usNumSubCounters;

	for(i = 0 ; i < pCounter->usNumSubCounters ; i++){
		pVCA5Counter->pSubCounters[i].usSubCounterType	= pCounter->pSubCounters[i].usSubCounterType;
		pVCA5Counter->pSubCounters[i].usTrigId			= pCounter->pSubCounters[i].usTrigId;
	}
}

static void	ConvertVCACalibInfoToVCA5(VCA_CALIB_ALL_INFO* pCalibInfo, VCA5_CALIB_INFO* pCalibVCA5)
{
	pCalibVCA5->fHeight	= pCalibInfo->info.fHeight;
	pCalibVCA5->fTilt	= pCalibInfo->info.fTilt;
	pCalibVCA5->fRoll	= pCalibInfo->info.fRoll;
	pCalibVCA5->fFOV	= pCalibInfo->info.fFOV;
}






static void	ConvertVCA5ZoneToVCA(VCA5_ZONE *pVCA5Zone, VCA_ZONE_T* pZone)
{
	memset(pZone, 0, sizeof(VCA_ZONE_T));
	pZone->usZoneId		= pVCA5Zone->usZoneId;
	pZone->usZoneStyle	= pVCA5Zone->usZoneStyle;
	pZone->usZoneType	= pVCA5Zone->usZoneType;
	pZone->uiTotalPoints= pVCA5Zone->ulTotalPoints;
	pZone->ucEnable		= 1;
	pZone->ucUsed		= 1;
	memcpy(pZone->pPoints, pVCA5Zone->pPoints, sizeof(VCA_POINT_T)*(pZone->uiTotalPoints));
}


static void	ConvertVCA5RuleToVCAEvent(VCA5_RULE *pVCA5Rule, VCA_EVENT_T* pEvent)
{
	memset(pEvent, 0, sizeof(VCA_EVENT_T));

	pEvent->iEventId			= pVCA5Rule->ulRuleId;
	pEvent->iNumRules			= 1;
	pEvent->ucUsed				= 1;
	pEvent->ucTicked			= 1;
	pEvent->ucEnable			= 1;

	pEvent->pRules[0].eRuleType = (VCA_RULE_E)pVCA5Rule->usRuleType;
	pEvent->pRules[0].ucUsed	= 1;
	pEvent->pLogics[0]			= VCA_LOGIC_AND;
		
	switch(pVCA5Rule->usRuleType){
	case VCA_RULE_PRESENCE:
	case VCA_RULE_ENTER:
	case VCA_RULE_EXIT:
	case VCA_RULE_APPEAR:
	case VCA_RULE_DISAPPEAR:{
		VCA_RULE_BASIC_T *pRuleBasic = ((VCA_RULE_BASIC_T *)pEvent->pRules[0].pRuleData);
		pRuleBasic->usZoneId = pVCA5Rule->usZoneId;
		pEvent->pRules[0].uiRuleDataSize = sizeof(VCA_RULE_BASIC_T);
		
		break;}
	case VCA_RULE_STOP:
	case VCA_RULE_LOITERING:{
		VCA_RULE_TIMER_T *pRuleTimer = ((VCA_RULE_TIMER_T *)pEvent->pRules[0].pRuleData);
		pRuleTimer->usZoneId = pVCA5Rule->usZoneId;
		pRuleTimer->uiTimeThreshold = pVCA5Rule->tRuleDataEx.tTimer.ulTimeThreshold;
		pEvent->pRules[0].uiRuleDataSize = sizeof(VCA_RULE_TIMER_T);
		break;}
	
	case VCA_RULE_DIRECTIONFILTER:{
		VCA_RULE_DIRECTIONFILTERS_T* pRuleDirect = ((VCA_RULE_DIRECTIONFILTERS_T*)pEvent->pRules[0].pRuleData);
		pRuleDirect->usZoneId = pVCA5Rule->usZoneId;
		pRuleDirect->pDirections->startangle	= pVCA5Rule->tRuleDataEx.tDirection.startangle;
		pRuleDirect->pDirections->finishangle	= pVCA5Rule->tRuleDataEx.tDirection.finishangle;
		pEvent->pRules[0].uiRuleDataSize = sizeof(VCA_RULE_DIRECTIONFILTERS_T);
		break;}
	}
}

static void	ConvertVCA5CounterToVCA(VCA5_COUNTER *pVCA5Counter, VCA_COUNTER_T* pCounter)
{

	memset(pCounter, 0, sizeof(VCA_COUNTER_T));
	ULONG i;
	pCounter->usCounterId		= pVCA5Counter->usCounterId;
	pCounter->usNumSubCounters	= pVCA5Counter->usNumSubCounters;
	pCounter->ucEnable			= 1;
	pCounter->ucUsed			= 1;

	for(i = 0 ; i < pCounter->usNumSubCounters ; i++){
		pCounter->pSubCounters[i].usSubCounterId	= i;
		pCounter->pSubCounters[i].ucEnable	= 1;
		pCounter->pSubCounters[i].ucUsed	= 1;
		pCounter->pSubCounters[i].ucTicked	= 1;
		pCounter->pSubCounters[i].usSubCounterType = pVCA5Counter->pSubCounters[i].usSubCounterType;
		pCounter->pSubCounters[i].usTrigId = pVCA5Counter->pSubCounters[i].usTrigId;
	}
}

static void	ConvertVCA5CalibInfoToVCA(VCA5_CALIB_INFO* pCalibVCA5, VCA_CALIB_ALL_INFO* pCalibInfo)
{
	memset(pCalibInfo, 0, sizeof(VCA_CALIB_ALL_INFO));
	pCalibInfo->speedUnits	= SPEED_UNITS_KPH;
	pCalibInfo->heightUnits	= HEIGHT_UNITS_METERS;

	pCalibInfo->info.fHeight = pCalibVCA5->fHeight;
	pCalibInfo->info.fTilt	= pCalibVCA5->fTilt;
	pCalibInfo->info.fRoll	= pCalibVCA5->fRoll;
	pCalibInfo->info.fFOV	= pCalibVCA5->fFOV;
}





#endif