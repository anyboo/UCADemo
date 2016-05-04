#pragma once
#include "afxcmn.h"
#include "AlarmNotification.h"
#include <map>
#include <queue>
#include "VCAConfigure.h"
#include "VCAEventSink.h"


#define MAX_NUM_ITEMS 1000

#define GET_ENG_BITS(engId)		((engId) & 0xFF)
#define GET_ALM_BITS(alarmId)		((alarmId) & 0xFFFFFF)
#define MAKE_ENG_ALM_KEY(engId, alarmId)		((GET_ENG_BITS(engId) << 24) | GET_ALM_BITS(alarmId))

// BW - the order of inheritance here is important! MFC message maps don't
// work properly for classes with multiple inheritance unless the MFC class comes first in the order...
// http://blogs.msdn.com/oldnewthing/archive/2004/02/09/70002.aspx

#define RULE_NAME_SIZE	32
typedef struct {
	TCHAR szRuleType[RULE_NAME_SIZE];
	int nRuleType;
} RULE_NAME_TYPE;

class CVCADataMgr;

class AlarmListCtrl :
	public CListCtrl,
	public IVCAEventSink,
	public std::map< int, int >
	
{
public:

//	DECLARE_DYNAMIC( AlarmListCtrl )

	AlarmListCtrl(void);
public:
	~AlarmListCtrl(void);

public:
	void Initialize();
	void SetVCADataMgr(ULONG index, CVCADataMgr* pVCADataMgr){
		m_pVCADataMgr[index] = pVCADataMgr;
	}

	// From IAlarmListCtrl
public:
	virtual HRESULT AddAlarm( AlarmNotification &stNotification );
	virtual HRESULT UpdateAlarm( AlarmNotification &stNotification );
	virtual HRESULT ClearAlarm( DWORD dwEngId, int iAlarmId );
	virtual HRESULT ClearAllAlarms( );

	virtual BOOL	ChangeVCASourceInfo(DWORD EngId, BITMAPINFOHEADER *pbm) {return TRUE;}
	virtual BOOL	ProcessVCAData(DWORD EngId, BYTE *pImage, BITMAPINFOHEADER *pbm, VCA5_TIMESTAMP *pTimestamp, 
		BYTE *pMetaData, int iMataDataLen, CVCAMetaLib *pVCAMetaLib);

protected:

	virtual void DoDataExchange( CDataExchange *pDX );

	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnContextMenuExportToCSV();
	afx_msg void OnContextMenuClearItem();
	afx_msg void OnContextMenuClearAllItems();
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg LRESULT OnUpdateElement( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()
	// Helper funcs
protected:
	void UpdateElement( AlarmNotification &stNotification );
	void DeleteElement( int iIndex );
	void DeleteAllElements();
	LRESULT	_insertElement( AlarmNotification* pNotification );
	LRESULT	_updateElement( AlarmNotification* pNotification, int iIndex );
	CString MakeEventName(unsigned short usRuleType, CString strObjectName, int zoneId);


protected:
	enum eColumns
	{
		eColEngine = 0, 
		eColAlarmId,
		eColRuleType,
		eColRuleName,
		eColZone,
		eColObjCls,
		eColStartTime,
		eColEndTime,
	};

protected:
	CImageList	m_Icons;

	static RULE_NAME_TYPE m_szRuleTypeLUT[eMaxNumAlarmTypes];
	CMenu		m_ContextMenu;
	CRITICAL_SECTION	m_cs;
	CVCADataMgr* m_pVCADataMgr[VCA5_MAX_NUM_ENGINE];
	

	BOOL		m_bSetup;

	VCA5_PACKET_OBJECTS *m_PrePacketObjects[VCA5_MAX_NUM_ENGINE];
	std::queue<AlarmNotification>	m_queAlrams;
};
