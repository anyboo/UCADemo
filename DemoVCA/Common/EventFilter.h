#pragma once

#include <map>

#define MAX_NUM_EVENT_FILTER	40
#define MAX_NUM_CONDITION		20
#define MAX_NUM_RULE			20

typedef struct {
	int	nCount;
	TCHAR rule[MAX_NUM_RULE][MAX_PATH];
} EVENT_FILTER_RULES;

typedef struct {
	int nZoneId;
	EVENT_FILTER_RULES rules;
	TCHAR szObjectName[MAX_PATH];
} EVENT_FILTER_CONDITION;

typedef struct {
	int	nCount;
	EVENT_FILTER_CONDITION condition[MAX_NUM_CONDITION];
} EVENT_FILTER_CONDITIONS;

typedef struct {
	TCHAR szFilterName[MAX_PATH];	
	EVENT_FILTER_CONDITIONS eventFilterConds;
} EVENT_FILTER;

typedef struct {
	int	nCount;
	EVENT_FILTER eventFilter[MAX_NUM_EVENT_FILTER];
} EVENT_FILTERS;


class CEventFilter
{
private:
	CEventFilter(void);
	~CEventFilter(void);

public:
	static CEventFilter *Instance();
	void	DestroySelf() {delete this;}

	HRESULT		Load(TCHAR * szFilename);
	HRESULT		Save(TCHAR * szFilename=NULL);
	TCHAR *		GetEventName(unsigned short usRuleType, CString strObjectName, int zoneId);

private:

	static		CEventFilter	*m_pInstance;

	BOOL		m_bLoad;
	
	EVENT_FILTERS m_eventFilters;
	std::map< int, const TCHAR* > m_RuleIntRuleStrMap;
};
