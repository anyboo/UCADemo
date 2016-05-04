#include "StdAfx.h"
#include "../resource.h"
#include "AlarmListCtrl.h"
#include "AlarmViewDlg.h"
#include "VCADataMgr.h"
#include <Shlwapi.h>
#include "wm_user.h"
#include "../Common/EventFilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



RULE_NAME_TYPE AlarmListCtrl::m_szRuleTypeLUT[eMaxNumAlarmTypes] =
{
	{_T("Object Presence"),				VCA5_RULE_TYPE_PRESENCE},
	{_T("Object Entered"),				VCA5_RULE_TYPE_ENTER},
	{_T("Object Exited"),				VCA5_RULE_TYPE_EXIT},
	{_T("Object Appeared"),				VCA5_RULE_TYPE_APPEAR},
	{_T("Object Disappeared"),			VCA5_RULE_TYPE_DISAPPEAR},
	{_T("Object Stopped"),				VCA5_RULE_TYPE_STOP},
	{_T("Object Dwelling"),				VCA5_RULE_TYPE_DWELL},
	{_T("Directional Violation"),		VCA5_RULE_TYPE_DIRECTION},
	{_T("Speed Violation"),				VCA5_RULE_TYPE_SPEED},
	{_T("Abandoned/Removed Object"),	VCA5_RULE_TYPE_ABOBJ},
	{_T("Tailgating"),					VCA5_RULE_TYPE_TAILGATING},
	{_T("Counting Line: Direction A"),	VCA5_RULE_TYPE_LINECOUNTER_A},
	{_T("Counting Line: Direction B"),	VCA5_RULE_TYPE_LINECOUNTER_B},
	{_T("Camera Tamper"),				VCA5_EVENT_TYPE_TAMPER},	// ??
	{_T("Abandoned/Removed Object"),	VCA5_RULE_TYPE_RMOBJ},
	{_T("Color Filter"),				VCA5_RULE_TYPE_COLSIG},
	{_T("Smoke"),						VCA5_RULE_TYPE_SMOKE},
	{_T("Fire"),						VCA5_RULE_TYPE_FIRE},
	{_T("Unknown"),						VCA5_RULE_TYPE_OTHERS}
};


//////////////////////////////////////////////////////////////////////////

//IMPLEMENT_DYNAMIC( AlarmListCtrl, CListCtrl );

AlarmListCtrl::AlarmListCtrl(void)
{
	InitializeCriticalSection( &m_cs );
	m_bSetup = FALSE;
	memset(m_pVCADataMgr, 0, sizeof(m_pVCADataMgr));
	for(DWORD i = 0 ; i < VCA5_MAX_NUM_ENGINE ;i++)
	{
		m_PrePacketObjects[i] = (VCA5_PACKET_OBJECTS*)malloc(sizeof(VCA5_PACKET_OBJECTS));
		memset(m_PrePacketObjects[i], 0, sizeof(VCA5_PACKET_OBJECTS));
	}

}

AlarmListCtrl::~AlarmListCtrl(void)
{
//	m_ContextMenu.DestroyMenu();

	for(DWORD i = 0 ; i < VCA5_MAX_NUM_ENGINE ;i++)
	{
		free(m_PrePacketObjects[i]);
	}

	EnterCriticalSection( &m_cs );
	LeaveCriticalSection( &m_cs );
	DeleteCriticalSection( &m_cs );
}



void AlarmListCtrl::DoDataExchange(CDataExchange* pDX)
{
	CListCtrl::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP( AlarmListCtrl, CListCtrl )

	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_COMMAND( ID_CONTEXTMENU_SAVEALARMSTOCSV,	&AlarmListCtrl::OnContextMenuExportToCSV )
	ON_COMMAND( ID_CONTEXTMENU_CLEARITEM,		&AlarmListCtrl::OnContextMenuClearItem )
	ON_COMMAND( ID_CONTEXTMENU_CLEARALLITEMS,	&AlarmListCtrl::OnContextMenuClearAllItems )
	ON_NOTIFY_REFLECT(NM_DBLCLK,				&AlarmListCtrl::OnNMDblclk)
	ON_MESSAGE( WM_UPDATE_ELEMENT,				&AlarmListCtrl::OnUpdateElement )

END_MESSAGE_MAP()

int AlarmListCtrl::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	return -1;
}

void AlarmListCtrl::OnDestroy()
{
	m_Icons.DeleteImageList();
	m_ContextMenu.DestroyMenu();
}

void AlarmListCtrl::Initialize()
{
	// Load up the image list
	m_Icons.Create( 16, 16, ILC_COLOR8 | ILC_MASK, 5, 5 );

	// NB - the ordering of insertion should match the AlarmType enum declared in AlarmNotification.h
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_PRESENCE ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_ENTER ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_EXIT ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_APPEAR ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_DISAPPEAR ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_TIMER ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_PRESENCE ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_DIRECTION ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_SPEED ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_ABOBJ ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_TAILGATING ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_LINECOUNTER ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_LINECOUNTER ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_TAMPER ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_ABOBJ ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_COLSIG ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_SMOKE ) );
	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_FIRE ) );
//	m_Icons.Add( AfxGetApp()->LoadIcon( IDI_ICON_AUTOTRACKING ) );

	
	SetImageList( &m_Icons, LVSIL_SMALL );

	InsertColumn( eColEngine, _T("Engine"), LVCFMT_LEFT, 50 );
	InsertColumn( eColAlarmId, _T("Id"), LVCFMT_LEFT, 42 );
	InsertColumn( eColRuleType, _T("Rule Type"), LVCFMT_LEFT, 180 );
	InsertColumn( eColRuleName, _T("Rule Name"), LVCFMT_LEFT, 180 );
	InsertColumn( eColZone, _T("Zone Name"), LVCFMT_LEFT, 100 );
	InsertColumn( eColObjCls, _T("Object Type"), LVCFMT_LEFT, 85 );
	InsertColumn( eColStartTime, _T("Start Time (UTC)"), LVCFMT_LEFT, 140 );
	InsertColumn( eColEndTime, _T("End Time (UTC)"), LVCFMT_LEFT, 140 );
	
	// Load up my context menu
	m_ContextMenu.CreatePopupMenu();

	CAPPConfigure *pAppCfg = CAPPConfigure::Instance();
	if( pAppCfg->IsAlarmSaveEnabled() && pAppCfg->IsEventExportEnabled() ){
		m_ContextMenu.AppendMenu(MF_STRING, ID_CONTEXTMENU_SAVEALARMSTOCSV, _T("Save alarms to CSV"));
	}
	m_ContextMenu.AppendMenu(MF_STRING, ID_CONTEXTMENU_CLEARITEM, _T("Clear Item"));
	m_ContextMenu.AppendMenu(MF_STRING, ID_CONTEXTMENU_CLEARALLITEMS, _T("Clear All Items"));
	
	m_bSetup = TRUE;

	clear();
}

// MFC handlers
void AlarmListCtrl::OnRButtonDown( UINT nFlags, CPoint point )
{
	ClientToScreen( &point );

	//CMenu *pPopup = m_ContextMenu.GetSubMenu(0);

	//if( pPopup )
	{
		BOOL bFFMPEGExist = PathFileExists( _T("ffmpeg.exe") );
		POSITION pos = GetFirstSelectedItemPosition();

		CAPPConfigure *pAppCfg = CAPPConfigure::Instance();
		UINT uExportToAVIFlag = (pos && bFFMPEGExist && pAppCfg->IsAlarmSaveEnabled()) ? MF_ENABLED : MF_DISABLED|MF_GRAYED;
		UINT uClearItemFlag = pos ? MF_ENABLED : MF_DISABLED|MF_GRAYED;

		m_ContextMenu.EnableMenuItem(ID_CONTEXTMENU_EXPORTTOAVI, uExportToAVIFlag);
		m_ContextMenu.EnableMenuItem(ID_CONTEXTMENU_CLEARITEM, uClearItemFlag);

		m_ContextMenu.TrackPopupMenu( TPM_LEFTALIGN | TPM_TOPALIGN, point.x, point.y, this );
	}
}



const int COLUMN_NAME_SIZE = 20;

#ifdef KOR_LANG
TCHAR szHeader[][COLUMN_NAME_SIZE] =
{
	_T("번호"),
	_T("이벤트 검지일자"),
	_T("최초 이벤트 검지 시간(시:분:초)"),
	_T("이벤트 검지 종료 시각(시:분:초)"),
	_T("이벤트 검지종류"),
	_T("영상 번호"),
	_T("비고")
};
#else
TCHAR szHeader[][COLUMN_NAME_SIZE] =
{
	_T("Id"),
	_T("Event Date"),
	_T("Start Event Time"),
	_T("End Event Time"),
	_T("Event Type"),
	_T("Video Id"),
	_T("ETC")
};
#endif


enum eReportColumns
{
	eNumber = 0, 
	eDate,
	eStartTime,
	eEndTime,
	eEvent,
	eSnapshotPath,
	eNote,
	eNumCols,
};

void AlarmListCtrl::OnContextMenuExportToCSV()
{
	USES_CONVERSION;

	// Save records on alarm list control to a file
	{
		TCHAR alarmListFile[MAX_PATH] = _T("alarm_list_control.csv");
		FILE* fpAlarmList = _tfopen(alarmListFile, _T("wt,ccs=UNICODE"));
		if(!fpAlarmList)
		{
			AfxMessageBox(_T("Failed to create alarm_list_control.csv."));
			return;
		}
		for(int iAlarm = 0; iAlarm < GetItemCount(); iAlarm++)
		{
			EnterCriticalSection( &m_cs );

			CString strEngId		= GetItemText(iAlarm , eColEngine);
			CString strAlarmId		= GetItemText(iAlarm , eColAlarmId);
			CString strRuleType		= GetItemText(iAlarm , eColRuleType);
			CString strRuleName		= GetItemText(iAlarm , eColRuleName);
			CString strZoneName		= GetItemText(iAlarm , eColZone);
			CString strObjectName	= GetItemText(iAlarm , eColObjCls);
			CString strStartTime	= GetItemText(iAlarm , eColStartTime);
			CString strEndTime		= GetItemText(iAlarm , eColEndTime);
			
			_ftprintf(fpAlarmList, _T("%s\t"), strEngId.GetBuffer());
			_ftprintf(fpAlarmList, _T("%s\t"), strAlarmId.GetBuffer());
			_ftprintf(fpAlarmList, _T("%s\t"), strRuleType.GetBuffer());
			_ftprintf(fpAlarmList, _T("%s\t"), strRuleName.GetBuffer());
			_ftprintf(fpAlarmList, _T("%s\t"), strZoneName.GetBuffer());
			_ftprintf(fpAlarmList, _T("%s\t"), strObjectName.GetBuffer());
			_ftprintf(fpAlarmList, _T("%s\t"), strStartTime.GetBuffer());
			_ftprintf(fpAlarmList, _T("%s\n"), strEndTime.GetBuffer());

			LeaveCriticalSection( &m_cs );
		}
		fclose(fpAlarmList);
	}

//	AfxMessageBox(_T("Succeeded to create alarm_list_control.csv."));

	TCHAR csvFile[MAX_PATH] = _T("alarm_list.csv");
	FILE* fpCSV = _tfopen(csvFile, _T("wt,ccs=UNICODE"));
	if(!fpCSV)
	{
		AfxMessageBox(_T("alarm_list.csv isn't opened."));
		return;
	}

	// Write title of columns
	int nColumnCount = sizeof(szHeader) / sizeof(szHeader[0]);
	for(int i = 0; ; i++)
	{
		if(i < nColumnCount-1)
		{
			_ftprintf(fpCSV, _T("%s\t"), szHeader[i]);
		}
		else
		{
			_ftprintf(fpCSV, _T("%s\n"), szHeader[i]);
			break;
		}
	}

	// Write information of alarms
	for(int iAlarm = 0; iAlarm < GetItemCount(); iAlarm++)
	{
		EnterCriticalSection( &m_cs );
		CString strEngId		= GetItemText(iAlarm , eColEngine);
		CString strAlarmId		= GetItemText(iAlarm , eColAlarmId);
		CString strRuleType		= GetItemText(iAlarm , eColRuleType);
		CString strRuleName		= GetItemText(iAlarm , eColRuleName);
		CString strZoneName		= GetItemText(iAlarm , eColZone);
		CString strObjectName	= GetItemText(iAlarm , eColObjCls);
		CString strStartTime	= GetItemText(iAlarm , eColStartTime);
		CString strEndTime		= GetItemText(iAlarm , eColEndTime);
		CString strDate			= strStartTime.Left(10);

		CAlarmViewDlg Dlg;
		Dlg.ExportSnapShots(strStartTime, strEndTime, CAPPConfigure::Instance()->GetAlarmSavePeriod(), 
			strEngId, strAlarmId, CAPPConfigure::Instance()->GetAlarmSavePath());

		CString strSnapshotPath	= Dlg.GetAviFilePath();
		strStartTime			= strStartTime.Right(8);
		strEndTime				= strEndTime.Right(8);
		int zoneId				= _ttoi(strZoneName.Right(1));
		int ruleType = 0;

		for(int i=0; i<eMaxNumAlarmTypes; i++)
		{
			CString strRuleNameLocal = m_szRuleTypeLUT[i].szRuleType;

			if(strRuleNameLocal == strRuleType)
			{
				ruleType = m_szRuleTypeLUT[i].nRuleType;
				break;
			}
		}

		CString strEventName	= MakeEventName(ruleType, strObjectName, zoneId);
		if(strEventName.GetLength() > 0)
		{
			_ftprintf(fpCSV, _T("%d\t"), iAlarm+1);
			_ftprintf(fpCSV, _T("%s\t"), strDate.GetBuffer());
			_ftprintf(fpCSV, _T("%s\t"), strStartTime.GetBuffer());
			_ftprintf(fpCSV, _T("%s\t"), strEndTime.GetBuffer());
			_ftprintf(fpCSV, _T("%s\t"), strEventName.GetBuffer());
			_ftprintf(fpCSV, _T("%s\n"), strSnapshotPath.GetBuffer());
		}

		LeaveCriticalSection( &m_cs );
	}

	fclose(fpCSV);

	CString option;
	option.Format(_T("/select,%s"), csvFile);

	ShellExecute(NULL, _T("open"), _T("explorer"), option, NULL, SW_SHOW);
}


void AlarmListCtrl::OnContextMenuClearItem()
{
	POSITION pos = GetFirstSelectedItemPosition();

	while( pos )
	{
		EnterCriticalSection( &m_cs );
		int iIndex = GetNextSelectedItem( pos );

		int iAlarmId = (int)GetItemData( iIndex );
		AlarmListCtrl::iterator it = find( iAlarmId );

		if( it != end() )
		{
			erase( it );
			DeleteElement( iIndex );
		}
		else
		{
			// Data is screwed
			ASSERT( false );
		}
		LeaveCriticalSection( &m_cs );
	}
}

void AlarmListCtrl::OnContextMenuClearAllItems()
{
	EnterCriticalSection( &m_cs );
	DeleteAllElements();
	clear();
	LeaveCriticalSection( &m_cs );
}

void AlarmListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	int i = pNMListView->iItem;

	if(i<0 || !CAPPConfigure::Instance()->IsAlarmSaveEnabled())
	{
		*pResult = 0;
		return;
	}

	CString strEngId = GetItemText(i , 0);
	CString strAlarmId = GetItemText(i , 1);
	CString strStartTime = GetItemText(i , 5);
	CString strEndTime = GetItemText(i , 6);

	CAlarmViewDlg Dlg;
	Dlg.DoModal(strStartTime, strEndTime, CAPPConfigure::Instance()->GetAlarmSavePeriod(), 
		strEngId, strAlarmId, CAPPConfigure::Instance()->GetAlarmSavePath());

	*pResult = 0;
}

// IAlarmListCtrl
HRESULT AlarmListCtrl::AddAlarm(AlarmNotification &stNotification)
{
	HRESULT hr = S_OK;

	UpdateElement( stNotification );

	return hr;
}

HRESULT AlarmListCtrl::UpdateAlarm(AlarmNotification &stNotification)
{
	HRESULT hr = S_OK;
	
	UpdateElement( stNotification );

	return hr;
}

HRESULT AlarmListCtrl::ClearAlarm( DWORD dwEngId, int iAlarmId )
{
	EnterCriticalSection( &m_cs );
	HRESULT hr = S_OK;

	AlarmListCtrl::iterator it = find( MAKE_ENG_ALM_KEY( dwEngId, iAlarmId ) );
	if( it != end() )
	{
		DeleteElement( it->second );
		erase( it );
	}
	else
	{
		// WTF?
		ASSERT( false );
		hr = E_INVALIDARG;
	}

	LeaveCriticalSection( &m_cs );
	return hr;
}

HRESULT AlarmListCtrl::ClearAllAlarms()
{
	EnterCriticalSection( &m_cs );
	HRESULT hr = S_OK;
	DeleteAllElements();
	// Wipe our mappings
	clear();
	LeaveCriticalSection( &m_cs );
	return hr;
}

void AlarmListCtrl::UpdateElement( AlarmNotification &stNotification )
{
	EnterCriticalSection( &m_cs );

	m_queAlrams.push(stNotification);

	LeaveCriticalSection( &m_cs );

	PostMessage( WM_UPDATE_ELEMENT );
}


// Helper funcs
LRESULT AlarmListCtrl::OnUpdateElement( WPARAM wParam, LPARAM lParam )
{
	EnterCriticalSection( &m_cs );

	if ( m_queAlrams.empty() )
		return 0;

	AlarmNotification notification = m_queAlrams.front();
	m_queAlrams.pop();

	LeaveCriticalSection( &m_cs );

	AlarmListCtrl::iterator it = find( MAKE_ENG_ALM_KEY( notification.dwEngId, notification.iAlarmId ) );

	if( it != end() )
	{
		// Already exists, update it
		_updateElement( &notification, it->second );
	}
	else
	{
		// Doesn't exist, add it
		_insertElement( &notification );
	}

	return 0;
}

LRESULT AlarmListCtrl::_insertElement( AlarmNotification* pNotification )
{
	// Check for overflow
	while( size() > MAX_NUM_ITEMS )
	{
		// Find the oldest
		int iIdx = GetItemCount() - 1;
		int iAlarmId = (int) GetItemData( iIdx );

		DeleteElement( iIdx );
		AlarmListCtrl::iterator it = find( iAlarmId );
		if ( it != end() )
			erase( it );
	}

	// Add this into the list
	// Add it in at the most recent position
	LVITEM lvItem;
	TCHAR tcsBuf[256];

	_itot_s( pNotification->dwEngId, tcsBuf, 128, 10 );
	memset( &lvItem, NULL, sizeof( LVITEM ) );
	lvItem.iItem		= 0;
	lvItem.iSubItem		= 0;
	lvItem.pszText		= tcsBuf;
	lvItem.cchTextMax	= (int)_tcslen( lvItem.pszText );
	lvItem.iImage		= pNotification->eAlarmType;
	lvItem.lParam		= MAKE_ENG_ALM_KEY( pNotification->dwEngId, pNotification->iAlarmId );
	lvItem.mask			= LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;

	int iIdx = InsertItem( &lvItem );

	ASSERT( iIdx > -1 );

	// Now go through and update the indices of each element
	// Everything will have moved down by 1
	AlarmListCtrl::iterator it;
	for( it = begin(); it != end(); it++ )
	{
		it->second++;
	}

	// Add to map
	insert( std::pair< int, int >(MAKE_ENG_ALM_KEY( pNotification->dwEngId, pNotification->iAlarmId), 0) );

	// Update the mapping
	it = find( MAKE_ENG_ALM_KEY( pNotification->dwEngId, pNotification->iAlarmId ) );
	if( it != end() )
	{
		_updateElement( pNotification, it->second );
	}
	else
	{
		// WTF?
		ASSERT( false );
	}

	return 0;
}
LRESULT AlarmListCtrl::_updateElement( AlarmNotification* pNotification, int iIndex )
{
	CTime cTime;
	// Set the values for all other columns
	LVITEM lvItem;
	memset( &lvItem, NULL, sizeof( LVITEM ) );
	TCHAR tcsBuf[1024];
	USES_CONVERSION;

	if( pNotification->fMask & eMaskAlarmId )
	{
		_itot_s( pNotification->iAlarmId, tcsBuf, 128, 10 );
		lvItem.iItem		= iIndex;
		lvItem.iSubItem		= eColAlarmId;
		lvItem.pszText		= tcsBuf;
		lvItem.cchTextMax	= (int)_tcslen( lvItem.pszText );
		lvItem.mask			= LVIF_TEXT;

		SetItem( &lvItem );
	}

	if( pNotification->fMask & eMaskAlarmType )
	{
		lvItem.iItem		= iIndex;
		lvItem.iSubItem		= eColRuleType;
		lvItem.pszText		= m_szRuleTypeLUT[ pNotification->eAlarmType ].szRuleType;
		lvItem.cchTextMax	= (int)_tcslen( lvItem.pszText );
		lvItem.mask			= LVIF_TEXT;

		SetItem( &lvItem );
	}

	if( pNotification->fMask & eMaskAlarmName )
	{
		USES_CONVERSION;
		lvItem.iItem		= iIndex;
		lvItem.iSubItem		= eColRuleName;
		lvItem.pszText		= A2T(pNotification->csRuleName);
		lvItem.cchTextMax	= (int)_tcslen( lvItem.pszText );
		lvItem.mask			= LVIF_TEXT;

		SetItem( &lvItem );
	}

/*	if( pNotification->fMask & eMaskZoneId )
	{
		// Zone id
		_itot_s( pNotification->iZoneId, tcsBuf, 128, 10 );
		lvItem.iItem		= iIndex;
		lvItem.iSubItem		= eColZone;
		lvItem.pszText		= tcsBuf;
		lvItem.cchTextMax	= (int)_tcslen( tcsBuf );
		lvItem.mask			= LVIF_TEXT;

		SetItem( &lvItem );
	}*/
	
	if( pNotification->fMask & eMaskZoneName )
	{
		// Zone Name
		VCA5_APP_ZONE *pZone = m_pVCADataMgr[pNotification->dwEngId]->GetZoneById(pNotification->iZoneId);
		if (pZone && pZone->szZoneName[0] != '\0')
		{
			_tcscpy(tcsBuf, CA2T(pZone->szZoneName, CP_UTF8));
		}
		else
		{
			if(pNotification->eAlarmType == eTamper)
			{
				_stprintf_s(tcsBuf, _countof(tcsBuf), _T("%s"), _T("---"));
			}
			else
			{
				_stprintf_s(tcsBuf, _countof(tcsBuf), _T("%s"), _T("Unknown"));
			}
		}
		lvItem.iItem		= iIndex;
		lvItem.iSubItem		= eColZone;
		lvItem.pszText		= tcsBuf;
		lvItem.cchTextMax	= (int)_tcslen( lvItem.pszText );
		lvItem.mask			= LVIF_TEXT;

		SetItem( &lvItem );
	}

	if( pNotification->fMask & eMaskObjCls )
	{
		// Object class
		if (-1 == pNotification->iObjCls )
		{
			_stprintf_s(tcsBuf, _countof(tcsBuf), _T("%s"), _T("Unclassified"));
		}
		else
		{
			VCA5_APP_CLSOBJECT *pClsObj = m_pVCADataMgr[pNotification->dwEngId]->GetClsObject(pNotification->iObjCls);
			if (pClsObj)
			{
				_tcscpy(tcsBuf, CA2T(&pClsObj->szClsobjectName[0], CP_UTF8));
			}
			else
			{
				_stprintf_s(tcsBuf, _countof(tcsBuf), _T("%s"), _T("Unknown"));
			}
		}

		lvItem.iItem		= iIndex;
		lvItem.iSubItem		= eColObjCls;
		lvItem.pszText		= tcsBuf;
		lvItem.cchTextMax	= (int)_tcslen( lvItem.pszText );
		lvItem.mask			= LVIF_TEXT;

		SetItem( &lvItem );
	}

	if( pNotification->fMask & eMaskStartTime )
	{
		// Start time
		cTime = CTime( (time_t) pNotification->i64StartTime );
		struct tm	osTime;
		cTime.GetLocalTm( &osTime );
		CString s;
		s.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),osTime.tm_year+1900, osTime.tm_mon + 1,osTime.tm_mday, osTime.tm_hour, osTime.tm_min, osTime.tm_sec );

		lvItem.iItem		= iIndex;
		lvItem.iSubItem		= eColStartTime;
		lvItem.pszText		= s.GetBuffer(0);
		lvItem.cchTextMax	= (int)_tcslen( lvItem.pszText );
		lvItem.mask			= LVIF_TEXT;

		SetItem( &lvItem );
	}

	if( pNotification->fMask & eMaskStopTime )
	{
		// End time
		cTime = CTime( (time_t) pNotification->i64StopTime );
		struct tm	osTime;
		cTime.GetLocalTm( &osTime );
		CString s;
		s.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),osTime.tm_year+1900, osTime.tm_mon + 1,osTime.tm_mday, osTime.tm_hour, osTime.tm_min, osTime.tm_sec );

		lvItem.iItem		= iIndex;
		lvItem.iSubItem		= eColEndTime;
		lvItem.pszText		= s.GetBuffer(0);
		lvItem.cchTextMax	= (int)_tcslen( lvItem.pszText );
		lvItem.mask			= LVIF_TEXT;

		SetItem( &lvItem );
	}
	return 0;
}


void AlarmListCtrl::DeleteElement( int iIndex )
{
	// Find this element and delete it
	DeleteItem( iIndex );
}

void AlarmListCtrl::DeleteAllElements()
{
	DeleteAllItems();
}


BOOL	AlarmListCtrl::ProcessVCAData(DWORD EngId, BYTE *pImage, BITMAPINFOHEADER *pbm, VCA5_TIMESTAMP *pTimestamp, BYTE *pMetaData, int iMataDataLen, CVCAMetaLib *pVCAMetaLib)
{
	AlarmNotification al;
	memset( &al, 0, sizeof( al ) );
	VCA5_PACKET_EVENT *pEvent;

	VCA5_RULE *pRule;
	VCA5_PACKET_OBJECT *pObject = NULL;
	VCA5_PACKET_EVENTS *pPacketEvents = pVCAMetaLib->GetEvents();
	VCA5_PACKET_OBJECTS *pPacketObjects = pVCAMetaLib->GetObjects();
	if(m_bSetup && m_pVCADataMgr[EngId] && pPacketEvents->ulTotalEvents)
	{
		for (DWORD i = 0; i < pPacketEvents->ulTotalEvents ; i++ )
		{
			pEvent	= &pPacketEvents->Events[i];
			pObject = NULL;


			al.dwEngId		= EngId;
			al.iAlarmId		= pEvent->ulId;
			al.iZoneId		= pEvent->ulZoneId;
			strncpy_s( al.csRuleName, pEvent->szRuleName, sizeof(al.csRuleName) );
			strncpy_s( al.csZoneName, pEvent->szZoneName, sizeof(al.csZoneName) );
			al.i64StartTime = pEvent->tStartTime.ulSec;
			al.i64StopTime	= pEvent->tStopTime.ulSec;
			al.fMask		= eMaskEngineId | eMaskAlarmId | eMaskZoneName | eMaskAlarmType | eMaskAlarmName | eMaskStartTime | eMaskStopTime;
			
			for (DWORD j = 0; j < pPacketObjects->ulTotalObject ; j++ )
			{
				if(pPacketObjects->Objects[j].ulId == pEvent->ulObjId)	pObject = &pPacketObjects->Objects[j];
			}
			//In case of Presence version ..

			//When Disappear Object did not find, so we need to consideration.
			if(!pObject)
			{
				for (DWORD j = 0; j < m_PrePacketObjects[EngId]->ulTotalObject ; j++ )
				{
					if(m_PrePacketObjects[EngId]->Objects[j].ulId == pEvent->ulObjId)
					{
						pObject = &m_PrePacketObjects[EngId]->Objects[j];
					}
				}
			}


			if(pObject)
			{
				if (pObject->iClassificationId >= -1 && pObject->iClassificationId < VCA5_MAX_NUM_OBJECTS)
				{
					al.iObjCls	= pObject->iClassificationId;
					al.fMask	|= eMaskObjCls;
				}
			}


			if( VCA5_EVENT_TYPE_TAMPER == pEvent->ulEventType )
			{
				al.eAlarmType = eTamper;
				al.iZoneId = -1;
				AddAlarm( al );
			}
			else
			{
				pRule = m_pVCADataMgr[EngId]->GetRuleByRealId(pEvent->ulRuleId);
				if (!pRule) continue;

				switch( pRule->usRuleType )
				{
					case VCA5_RULE_TYPE_PRESENCE:		al.eAlarmType = ePresence;		break;
					case VCA5_RULE_TYPE_ENTER:			al.eAlarmType = eEntered;		break;
					case VCA5_RULE_TYPE_EXIT:			al.eAlarmType = eExited;		break;
					case VCA5_RULE_TYPE_APPEAR:			al.eAlarmType = eAppeared;		break;
					case VCA5_RULE_TYPE_DISAPPEAR:		al.eAlarmType = eDisappeared;	break;
					case VCA5_RULE_TYPE_STOP:			al.eAlarmType = eStopped;		break;
					case VCA5_RULE_TYPE_DIRECTION:		al.eAlarmType = eDirection;		break;
					case VCA5_RULE_TYPE_DWELL:			al.eAlarmType = eDwell;			break;
					case VCA5_RULE_TYPE_SPEED:			al.eAlarmType = eSpeed;			break;
					case VCA5_RULE_TYPE_TAILGATING:		al.eAlarmType = eTailgating;	break;
//					case VCA5_RULE_TYPE_ABOBJ:			al.eAlarmType = eAbObj;			break;
					case VCA5_RULE_TYPE_LINECOUNTER_A:	al.eAlarmType = eLineCounterA;	break;
					case VCA5_RULE_TYPE_LINECOUNTER_B:	al.eAlarmType = eLineCounterB;	break;
					case VCA5_RULE_TYPE_RMOBJ:			al.eAlarmType = eRmObj;			break;
					case VCA5_RULE_TYPE_COLSIG:			al.eAlarmType = eColFilter;		break;
					case VCA5_RULE_TYPE_SMOKE:			al.eAlarmType = eSmoke;			break;
					case VCA5_RULE_TYPE_FIRE:			al.eAlarmType = eFire;			break;
					default:							al.eAlarmType = eUnknown;		break;
				}
			
				switch ( pEvent->ulStatus )
				{
				case VCA5_EVENT_STATUS_INSTANT:
				case VCA5_EVENT_STATUS_START:
					AddAlarm( al );
					break;
				case VCA5_EVENT_STATUS_END:
					UpdateAlarm( al );
					break;
				case VCA5_EVENT_STATUS_UPDATE:
					UpdateAlarm( al );
					break;
				}
			}
		}

		memcpy(m_PrePacketObjects[EngId],pPacketObjects,sizeof(VCA5_PACKET_OBJECTS));
		return TRUE;
	}
	else
	{
		memcpy(m_PrePacketObjects[EngId],pPacketObjects,sizeof(VCA5_PACKET_OBJECTS));
		return FALSE;
	}
}

CString AlarmListCtrl::MakeEventName(unsigned short usRuleType, CString strObjectName, int zoneId)
{
	CEventFilter *pEventFilter = CEventFilter::Instance();
	return pEventFilter->GetEventName(usRuleType, strObjectName, zoneId);
}