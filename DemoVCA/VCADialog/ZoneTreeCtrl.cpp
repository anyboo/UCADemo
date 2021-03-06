#include "stdafx.h"
#include "ZoneTreeCtrl.h"
#include "string.h"
#include "../resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define RGB2(r,g,b)	((r<<16)+(g<<8)+b)
#define GETBVALUE(rgb)  ((BYTE) (rgb)) 
#define GETGVALUE(rgb)  ((BYTE) (((WORD) (rgb)) >> 8)) 
#define GETRVALUE(rgb)	((BYTE) ((rgb) >> 16)) 
#define GETALPHA(argb)	((BYTE) ((argb) >> 24)) 

#define RGB2TORGB( color ) RGB( GETRVALUE(color), GETGVALUE(color), GETBVALUE(color) )


COLORREF  gPieColours[VCA5_APP_PALETTE_SIZE] = {RGB2(0,0,0),RGB2(150,75,0),RGB2(100,100,100),RGB2(0,0,200),RGB2(0,150,0),RGB2(0,255,255),RGB2(255,0,0),RGB2(200,0,200),RGB2(255,255,0),RGB2(255,255,255)};
char *gPieColourNames[VCA5_APP_PALETTE_SIZE] = {"Black","Brown","Grey","Blue","Green","Cyan","Red","Pink","Yellow","White"};


LPCTSTR strTreeItem[VCA5_RULE_TYPE_NUM-1][VCA5_APP_MAX_NUM_OPTIONSPERRULE+1] = {
	{	_T("Presence"),			_T(""),					_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Enter"),			_T(""),					_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Exit"),				_T(""),					_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Appear"),			_T(""),					_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Disappear"),		_T(""),					_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Stopped"),			_T("Time(secs)"),		_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Dwell"),			_T("Threshold(secs)"),	_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Direction Filter"),	_T("Direction"),		_T("Acceptance Angle"),	_T(""),				_T(""),			_T("")	},
	{	_T("Speed Filter"),		_T("Lower bound"),		_T("Upper bound"),		_T(""),				_T(""),			_T("")	},
	{	_T("Area Filter"),		_T("Lower bound (sqm)"), _T("Upper bound (sqm)"),_T(""),			_T(""),			_T("")	},
	{	_T("Tailgating"),		_T("Threshold (secs)"),	_T("Trigger"),			_T(""),				_T(""),			_T("")	},
	{	_T(""),	_T(""),		_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Cnt Line - Dir A"),	_T("Direction A"),		_T("Shadow Filter"),	_T("Counting Line"),_T(""),			_T("")	},
	{	_T("Cnt Line - Dir B"),	_T("Direction B"),		_T("Width Calibration"),_T("Counting Line"),_T(""),			_T("")	},
	{	_T("Abandoned/Removed Object"),	_T("Time (secs)"),		_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Color Filter"),		_T("Color"),			_T("Threshold"),		_T(""),				_T(""),			_T("")	},
	{	_T("Smoke"),			_T("Time(secs)"),		_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Fire"),				_T("Time(secs)"),		_T(""),					_T(""),				_T(""),			_T("")	},
	{	_T("Others"),			_T(""),					_T(""),					_T(""),				_T(""),			_T("")	},
};

LPCTSTR strSpeedUnits[2] = { _T(" (km/h)"),	_T(" (mph)") };

LPCTSTR strAreaUnits[2]	= { _T(" (sqm)"),	_T(" (sqm)") };

char	gszFobiddenChar[] = "<>[],:";

LPCTSTR	gszWarning[1] = { _T("<>[],: are forbidden characters for zone/rule/counter names. All forbidden characters will be removed automatically.") };

	
static UINT  s_uIconTable[NUM_ICONS][2]= { 
	{	IDI_ICON_ZONE,		0	},
	{	IDI_ICON_PRESENCE,	1	},
	{	IDI_ICON_ENTER,		2	},
	{	IDI_ICON_EXIT,		3	},
	{	IDI_ICON_APPEAR,	4	},
	{	IDI_ICON_DISAPPEAR,	5	},
	{	IDI_ICON_TIMER,		6	},
	{	IDI_ICON_DIRECTION,	7	},
	{	IDI_ICON_SPEED,		8	},
	{	IDI_ICON_TAILGATING, 9	},
	{	IDI_ICON_LINECOUNTER,10	},	
	{	IDI_ICON_ABOBJ,		11	},
	{	IDI_ICON_RMOBJ,		12	},
	{	IDI_ICON_COUNTER,	13	},
	{	IDI_ICON_SMOKE,		14	},
	{	IDI_ICON_FIRE,		15	},
	{	IDI_ICON_PERSON,	16	},
	{	IDI_ICON_VEHICLE,	17	},
	{	IDI_ICON_CLUTTER,	18	},
	{	IDI_ICON_OBJFLT,	19	},
	{	IDI_ICON_UNKNOWN,	20	},
	{	IDI_ICON_GOP,		21	},
	{	IDI_ICON_COLSIG,	22	}
};


static int GETICONINDEX( UINT uICON )
{
	int	i;
	for ( i = 0; i < NUM_ICONS; i ++ )
		if ( s_uIconTable[i][0] == uICON )
			return s_uIconTable[i][1];
	return TV_NOIMAGE;
}

// icon table for rules
static UINT uIconItemZone[VCA5_RULE_TYPE_NUM-1]={
	GETICONINDEX(IDI_ICON_PRESENCE),
	GETICONINDEX(IDI_ICON_ENTER),
	GETICONINDEX(IDI_ICON_EXIT),
	GETICONINDEX(IDI_ICON_APPEAR),
	GETICONINDEX(IDI_ICON_DISAPPEAR),
	GETICONINDEX(IDI_ICON_TIMER),
	GETICONINDEX(IDI_ICON_PRESENCE),
	GETICONINDEX(IDI_ICON_DIRECTION),
	GETICONINDEX(IDI_ICON_SPEED),
	GETICONINDEX(IDI_ICON_SPEED),
	GETICONINDEX(IDI_ICON_TAILGATING),
	GETICONINDEX(IDI_ICON_ABOBJ),
	GETICONINDEX(IDI_ICON_LINECOUNTER),
	GETICONINDEX(IDI_ICON_LINECOUNTER),
	GETICONINDEX(IDI_ICON_ABOBJ),
	GETICONINDEX(IDI_ICON_COLSIG),
	GETICONINDEX(IDI_ICON_SMOKE),
	GETICONINDEX(IDI_ICON_FIRE),
};


static UINT uIconItemCounter[VCA5_APP_MAX_TYPE_COUNTERS]={
	GETICONINDEX(IDI_ICON_COUNTER),
	GETICONINDEX(IDI_ICON_COUNTER),
	GETICONINDEX(IDI_ICON_COUNTER),
	GETICONINDEX(IDI_ICON_COUNTER),
	GETICONINDEX(IDI_ICON_COUNTER),
};


static OBJCLS_ICON_T uIconObjcls[NUM_PREDEFINED_OBJICONS]={
	{ GETICONINDEX(IDI_ICON_PERSON),	_T("Person")	},
	{ GETICONINDEX(IDI_ICON_VEHICLE),	_T("Vehicle")	},
	{ GETICONINDEX(IDI_ICON_CLUTTER),	_T("Clutter")	},
	{ GETICONINDEX(IDI_ICON_GOP),		_T("Group of People") },
};

#define VALUE_NA	127

UINT GetObjectIcon( LPCTSTR pszObjectName )
{
	CString	s1(pszObjectName);
	s1.MakeLower();
	for ( int i = 0; i < NUM_PREDEFINED_OBJICONS; i++ )
	{
		CString s2( uIconObjcls[i].strName );
		s2.MakeLower();
		if ( s1 == s2 )
			return uIconObjcls[i].uICON;
	}
	return GETICONINDEX(IDI_ICON_UNKNOWN);
}

CZoneTreeCtrl::CZoneTreeCtrl()
{
	m_pDataMgr		= NULL;

	m_iTreeType		= TREELIST_NULL;
	m_iPreRule		= NOT_IN_USE;
	m_iCurRule		= NOT_IN_USE;
	m_iPreRule		= NOT_IN_USE;
	m_iCurRule		= NOT_IN_USE;
	m_iPreOption	= NOT_IN_USE;
	m_iCurOption	= NOT_IN_USE;
	m_bImageChanged = FALSE;
	m_bEditing		= FALSE;
	
	memset(&m_tRuleCounterCombo, 0, sizeof(m_tRuleCounterCombo));
	m_tRuleCounterCombo.uiNumComboItems = 0;
	for ( int i = 0; i < VCA5_MAX_NUM_ZONES*VCA5_MAX_NUM_RULES; i++)
		m_tRuleCounterCombo.pComboItem[i].pszComboString = new TCHAR[80];

	memset(&m_tZoneColorCombo, 0, sizeof(m_tZoneColorCombo));
	m_tZoneColorCombo.uiNumComboItems = 0;
	for ( int i = 0; i < MAXZONECOLORS; i++) {
		m_tZoneColorCombo.pComboItem[i].pszComboString = new TCHAR[80];
	}
	BuildZoneColorCombo();

	memset(&m_tColorBinCombo, 0, sizeof(m_tColorBinCombo));
	m_tColorBinCombo.uiNumComboItems = 0;
	for ( int i = 0; i < MAX_NUM_COMBOITEMS; i++) {
		m_tColorBinCombo.pComboItem[i].pszComboString = new TCHAR[80];
	}

	memset(&m_tZoneTypeCombo, 0, sizeof(m_tZoneTypeCombo));
	for ( int i = 0; i < 2; i++) {
		m_tZoneTypeCombo.pComboItem[i].pszComboString = new TCHAR[20];
	}

	memset(&m_tObjClsIdCombo, 0, sizeof(m_tObjClsIdCombo));
	for (int i = 0; i < VCA5_MAX_NUM_CLSOBJECTS; i++) {
		m_tObjClsIdCombo.pComboItem[i].pszComboString = new TCHAR[VCA5_APP_MAX_NAME];
	}

	memset(&m_tObjClsLogicCombo, 0, sizeof(m_tObjClsLogicCombo));
	for (int i = 0; i < 2; i++) {
		m_tObjClsLogicCombo.pComboItem[i].pszComboString = new TCHAR[20];
	}
	
	memset(&m_tTailGateModeCombo, 0, sizeof(m_tTailGateModeCombo));
	for (int i = 0; i < 2; i++) {
		m_tTailGateModeCombo.pComboItem[i].pszComboString = new TCHAR[20];
	}
	

	memset(m_pObjClsFilters, 0, sizeof(m_pObjClsFilters));
}

CZoneTreeCtrl::~CZoneTreeCtrl()
{
	for ( int i = 0; i < VCA5_MAX_NUM_ZONES*VCA5_MAX_NUM_RULES; i++)
	{
		if ( m_tRuleCounterCombo.pComboItem[i].pszComboString )
			delete[] m_tRuleCounterCombo.pComboItem[i].pszComboString;
	}
	for ( int i = 0; i < MAXZONECOLORS; i++)
	{
		if ( m_tZoneColorCombo.pComboItem[i].pszComboString )
			delete[] m_tZoneColorCombo.pComboItem[i].pszComboString;
	}
	for ( int i = 0; i < MAX_NUM_COMBOITEMS; i++)
	{
		if ( m_tColorBinCombo.pComboItem[i].pszComboString )
			delete[] m_tColorBinCombo.pComboItem[i].pszComboString;
	}
	for ( int i = 0; i < 2; i++)
	{
		if ( m_tZoneTypeCombo.pComboItem[i].pszComboString )
			delete[] m_tZoneTypeCombo.pComboItem[i].pszComboString;
	}
	for (int i = 0; i < VCA5_MAX_NUM_CLSOBJECTS; i++) {
		if (m_tObjClsIdCombo.pComboItem[i].pszComboString) 
			delete[] m_tObjClsIdCombo.pComboItem[i].pszComboString;
	}
	for (int i = 0; i < 2; i++) {
		if (m_tObjClsLogicCombo.pComboItem[i].pszComboString)
			delete[] m_tObjClsLogicCombo.pComboItem[i].pszComboString;
	}
	for (int i = 0; i < 2; i++) {
		if (m_tTailGateModeCombo.pComboItem[i].pszComboString)
			delete[] m_tTailGateModeCombo.pComboItem[i].pszComboString;
	}
}

BOOL CZoneTreeCtrl::Setup(CWnd* pWnd, CVCADataMgr *pDataMgr)
{
	CRect		sRect;
	CPoint		sPoint(0,0);
	unsigned	uStyle;
	unsigned	uExStyle;

	uStyle		= TVS_HASBUTTONS|TVS_HASBUTTONS|TVS_HASLINES|TVS_FULLROWSELECT | TVS_CHECKBOXES| TVS_EDITLABELS;
	uExStyle	= TVS_EX_ITEMLINES|TVS_EX_ITEMLINES|TVS_EX_ALTERNATECOLOR|TVS_EX_SUBSELECT ;//|TVS_EX_BITCHECKBOX;
	
	m_ImageList.Create( 16, 16 ,ILC_COLOR8 | ILC_MASK, 5, 10);
	for ( int i = 0 ; i < NUM_ICONS; i++ )
		m_ImageList.Add( AfxGetApp()->LoadIcon(s_uIconTable[i][0]) );
	
	Create(WS_CHILD|WS_BORDER,CRect(0,0,0,0),pWnd,IDC_TREELIST);
	
	SetImageList(&m_ImageList,TVSIL_NORMAL);
	InsertColumn(0,_T("Tree"),0,160);
	InsertColumn(1,_T("Value"),0,128 );
	SetStyle(uStyle|WS_CHILD|WS_BORDER);
	SetExtendedStyle(uExStyle);
	SetUserDataSize(sizeof( VCA_TREELIST_LINE_T ) );

	m_pParentWnd= pWnd;
	m_pDataMgr	= pDataMgr;
	
	m_pDataMgr->RegisterObserver(this);
	BuildObjclsFilter();

	return TRUE;
}

void CZoneTreeCtrl::Destroy()
{
	m_pDataMgr->UnregisterObserver( this );
	m_ImageList.DeleteImageList();
	DestroyWindow();
}

void CZoneTreeCtrl::SetItemTick( HTREEITEM hItem, unsigned char ucTick )
{
	SetItemState( hItem, INDEXTOSTATEIMAGEMASK( ucTick + 1 ), TVIS_STATEIMAGEMASK );
	SetItemBkColor( hItem, 0, RGB(255,255,255) );
	SetItemBkColor( hItem, 1, RGB(255,255,255) );
}

void CZoneTreeCtrl::SetItemExpand( HTREEITEM hItem, unsigned char ucExpand )
{
	if ( ucExpand )
		Expand( hItem, TVE_EXPAND );
}



/************************************** For Zones Tree ************************************/

void CZoneTreeCtrl::FindMatchZoneInTree(VCA5_APP_ZONE *pZone)
{
	HTREEITEM hLoop; 
	hLoop = GetChildItem( m_hRoot );
//	hLoop = GetFirstSelected();
	do
	{
		hLoop = GetNextItem(hLoop, TVGN_NEXT);
	}while (hLoop!=NULL);
}

void CZoneTreeCtrl::RemoveZoneFromTree(VCA5_APP_ZONE *pZone)
{
	DeleteAllItems( );
}

void CZoneTreeCtrl::AddPredefinedZoneRules(VCA5_APP_ZONE *pZone)
{
	int iZoneIdx;
	VCA5_APP_RULE *pRule;

	if (m_pDataMgr->CheckFeature( VCA5_FEATURE_CALIBRATION )){
		Add_ObjectFilter_Tree(pZone);
		BuildObjclsIdCombo();
		BuildObjclsLogicCombo();
	}

	iZoneIdx = m_pDataMgr->GetZoneIdx(pZone);
	for (int i = 0; i < VCA5_RULE_TYPE_NUM; i++)
	{
		pRule = m_pDataMgr->GetRule(iZoneIdx, (VCA5_RULE_TYPE_E) i);
		if (pRule) {
			AddRuleToZone( pZone,  pRule );
		}
	}
}

int CZoneTreeCtrl::CheckZoneObjCls( OBJCLS_FILTER_T *pOF )
{
	if ( pOF->cId != VALUE_NA )
	{
		if ( pOF->ucIsNew )
		{
			pOF->ucIsNew = 0;
			pOF->ucTicked = 1;
			VCA5_APP_AREA_T_E	eAreaType;
			int			iAreaIdx;
			m_pDataMgr->GetSelectAreaIdx( &iAreaIdx, &eAreaType );
			OBJCLS_FILTERS_T *pObjFilters = &m_pObjClsFilters[iAreaIdx];
			int idx = pObjFilters->iTotalTrkObjs++;
			pObjFilters->tObjclsFilter[idx].cId = VALUE_NA;
			pObjFilters->tObjclsFilter[idx].ucTicked = 0;
			pObjFilters->tObjclsFilter[idx].ucIsNew = 1;
		}
		return TRUE;
	}
	else
		return FALSE;
}


void CZoneTreeCtrl::Add_ObjectFilter_Tree( VCA5_APP_ZONE *pZone )
{
	CString		s;
	HTREEITEM	hParentItem, hItemFilter;
	UINT		uIcon = 13;
	int			i;
	LPCTSTR		strLogic;
	OBJCLS_FILTERS_T	*pOFs;
	USES_CONVERSION;
	//int			iZoneIdx;

	//iZoneIdx = m_pDataMgr->GetZoneIdx(pZone);
	//if (iZoneIdx == NOT_IN_USE) return;
	//pOFs = &m_pObjClsFilters[iZoneIdx];
	pOFs = GetObjclsFilter(pZone->usZoneId);

	s.Format( _T("Object Class Filter") );
	hParentItem = InsertItem( s, uIcon , uIcon, m_hRoot);
	SetItemTick( hParentItem, -1 );
	// Always expand
	SetItemExpand( hParentItem, 1/*pOFs->ucExpanded*/ );

	VCA_TREELIST_LINE_T	tTreeLine;
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 0;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fExpand ;
	tTreeLine.pItem[0].pucExpanded = &pOFs->ucExpanded;
	tTreeLine.pItem[1].usFlags = fStatic;
	tTreeLine.pItem[1].pData = &pOFs;

	InsertUserData( hParentItem, &tTreeLine );

	{
		
		switch( pOFs->ucLogic )
		{
		case TRKOBJ_INCLUDE:
			strLogic = _T("Include") ;
			break;
		case TRKOBJ_EXCLUDE:
			strLogic = _T("Exclude");
			break;
		default:
			strLogic = _T("");
			break;
		}

		s.Format(_T("Include/Exclude") );
		HTREEITEM hItem = InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem );
		SetItemText( hItem,strLogic, 1 );
	
		SetItemTick( hItem, -1 );
		// Set itemdata
		VCA_TREELIST_LINE_T	tTreeLine;
		tTreeLine.eType = VCA_OP_ZONE;
		tTreeLine.usCanEditRuleName = 0;
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fStatic;
		tTreeLine.pItem[0].pData = pZone;;
		tTreeLine.pItem[1].usFlags = fCombo;
		tTreeLine.pItem[1].eEditType = cZoneObjclsLogic;
		tTreeLine.pItem[1].pData = &pOFs->ucLogic;
		InsertUserData( hItem, &tTreeLine );
	}

	//for ( i = 0; i < m_pObjClsFilters[iZoneIdx].iTotalTrkObjs; i++ )
	for ( i = 0; i < pOFs->iTotalTrkObjs; i++ )
	{
		//OBJCLS_FILTER_T *pObjFilter = &m_pObjClsFilters[iZoneIdx].tObjclsFilter[i];
		OBJCLS_FILTER_T *pObjFilter = &pOFs->tObjclsFilter[i];
		CString strType;

		if ( pObjFilter->cId == VCA5_APP_CLSOBJECT_UNCLASSIFIED_ID) {
			strType = _T("Unclassified");
		}
		else if ( pObjFilter->cId != VALUE_NA ) 
		{
			strType = CA2T(m_pDataMgr->GetClsObjectById(pObjFilter->cId)->szClsobjectName, CP_UTF8);
//			strType = A2T(m_pDataMgr->GetClsObjectById(pObjFilter->cId)->szClsobjectName);
		}
		else
		{
			strType = _T("");
		}

		{
			UINT	uIcon = GetObjectIcon( strType );
			s.Format(_T("Filter %d"), i);
			hItemFilter = InsertItem( s, uIcon, uIcon, hParentItem );
			// Set tick box
			SetItemTick( hItemFilter, pObjFilter->ucTicked);
		
			// Set type
			SetItemText( hItemFilter, strType, 1 );
			// Set itemdata
			VCA_TREELIST_LINE_T	tTreeLine;
			tTreeLine.eType = VCA_OP_ZONE;
			tTreeLine.usCanEditRuleName = 0;
			tTreeLine.usNumColumns = 2;
			tTreeLine.pItem[0].usFlags = fStatic | fTickBox;
			tTreeLine.pItem[0].pData = &pObjFilter->ucTicked;
			tTreeLine.pItem[1].usFlags = fCombo;
			tTreeLine.pItem[1].eEditType = cZoneObjclsId;
			tTreeLine.pItem[1].pData = pObjFilter;
			InsertUserData( hItemFilter, &tTreeLine );
		}
	}
}


void CZoneTreeCtrl::Add_Stock_BASICT_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule )
{
	CString		s;
	int iType = pRule->usRuleType - 1;
	HTREEITEM hParentItem;
	UINT uIcon = uIconItemZone[iType];
	int nEnable = m_pDataMgr->CheckFeatureByRuleType(pRule->usRuleType);

	s.Format( strTreeItem[iType][0] );
	hParentItem = InsertItem( s, uIcon , uIcon, m_hRoot);
	SetItemText( hParentItem, _T(" "), 1 );


	// Insert userdata
	VCA_TREELIST_LINE_T	tTreeLine;
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 1;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fTickBox | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[0].pucTicked = &pRule->ucTicked;
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edEventName;
	tTreeLine.pItem[1].pData = pRule->szRuleName;

	InsertUserData( hParentItem, &tTreeLine );

	if ( !nEnable ){
		SetItem( hParentItem, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
		SetItemTick( hParentItem, -1 );
		return;
	}

	// Set tick box
	SetItemTick( hParentItem, pRule->ucTicked);

	if (pRule->ucTicked)
		SetItem( hParentItem, 1, TVIF_TEXT,  CA2T(pRule->szRuleName, CP_UTF8), 0,0,0,0,0);

	SetItemExpand( hParentItem, pRule->ucExpanded);
}


void CZoneTreeCtrl::Add_Stock_LINECOUNTER_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule )
{
	CString		s;
	int iType = pRule->usRuleType - 1;
	HTREEITEM hChildItem1, hChildItem2, hChildItem3;
	UINT uIcon = uIconItemZone[iType];
	VCA_TREELIST_LINE_T	tTreeLine;
	HTREEITEM		hParentItem;

	//check 
	int nEnable = m_pDataMgr->CheckFeatureByRuleType(pRule->usRuleType);
	
	static VCA5_APP_RULE	*pRuleA;
	
	if (pRule->usRuleType == VCA5_RULE_TYPE_LINECOUNTER_A)
	{
		pRuleA = pRule;
	}
	else if(pRule->usRuleType == VCA5_RULE_TYPE_LINECOUNTER_B)
	{
		
		s.Format( strTreeItem[iType][3] );
		
		hParentItem = InsertItem(s, uIcon , uIcon, m_hRoot);

// Insert userdata
		tTreeLine.eType = VCA_OP_ZONE;
		tTreeLine.usCanEditRuleName = 0;
		tTreeLine.usNumColumns = 1;
		tTreeLine.pItem[0].usFlags = fStatic | fExpand;
		tTreeLine.pItem[0].pData = pRule;
		tTreeLine.pItem[0].pucExpanded = &pRule->ucExpanded;
	//	tTreeLine.pItem[0].pucTicked = &pEvent->ucTicked; 
		InsertUserData( hParentItem, &tTreeLine );
		SetItemTick( hParentItem, -1 );

		if ( !nEnable )
		{
			SetItem( hParentItem, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
	//		SetItemTick( hParentItem, -1 );
			return;
		}

		hChildItem1 =  InsertItem( strTreeItem[VCA5_RULE_TYPE_LINECOUNTER_A-1][1], TV_NOIMAGE, TV_NOIMAGE, hParentItem );
		
		// Insert userdata
		tTreeLine.eType = VCA_OP_ZONE;
		tTreeLine.usCanEditRuleName = 1;
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fStatic | fTickBox | fData;
		tTreeLine.pItem[0].pData = pRuleA;
		tTreeLine.pItem[0].pucExpanded = &pRuleA->ucExpanded;
		tTreeLine.pItem[0].pucTicked = &pRuleA->ucTicked; 
		tTreeLine.pItem[1].usFlags = fEdit;
		tTreeLine.pItem[1].eEditType = edEventName;
		tTreeLine.pItem[1].pData = pRuleA->szRuleName;

		InsertUserData( hChildItem1, &tTreeLine );

		SetItemTick( hChildItem1, pRuleA->ucTicked );
		
		if (pRuleA->ucTicked)
			SetItem( hChildItem1, 1, TVIF_TEXT,  CA2T(pRuleA->szRuleName, CP_UTF8), 0,0,0,0,0);

		hChildItem1 =  InsertItem(strTreeItem[VCA5_RULE_TYPE_LINECOUNTER_B-1][1], TV_NOIMAGE, TV_NOIMAGE, hParentItem );
		SetItemTick( hChildItem1,   pRule->ucTicked);

		
		// Insert userdata
		VCA_TREELIST_LINE_T	tTreeLine;
		tTreeLine.eType = VCA_OP_ZONE;
		tTreeLine.usCanEditRuleName = 1;
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fStatic | fTickBox | fData;
		tTreeLine.pItem[0].pData = pRule;
		tTreeLine.pItem[0].pucExpanded = &pRule->ucExpanded;
		tTreeLine.pItem[0].pucTicked = &pRule->ucTicked; 
		tTreeLine.pItem[1].usFlags = fEdit;
		tTreeLine.pItem[1].eEditType = edEventName;
		tTreeLine.pItem[1].pData = pRule->szRuleName;

		InsertUserData( hChildItem1, &tTreeLine );
		
		if (pRule->ucTicked)
			SetItem( hChildItem1, 1, TVIF_TEXT,  CA2T(pRule->szRuleName, CP_UTF8), 0,0,0,0,0);
		
		hChildItem2 =  InsertItem(strTreeItem[VCA5_RULE_TYPE_LINECOUNTER_B-1][2], TV_NOIMAGE, TV_NOIMAGE, hParentItem );
		hChildItem3 =  InsertItem(strTreeItem[VCA5_RULE_TYPE_LINECOUNTER_A-1][2], TV_NOIMAGE, TV_NOIMAGE, hParentItem );

		int calibWidth = 0;
		int shadowFilterEnabled = 0;
		if (pRule->ucTicked)
		{
			calibWidth			= pRule->tRuleDataEx.tLineCounter.ulCalibrationWidth;
		}
		else if (pRuleA->ucTicked)
		{
			calibWidth			= pRuleA->tRuleDataEx.tLineCounter.ulCalibrationWidth;
		}


		if (pRule->ucTicked || pRuleA->ucTicked)
		{
			s.Format( _T("%3.2f"), 100*(float)calibWidth/65535);
			SetItem( hChildItem2, 1, TVIF_TEXT, s, 0,0,0,0,0);
			SetItemTick( hChildItem2,  pRule->ucWidthCalibrationEnabled || pRuleA->ucWidthCalibrationEnabled);
			
			// Insert userdata
			tTreeLine.eType = VCA_OP_ZONE;
			tTreeLine.usCanEditRuleName = 0;
			tTreeLine.usNumColumns = 2;
			tTreeLine.pItem[0].usFlags = fStatic | fData | fTickBox;
			tTreeLine.pItem[0].pData = pRule;
			tTreeLine.pItem[0].pData2 = pRuleA;
			tTreeLine.pItem[0].pucTicked = 0;//&pRule->ucWidthCalibrationEnabled;
			tTreeLine.pItem[1].usFlags = fEdit;
			tTreeLine.pItem[1].eEditType = edLineCounter;
			tTreeLine.pItem[1].pData = &pRule->tRuleDataEx;
			tTreeLine.pItem[1].pData2 = &pRuleA->tRuleDataEx;

			InsertUserData( hChildItem2, &tTreeLine );

			/// shadow filter
			SetItemTick( hChildItem3,  pRule->ucShadowFilterEnabled || pRuleA->ucShadowFilterEnabled);
			
			// Insert userdata
			tTreeLine.eType = VCA_OP_ZONE;
			tTreeLine.usCanEditRuleName = 0;
			tTreeLine.usNumColumns = 1;
			tTreeLine.pItem[0].usFlags = fStatic | fData | fTickBox;
			tTreeLine.pItem[0].pData = pRule;
			tTreeLine.pItem[0].pData2 = pRuleA;
			tTreeLine.pItem[0].pucTicked = 0;

			InsertUserData( hChildItem3, &tTreeLine );
		}
		else
		{
			pRule->ucWidthCalibrationEnabled = 0;
			pRuleA->ucWidthCalibrationEnabled = 0;
			SetItem( hChildItem2, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );

			pRule->ucShadowFilterEnabled = 0;
			pRuleA->ucShadowFilterEnabled = 0;
			SetItem( hChildItem3, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
		}
		
		SetItemExpand( hParentItem, pRule->ucExpanded );
	}
}


void CZoneTreeCtrl::Add_Stock_SPEED_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule )
{
	CString s;
	int iType = pRule->usRuleType - 1;
	HTREEITEM hParentItem, hChildItem1, hChildItem2;
	UINT uIcon = uIconItemZone[iType];
	int nEnable = m_pDataMgr->CheckFeatureByRuleType(pRule->usRuleType);

	s.Format( strTreeItem[iType][0] );
	hParentItem = InsertItem( s, uIcon, uIcon, m_hRoot );
	SetItemText( hParentItem, _T(" "), 1 );

	// Insert userdata
	VCA_TREELIST_LINE_T	tTreeLine;
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 1;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fTickBox | fExpand | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[0].pucExpanded = &pRule->ucExpanded;
	tTreeLine.pItem[0].pucTicked = &pRule->ucTicked; 
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edEventName;
	tTreeLine.pItem[1].pData = pRule->szRuleName;

	InsertUserData( hParentItem, &tTreeLine );

	if ( !nEnable ){
		SetItem( hParentItem, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
		SetItemTick( hParentItem, -1 );
		return;
	}

	if (pRule->ucTicked)
		SetItem( hParentItem, 1, TVIF_TEXT,  CA2T(pRule->szRuleName, CP_UTF8), 0,0,0,0,0);

	// Set tick box
	SetItemTick( hParentItem, pRule->ucTicked );

	VCA5_RULE_SPEED *pSpeed = &pRule->tRuleDataEx.tSpeed;

	VCA5_CALIB_INFO *pCalibInfo = m_pDataMgr->GetCalibInfo();
	int nSpeedUnit = 0;
	if( VCA5_SPEED_UNITS_MPH == pCalibInfo->ulSpeedUnits ){
		nSpeedUnit = 1;
	}
	
	s.Format( _T("%s%s"), strTreeItem[iType][1], strSpeedUnits[nSpeedUnit] );
	hChildItem1 =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem );
	s.Format( _T("%u"), pSpeed->usSpeedLo );
	SetItem( hChildItem1, 1, TVIF_TEXT, s, 0,0,0,0,0);
	SetItemTick( hChildItem1, -1 );

	// Insert userdata
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 0;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edSpeedLow;
	tTreeLine.pItem[1].pData = pSpeed;
	InsertUserData( hChildItem1, &tTreeLine );


	s.Format( _T("%s%s"), strTreeItem[iType][2], strSpeedUnits[nSpeedUnit] );
	hChildItem2 =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem);
	s.Format( _T("%u"), pSpeed->usSpeedUp ); 
	SetItem( hChildItem2, 1, TVIF_TEXT, s, 0,0,0,0,0);
	SetItemTick( hChildItem2, -1 );

	// Insert userdata
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 0;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edSpeedUp;
	tTreeLine.pItem[1].pData = pSpeed;
	InsertUserData( hChildItem2, &tTreeLine );

	SetItemExpand( hParentItem, pRule->ucExpanded );
}


void CZoneTreeCtrl::Add_Stock_COLSIG_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE* pRule )
{
	CString s;
	int iType = pRule->usRuleType - 1;
	HTREEITEM hParentItem, hChildItem1, hChildItem2;
	UINT uIcon = uIconItemZone[iType];
	int nEnable = m_pDataMgr->CheckFeatureByRuleType(pRule->usRuleType);

	s.Format( strTreeItem[iType][0] );
	hParentItem = InsertItem( s, uIcon, uIcon, m_hRoot );
	SetItemText( hParentItem, _T(" "), 1 );

	// Insert userdata
	VCA_TREELIST_LINE_T	tTreeLine;
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 1;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fTickBox | fExpand | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[0].pucExpanded = &pRule->ucExpanded;
	tTreeLine.pItem[0].pucTicked = &pRule->ucTicked; 
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edEventName;
	tTreeLine.pItem[1].pData = pRule->szRuleName;

	InsertUserData( hParentItem, &tTreeLine );

	if ( !nEnable )
	{
		SetItem( hParentItem, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
		SetItemTick( hParentItem, -1 );
		return;
	}

	if (pRule->ucTicked)
		SetItem( hParentItem, 1, TVIF_TEXT,  CA2T(pRule->szRuleName, CP_UTF8), 0,0,0,0,0);

	// Set tick box
	SetItemTick( hParentItem, pRule->ucTicked );

	unsigned short* usColBin = &pRule->tRuleDataEx.tColorFilter.usColBin;
	unsigned short* usThres = &pRule->tRuleDataEx.tColorFilter.usThreshold;

	// ColBin
	{
		s.Format( _T("%s"), strTreeItem[iType][1] );
		hChildItem1 =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem );

		TCHAR *pszMode = BuildColorBinCombo( pRule );
		SetItemText( hChildItem1, pszMode, 1 );
		SetItem( hChildItem1, 1, TVIF_USECOLORCB, s,0,0,0,0, RGB2TORGB( gPieColours[*usColBin] ) );
		//s.Format( _T("%d"), *usColBin );
		//SetItemText( hChildItem1, s, 1 );
		SetItemTick( hChildItem1, -1 );

		// Insert userdata
		VCA_TREELIST_LINE_T	tTreeLine;
		tTreeLine.eType = VCA_OP_ZONE;
		tTreeLine.usCanEditRuleName = 0;
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fStatic | fData;
		tTreeLine.pItem[0].pData = pRule;
		tTreeLine.pItem[1].usFlags = fCombo;
		tTreeLine.pItem[1].eEditType = cColBin;
		tTreeLine.pItem[1].pData = usColBin;
		InsertUserData( hChildItem1, &tTreeLine );
	}

	// Thres
	{
		s.Format( _T("%s"), strTreeItem[iType][2] );
		hChildItem2 =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem);
		s.Format( _T("%d%%"), (int)((((float)(*usThres))/2.54)+0.5) ); 
		SetItemText( hChildItem2, s, 1 );
		SetItemTick( hChildItem2, -1 );

		// Insert userdata
		VCA_TREELIST_LINE_T	tTreeLine;
		tTreeLine.eType = VCA_OP_ZONE;
		tTreeLine.usCanEditRuleName = 0;
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fStatic | fData;
		tTreeLine.pItem[0].pData = pRule;
		tTreeLine.pItem[1].usFlags = fEdit;
		tTreeLine.pItem[1].eEditType = edThres;
		tTreeLine.pItem[1].pData = usThres;
		InsertUserData( hChildItem2, &tTreeLine );
	}

	//	if ( pEvent->iEventId == m_iCurEvent )
	{
		//		SelectItem( hParentItem, 0);
	}
	SetItemExpand( hParentItem, pRule->ucExpanded );
}


void CZoneTreeCtrl::Add_Stock_DIRECTIONT_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule)
{
	CString s;
	int iType = pRule->usRuleType - 1;
	int direction;
	HTREEITEM hParentItem, hChildItem1, hChildItem2;
	UINT uIcon = uIconItemZone[iType];
	int nEnable = m_pDataMgr->CheckFeatureByRuleType(pRule->usRuleType);

	s.Format( strTreeItem[iType][0] );
	hParentItem = InsertItem( s, uIcon, uIcon, m_hRoot );
	SetItemText( hParentItem, _T(" "), 1 );

	// Insert userdata
	VCA_TREELIST_LINE_T	tTreeLine;
	tTreeLine.eType = VCA_OP_ZONE;

	tTreeLine.usCanEditRuleName = 1;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fTickBox | fExpand | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[0].pucExpanded = &pRule->ucExpanded;
	tTreeLine.pItem[0].pucTicked = &pRule->ucTicked; 
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edEventName;
	tTreeLine.pItem[1].pData = pRule->szRuleName;

	InsertUserData( hParentItem, &tTreeLine );

	if ( !nEnable ){
		SetItem( hParentItem, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
		SetItemTick( hParentItem, -1 );
		return;
	}

	if (pRule->ucTicked)
		SetItem( hParentItem, 1, TVIF_TEXT,  CA2T(pRule->szRuleName, CP_UTF8), 0,0,0,0,0);

	SetItemTick( hParentItem, pRule->ucTicked);


	VCA5_RULE_DIRECTION *pDirection = &pRule->tRuleDataEx.tDirection;

	// direction
	s.Format( strTreeItem[iType][1] );
	hChildItem1 =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem );
	SetItemTick( hChildItem1, -1);

	direction = (pDirection->sStartAngle+pDirection->sFinishAngle) /2 / 10;
	direction = 90 - direction;
	ValidateSingleAngle( &direction );

	s.Format( _T("%d°"), direction );
	SetItem( hChildItem1, 1, TVIF_TEXT, s, 0,0,0,0,0);
	SetItemTick( hChildItem1, -1 );

	// Insert userdata
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 0;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edDirection;
	tTreeLine.pItem[1].pData = pDirection;
	InsertUserData( hChildItem1, &tTreeLine );


	// acceptance angle
	s.Format( strTreeItem[iType][2] );
	hChildItem2 =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem);
	s.Format( _T("%d°"), (pDirection->sFinishAngle-pDirection->sStartAngle)  / 10 ); 
	SetItemText( hChildItem2, s, 1 );
	SetItemTick( hChildItem2, -1);
	
	// Insert userdata
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 0;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edAcceptance;
	tTreeLine.pItem[1].pData = pDirection;
	InsertUserData( hChildItem2, &tTreeLine );

	SetItemExpand( hParentItem, pRule->ucExpanded );
}


int CZoneTreeCtrl::ValidTailgatingRule( VCA5_APP_RULE *pRule, unsigned short usZoneId )
{
	VCA5_APP_RULE	*pRuleA = m_pDataMgr->GetRule( usZoneId, VCA5_RULE_TYPE_LINECOUNTER_A );
	VCA5_APP_RULE	*pRuleB = m_pDataMgr->GetRule( usZoneId, VCA5_RULE_TYPE_LINECOUNTER_B );
	if ( ( pRuleA && pRuleA->ucTicked ) || ( pRuleB && pRuleB->ucTicked) )
		return TRUE;
	else
	{
		pRule->tRuleDataEx.tTimer.usFlags = VCA5_RULE_TAILGATING_BYTRACKER;
		return FALSE;
	}
}


void CZoneTreeCtrl::Add_Stock_TIMERT_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule )
{
	CString s;
	int iType = pRule->usRuleType - 1;

	//Line does not support Stop Dwell Abadon filter only support Tailgating as TIMER related filter
	if(pZone->usZoneStyle == VCA5_ZONE_STYLE_TRIPWIRE && pRule->usRuleType != VCA5_RULE_TYPE_TAILGATING){
		return;
	}

	HTREEITEM hParentItem, hChildItem1;
	UINT uIcon = uIconItemZone[iType];
	int nEnable = m_pDataMgr->CheckFeatureByRuleType(pRule->usRuleType);

	s.Format( strTreeItem[iType][0] );
	hParentItem = InsertItem( s, uIcon, uIcon, m_hRoot);
	SetItemText( hParentItem, _T(" "), 1 );

	// Insert userdata
	VCA_TREELIST_LINE_T	tTreeLine;
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usCanEditRuleName = 1;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic | fTickBox | fExpand | fData;
	tTreeLine.pItem[0].pData = pRule;
	tTreeLine.pItem[0].pucExpanded = &pRule->ucExpanded;
	tTreeLine.pItem[0].pucTicked = &pRule->ucTicked; 
	tTreeLine.pItem[1].usFlags = fEdit;
	tTreeLine.pItem[1].eEditType = edEventName;
	tTreeLine.pItem[1].pData = pRule->szRuleName;

	InsertUserData( hParentItem, &tTreeLine );

	if ( !nEnable ){
		SetItem( hParentItem, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
		SetItemTick( hParentItem, -1 );
		return;
	}

	if (pRule->ucTicked)
		SetItem( hParentItem, 1, TVIF_TEXT,  CA2T(pRule->szRuleName, CP_UTF8), 0,0,0,0,0);

	// Set tick box
	SetItemTick( hParentItem, pRule->ucTicked );


	// combo box for tailgating (detection line)
	if ( ( pZone->usZoneStyle == VCA5_ZONE_STYLE_TRIPWIRE ) && ( pRule->usRuleType == VCA5_RULE_TYPE_TAILGATING) )
	{
		int iFCSelected = ValidTailgatingRule( pRule, pZone->usZoneId );
		s.Format( strTreeItem[iType][2] );
		HTREEITEM hItem = InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem );

		TCHAR *pszMode = BuildTailgatingModeCombo( pRule );
		SetItemText( hItem, pszMode, 1 );
		// No tick box
		SetItemTick( hItem, -1 );

		// Insert userdata
		VCA_TREELIST_LINE_T	tTreeLine;
		tTreeLine.eType = VCA_OP_ZONE;
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fStatic | fData;
		tTreeLine.pItem[0].pData = pRule;
		tTreeLine.pItem[1].usFlags = iFCSelected ? fCombo : fStatic;
		tTreeLine.pItem[1].eEditType = cTailgatingMode;
		tTreeLine.pItem[1].pData = &(pRule->tRuleDataEx.tTimer.usFlags);
		InsertUserData( hItem, &tTreeLine );

		if ( !iFCSelected )
		{
//			SetItem( hItem, 0, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
//			SetItem( hItem, 1, TVIF_STATE, NULL, 0, 0, TVIS_DISABLED, TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK|TVIS_DISABLED );
		}
	}

	// time threshold
	{
		int	iTime = pRule->tRuleDataEx.tTimer.ulTimeThreshold;

		s.Format( strTreeItem[iType][1] );
		hChildItem1 =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentItem);
		s.Format( _T("%d"), iTime );
		SetItem( hChildItem1, 1, TVIF_TEXT, s, 0,0,0,0,0);
		SetItemTick( hChildItem1, -1 );

		// Insert userdata
		VCA_TREELIST_LINE_T	tTreeLine;
		tTreeLine.eType = VCA_OP_ZONE;
		tTreeLine.usCanEditRuleName = 0;
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fStatic | fData;
		tTreeLine.pItem[0].pData = pRule;
		tTreeLine.pItem[1].usFlags = fEdit;
		tTreeLine.pItem[1].eEditType = edTimeThreshold;
		tTreeLine.pItem[1].pData = &pRule->tRuleDataEx;
		InsertUserData( hChildItem1, &tTreeLine );
	}

	
	SetItemExpand( hParentItem, pRule->ucExpanded );
}


void CZoneTreeCtrl::AddRuleToZone( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule)
{
	//if ( pRule->eType == VCA_RULETYPE_PREDEFINED )
	//{
		// Do something with stock events, stock event only contains one rule
		switch ( pRule->usRuleType )
		{
		case VCA5_RULE_TYPE_PRESENCE:
		case VCA5_RULE_TYPE_ENTER:
		case VCA5_RULE_TYPE_EXIT:
		case VCA5_RULE_TYPE_APPEAR:
		case VCA5_RULE_TYPE_DISAPPEAR:
			Add_Stock_BASICT_Tree( pZone, pRule );
			break;

		case VCA5_RULE_TYPE_DIRECTION:
			Add_Stock_DIRECTIONT_Tree( pZone, pRule );
			break;

		case VCA5_RULE_TYPE_STOP:
		case VCA5_RULE_TYPE_DWELL:
//		case VCA5_RULE_TYPE_ABOBJ:	
		case VCA5_RULE_TYPE_RMOBJ:	
		case VCA5_RULE_TYPE_TAILGATING:
		case VCA5_RULE_TYPE_SMOKE:
		case VCA5_RULE_TYPE_FIRE:
			Add_Stock_TIMERT_Tree( pZone, pRule );				
			break;

		case VCA5_RULE_TYPE_SPEED:
			if (m_pDataMgr->CheckFeature(VCA5_FEATURE_CALIBRATION)){			
				Add_Stock_SPEED_Tree( pZone, pRule );
			}
			break;

		case VCA5_RULE_TYPE_LINECOUNTER_A:
		case VCA5_RULE_TYPE_LINECOUNTER_B:
			Add_Stock_LINECOUNTER_Tree( pZone, pRule );
			break;
		case VCA5_RULE_TYPE_COLSIG:
			Add_Stock_COLSIG_Tree( pZone, pRule );
			break;
		}
	//}
}

void CZoneTreeCtrl::RedrawZoneTree( VCA5_APP_ZONE *pZone )
{
	CString s, s1;
	USES_CONVERSION;
	
	DeleteAllItems();

	if ( pZone )
	{
		m_iTreeType = TREELIST_ZONE;

		s.Format(_T("Zone %d"), pZone->usZoneId );
		m_hRoot = InsertItem( s, 0, 0, (HTREEITEM)0, (HTREEITEM)0 );

		// No tick box
		SetItemTick( m_hRoot, -1 );
		
		// Insert userdata
		VCA_TREELIST_LINE_T	tTreeLine;
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fStatic | fData;
		tTreeLine.pItem[0].pData = pZone;
		tTreeLine.pItem[1].usFlags = fStatic;
		tTreeLine.pItem[1].pData = NULL;
		InsertUserData( m_hRoot, &tTreeLine );

		{
			s.Format(_T("Name"));
			HTREEITEM hItem = InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, m_hRoot );
			SetItem( hItem, 1, TVIF_TEXT, CA2T(pZone->szZoneName, CP_UTF8), 0,0,0,0,0);

			// No tick box
			SetItemTick( hItem, -1 );

			// Insert userdata
			VCA_TREELIST_LINE_T	tTreeLine;
			tTreeLine.usNumColumns = 2;
			tTreeLine.pItem[0].usFlags = fStatic ;
			tTreeLine.pItem[0].pData = pZone;
			tTreeLine.pItem[1].usFlags = fEdit;
			tTreeLine.pItem[1].eEditType = edZoneName;
			tTreeLine.pItem[1].pData = pZone->szZoneName;
			InsertUserData( hItem, &tTreeLine );
		}
		{
			s.Format(_T("Color"));
			HTREEITEM hItem = InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, m_hRoot );
			LPCTSTR pszZoneColor = GetColorName( pZone->uiColor );
			SetItem( hItem, 1, TVIF_TEXT, pszZoneColor, 0,0,0,0,0);
			SetItem( hItem, 1, TVIF_USECOLORCB, s,0,0,0,0, pZone->uiColor);

			// No tick box
			SetItemTick( hItem, -1 );

			// Insert userdata
			VCA_TREELIST_LINE_T	tTreeLine;
			tTreeLine.usNumColumns = 2;
			tTreeLine.pItem[0].usFlags = fStatic ;
			tTreeLine.pItem[0].pData = pZone;
			tTreeLine.pItem[1].usFlags = fCombo;
			tTreeLine.pItem[1].eEditType = cZoneColor;
			tTreeLine.pItem[1].pData = &pZone->uiColor;
			InsertUserData( hItem, &tTreeLine );
		}

		if (pZone->usZoneStyle != VCA5_ZONE_STYLE_TRIPWIRE) {
			s.Format(_T("Detect/non-detect"));
			HTREEITEM hItem = InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, m_hRoot );
			TCHAR *pszZoneType = BuildZoneTypeCombo( pZone );
			SetItem( hItem, 1, TVIF_TEXT, pszZoneType, 0,0,0,0,0);

			// No tick box
			SetItemTick( hItem, -1 );

			// Insert userdata
			VCA_TREELIST_LINE_T	tTreeLine;
			tTreeLine.usNumColumns = 2;
			tTreeLine.pItem[0].usFlags = fStatic ;
			tTreeLine.pItem[0].pData = pZone;
			tTreeLine.pItem[1].usFlags = fCombo;
			tTreeLine.pItem[1].eEditType = cZoneType;
			tTreeLine.pItem[1].pData = &pZone->usZoneType;
			InsertUserData( hItem, &tTreeLine );
		}
		
		// Select root and expand root
		SelectItem( m_hRoot, 0);
		SetItemExpand( m_hRoot, 1 );
		if (pZone->usZoneType == VCA5_ZONE_TYPE_DETECTION) {
			AddPredefinedZoneRules( pZone );
		}

		//SortChildren( m_hRoot );
	}else{
		RedrawAllZoneListTree( );
	}
}

/************************************** Main redraw fucnction ************************************/
void CZoneTreeCtrl::RedrawAllZoneListTree( )
{
	CString		s;
	int			i;
	UINT		uIcon = 0;
	VCA5_APP_ZONE	*pZone;
	HTREEITEM	hParent;
	HTREEITEM	hItem;
	VCA_TREELIST_LINE_T	tTreeLine;
	USES_CONVERSION;

	DeleteAllItems();

	m_iTreeType = TREELIST_SHOWALL;
	s.Format(_T("All zones and counters" ));
	m_hRoot = InsertItem( s, uIcon, uIcon, TVI_ROOT );
	SetItemText( m_hRoot, _T(" "), 1 );
	SetItemTick( m_hRoot, -1 );
	SetItemExpand( m_hRoot, 1 );
	//Insert the user data, 2 column
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic;
	tTreeLine.pItem[0].pData = NULL;
	tTreeLine.pItem[1].usFlags = fStatic;
	tTreeLine.pItem[1].pData = NULL;
	InsertUserData( m_hRoot, &tTreeLine );


// Add all zones/tripwires to the list
	uIcon = 0;
	s.Format(_T("Zones" ));
	hParent = InsertItem( s, uIcon, uIcon, m_hRoot );
	SetItemText( hParent, _T(" "), 1 );
	SetItemTick( hParent, -1 );
	SetItemExpand( hParent, 1 );
	//Insert the user data, 2 column
	tTreeLine.eType = VCA_OP_ZONE;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic;
	tTreeLine.pItem[0].pData = NULL;
	tTreeLine.pItem[1].usFlags = fStatic;
	tTreeLine.pItem[1].pData = NULL;
	InsertUserData( hParent, &tTreeLine );

	VCA5_APP_ZONES *pZones = m_pDataMgr->GetZones();
	for ( i = 0 ; i < (int) pZones->ulTotalZones; i++ )
	{
		pZone = &pZones->pZones[i];
		if (VCA5_APP_AREA_STATUS_NOT_USED & pZone->uiStatus) 	continue;

		switch( pZone->usZoneStyle )
		{
		case VCA5_ZONE_STYLE_TRIPWIRE:
		case VCA5_ZONE_STYLE_POLYGON:
			{
				uIcon = 0;
				s.Format(_T("Zone %d"), pZone->usZoneId);
				hItem = InsertItem( s, uIcon, uIcon, hParent );
				SetItemTick( hItem, -1 );
				SetItem( hItem, 1, TVIF_TEXT, CA2T(pZone->szZoneName, CP_UTF8), 0,0,0,0,0);
				//Insert the user data, 2 column
				tTreeLine.usNumColumns = 2;
				tTreeLine.pItem[0].usFlags = fTickBox;
				tTreeLine.pItem[0].pData = pZone;
				tTreeLine.pItem[1].usFlags = fStatic;
				tTreeLine.pItem[1].pData = NULL;
				InsertUserData( hItem, &tTreeLine );
			}
			break;
		case VCA5_ZONE_STYLE_NOTDEFINED:
			{
			}
			break;
		}
	}

// Add all counters to the list
	uIcon = uIconItemCounter[0];
	s.Format(_T("Counters" ));
	hParent = InsertItem( s, uIcon, uIcon, m_hRoot );
	SetItemText( hParent, _T(" "), 1 );
	SetItemTick( hParent, -1 );
	SetItemExpand( hParent, 1 );

	//Insert the user data, 2 column
	tTreeLine.eType = VCA_OP_COUNTER;
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fStatic;
	tTreeLine.pItem[0].pData = NULL;
	tTreeLine.pItem[1].usFlags = fStatic;
	tTreeLine.pItem[1].pData = NULL;
	InsertUserData( hParent, &tTreeLine );

	VCA5_APP_COUNTERS *pCounters = m_pDataMgr->GetCounters();
	VCA5_APP_COUNTER *pCounter;
	for ( i = (int)pCounters->ulTotalCounters - 1; i >= 0; i-- )
	{
		pCounter = &pCounters->pCounters[i];
		if (VCA5_APP_AREA_STATUS_NOT_USED & pCounter->uiStatus) continue;

		uIcon = uIconItemCounter[0];
		VCA5_APP_COUNTER *pCounter = m_pDataMgr->GetCounter( i );
		s.Format(_T("Counter %d"), pCounter->usCounterId );
		hItem = InsertItem( s, uIcon, uIcon, hParent );
		SetItemTick( hItem, -1 );
		SetItem( hItem, 1, TVIF_TEXT, CA2T(pCounter->szCounterName, CP_UTF8),0,0,0,0,0);
		//Insert the user data, 2 column
		tTreeLine.usNumColumns = 2;
		tTreeLine.pItem[0].usFlags = fTickBox;
		tTreeLine.pItem[0].pData = pCounter;
		tTreeLine.pItem[1].usFlags = fStatic;
		tTreeLine.pItem[1].pData = NULL;
		InsertUserData( hItem, &tTreeLine );
	
	}
}

void CZoneTreeCtrl::InsertUserData( HTREEITEM hItem, VCA_TREELIST_LINE_T *pTreeLine )
{
	void *pData = GetUserData( hItem );
	if(pData)memcpy( pData, (void *) pTreeLine, sizeof( VCA_TREELIST_LINE_T ) );
}

void CZoneTreeCtrl::RedrawCounterTree( VCA5_APP_COUNTER *pCounter )
{
	CString s, s1;
	USES_CONVERSION;
	
	DeleteAllItems();

	if (pCounter)
	{
		m_iTreeType = TREELIST_COUNTER;
		
		if ( pCounter )
		{
			HTREEITEM hCurrentCounter;
			s.Format(_T("Counter %d"), pCounter->usCounterId);
			UINT uIcon = uIconItemCounter[0];
			hCurrentCounter = InsertItem( s, uIcon, uIcon, (HTREEITEM) 0, (HTREEITEM) 0 );
			
			s.Format( _T("%d"),pCounter->iCounterResult );
			SetItem( hCurrentCounter, 1, TVIF_TEXT, s, 0,0,0,0,0);

			// No tick box
			SetItemTick( hCurrentCounter, -1 );

			SelectItem( hCurrentCounter, 0);
			SetItemExpand( hCurrentCounter, 1 );
			m_hRoot = hCurrentCounter;
			
			//Insert the user data
			VCA_TREELIST_LINE_T	tTreeLine;
			tTreeLine.usNumColumns = 2;
			tTreeLine.eType = VCA_OP_COUNTER;
			tTreeLine.pItem[0].usFlags = fStatic;
			tTreeLine.pItem[0].pData = pCounter;
			tTreeLine.pItem[1].usFlags = fStatic;
			tTreeLine.pItem[1].pData = &(pCounter->iCounterResult);
			InsertUserData( hCurrentCounter, &tTreeLine );

			{
				s.Format(_T("Name"));
				HTREEITEM hItem = InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, m_hRoot );
				SetItem( hItem, 1, TVIF_TEXT, CA2T(pCounter->szCounterName, CP_UTF8 ), 0,0,0,0,0);

				// Set tick box
				SetItemTick( hItem, -1 );

				VCA_TREELIST_LINE_T	tTreeLine;
				tTreeLine.eType = VCA_OP_COUNTER;
				tTreeLine.usNumColumns = 2;
				tTreeLine.pItem[0].usFlags = fStatic;
				tTreeLine.pItem[0].pData = pCounter;
				tTreeLine.pItem[1].usFlags = fEdit;
				tTreeLine.pItem[1].eEditType = edCounterName;
				tTreeLine.pItem[1].pData = pCounter->szCounterName;
				InsertUserData( hItem, &tTreeLine );
			}

			{
				s.Format(_T("Color"));
				HTREEITEM hItem = InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, m_hRoot );
				LPCTSTR pszColor = GetColorName( pCounter->uiColor );
				SetItemText( hItem, pszColor, 1 );
				SetItem( hItem, 1, TVIF_USECOLORCB, s,0,0,0,0, pCounter->uiColor);

				// No tick box
				SetItemTick( hItem, -1 );

				// Insert userdata
				VCA_TREELIST_LINE_T	tTreeLine;
				tTreeLine.eType = VCA_OP_COUNTER;
				tTreeLine.usNumColumns = 2;
				tTreeLine.pItem[0].usFlags = fStatic ;
				tTreeLine.pItem[0].pData = pCounter;
				tTreeLine.pItem[1].usFlags = fCombo;
				tTreeLine.pItem[1].eEditType = cCounterColor;
				tTreeLine.pItem[1].pData = &pCounter->uiColor;
				InsertUserData( hItem, &tTreeLine );
			}
			AddCounterTree( pCounter, hCurrentCounter );
		}
	}else{
		RedrawAllZoneListTree();
	}
}


void CZoneTreeCtrl::BuildCounterComboBoxByRule( VCA5_APP_COUNTER *pCounter )
{
	int			iNumItems = 0;
	CString		s;
	VCA5_APP_ZONE	*pZone;
	VCA5_APP_ZONES	*pZones;
	VCA5_APP_RULE	*pRule;
	int iZoneIdx;
	USES_CONVERSION;

	pZones = m_pDataMgr->GetZones();
	for (ULONG i = 0; i < pZones->ulTotalZones; i++){
		pZone = &pZones->pZones[i];
		if ( (pZone->uiStatus&VCA5_APP_AREA_STATUS_NOT_USED) == 0 && (pZone->usZoneStyle != VCA5_ZONE_STYLE_NOTDEFINED) ){

			iZoneIdx = m_pDataMgr->GetZoneIdx(pZone);
			for (int j = 0; j < VCA5_RULE_TYPE_NUM; j++) {
				pRule = m_pDataMgr->GetRule(iZoneIdx, (VCA5_RULE_TYPE_E) j);
				if(pRule && pRule->ucTicked){
					if(!m_pDataMgr->CheckFeatureByRuleType(pRule->usRuleType)) continue;
				
					TREELIST_COMBO_ITEM_T *pItem = &m_tRuleCounterCombo.pComboItem[iNumItems];
					pItem->usType = VCA5_TRIG_RULE;
					pItem->usTrigId = (unsigned short) pRule->ulRuleId;
					s.Format( _T("%s"), CA2T(pRule->szRuleName, CP_UTF8 ));
					int nLength =  s.GetLength();
					memcpy( pItem->pszComboString, s.GetBuffer(0), ( nLength+1 ) * 2 );
					pItem->crColour = pZone->uiColor;
					iNumItems++;
				}
			}
		}
	}
	m_tRuleCounterCombo.uiNumComboItems = iNumItems;
}


TCHAR* CZoneTreeCtrl::BuildColorBinCombo( VCA5_APP_RULE *pRule )
{
	int			iNumItems = 0, iIndex = 0;
	TREELIST_COMBO_ITEM_T *pItem;

	unsigned short usColBin = pRule->tRuleDataEx.tColorFilter.usColBin;

	for ( int i = 0; i < VCA5_APP_PALETTE_SIZE; i++ )
	{
		pItem = &m_tColorBinCombo.pComboItem[i];
		pItem->uiColor = gPieColours[i];
		pItem->crColour = RGB2TORGB( gPieColours[i] );
		pItem->usColBin = i;
		USES_CONVERSION;
		wsprintf( pItem->pszComboString, _T("%s"), A2T(gPieColourNames[i]) );
		if ( pItem->usColBin == usColBin )
			iIndex = i;
		iNumItems++;
	}
	m_tColorBinCombo.uiNumComboItems = iNumItems;
	return m_tColorBinCombo.pComboItem[iIndex].pszComboString;
}


void CZoneTreeCtrl::BuildZoneColorCombo()
{
	int			iNumItems = 0;
	int			iIndex = 0;
	TREELIST_COMBO_ITEM_T *pItem;

	for ( int i = 0; i < MAXZONECOLORS; i++ )
	{
		pItem = &m_tZoneColorCombo.pComboItem[i];
		//pItem->crColour = RGB2BGR(m_pDataMgr->GetColor(i));
		pItem->crColour = m_pDataMgr->GetColor(i);
		_stprintf_s(pItem->pszComboString, 80,_T("%s"), m_pDataMgr->GetColorName(i));
		iNumItems++;
	}
	m_tZoneColorCombo.uiNumComboItems = iNumItems;
}

TCHAR* CZoneTreeCtrl::BuildTailgatingModeCombo( VCA5_APP_RULE *pRule )
{
	int			iNumItems = 0;
	TREELIST_COMBO_ITEM_T *pItem;

	pItem = &m_tTailGateModeCombo.pComboItem[iNumItems];
	pItem->usMode = VCA5_RULE_TAILGATING_BYTRACKER;
	pItem->crColour = 0xFFFFFFFF;
	wsprintf( pItem->pszComboString, _T("Tracker") );
	iNumItems++;

	pItem = &m_tTailGateModeCombo.pComboItem[iNumItems];
	pItem->usMode = VCA5_RULE_TAILGATING_BYLINECOUNTER;
	pItem->crColour = 0xFFFFFFFF;
	wsprintf( pItem->pszComboString, _T("Counting Line") );
	iNumItems++;

	m_tTailGateModeCombo.uiNumComboItems = iNumItems;

	if ( pRule->tRuleDataEx.tTimer.usFlags == VCA5_RULE_TAILGATING_BYLINECOUNTER )
		return m_tTailGateModeCombo.pComboItem[1].pszComboString;
	else
		return m_tTailGateModeCombo.pComboItem[0].pszComboString;
}


TCHAR* CZoneTreeCtrl::BuildZoneTypeCombo( VCA5_APP_ZONE *pZone )
{
	int			iNumItems = 0;
	TREELIST_COMBO_ITEM_T *pItem;

	pItem = &m_tZoneTypeCombo.pComboItem[iNumItems];
	pItem->usZoneType = VCA5_ZONE_TYPE_DETECTION;
	pItem->crColour = 0xFFFFFFFF;
	wsprintf( pItem->pszComboString, _T("Detection") );
	iNumItems++;

	pItem = &m_tZoneTypeCombo.pComboItem[iNumItems];
	pItem->usZoneType = VCA5_ZONE_TYPE_NONDETECTION;
	pItem->crColour = 0xFFFFFFFF;
	wsprintf( pItem->pszComboString, _T("Non-Detection") );
	iNumItems++;

	m_tZoneTypeCombo.uiNumComboItems = iNumItems;

	if ( pZone->usZoneType == VCA5_ZONE_TYPE_DETECTION )
		return m_tZoneTypeCombo.pComboItem[0].pszComboString;
	else
		return m_tZoneTypeCombo.pComboItem[1].pszComboString;

}

void CZoneTreeCtrl::BuildObjclsIdCombo( )
{
	TREELIST_COMBO_ITEM_T *pItem;
	int	iNumItems = 0;
	int	i;
	USES_CONVERSION;

	for ( i = 0; i < VCA5_MAX_NUM_CLSOBJECTS; i++ )
	{
		VCA5_APP_CLSOBJECT *pClsobject = m_pDataMgr->GetClsObject( i );
		if (!pClsobject || !pClsobject->bEnable)
			continue;

		pItem = &m_tObjClsIdCombo.pComboItem[iNumItems];
		pItem->sObjClsId = pClsobject->sClsObjectId;
		wsprintf( pItem->pszComboString, _T("%s"), CA2T(pClsobject->szClsobjectName, CP_UTF8));
		pItem->crColour = 0xFFFFFFFF;
		iNumItems++;
	}

	pItem = &m_tObjClsIdCombo.pComboItem[iNumItems];
	pItem->sObjClsId = VCA5_APP_CLSOBJECT_UNCLASSIFIED_ID;
	wsprintf( pItem->pszComboString, _T("%s"), _T("Unclassified"));
	pItem->crColour = 0xFFFFFFFF;
	iNumItems++;	

	m_tObjClsIdCombo.uiNumComboItems = iNumItems;
}

void CZoneTreeCtrl::BuildObjclsLogicCombo( )
{
	int			iNumItems = 0;
	TREELIST_COMBO_ITEM_T *pItem;

	pItem = &m_tObjClsLogicCombo.pComboItem[iNumItems];
	pItem->ucObjClsLogic = TRKOBJ_INCLUDE;
	pItem->crColour = 0xFFFFFFFF;
	wsprintf( pItem->pszComboString, _T("Include") );
	iNumItems++;

	pItem = &m_tObjClsLogicCombo.pComboItem[iNumItems];
	pItem->ucObjClsLogic = TRKOBJ_EXCLUDE;
	pItem->crColour = 0xFFFFFFFF;
	wsprintf( pItem->pszComboString, _T("Exclude") );
	iNumItems++;

	m_tObjClsLogicCombo.uiNumComboItems = iNumItems;
}

void CZoneTreeCtrl::ActiveCombo( HTREEITEM hItem, TREELIST_COMBO_T *pComboTree )
{
	CComboBox	*pCombo;
	unsigned int i;
	CString s1, s2;
	TCHAR	cText[256];

	pCombo = EditLabelCb(hItem,1,1,1);
	if ( !pCombo ) 
		return;

	GetItemText( hItem, cText, 256, 1 );
	s1.Format( _T("%s"), cText );

	for ( i = 0; i < pComboTree->uiNumComboItems; i ++ )
	{
		pCombo->AddString( pComboTree->pComboItem[i].pszComboString);
		DWORD_PTR pItemData = (DWORD_PTR) &(pComboTree->pComboItem[i].crColour);
		pCombo->SetItemData( i, pItemData );

		s2.Format( _T("%s"), pComboTree->pComboItem[i].pszComboString );
		if ( s1 == s2 )
			pCombo->SetCurSel( i );
	}
}

LPCTSTR	CZoneTreeCtrl::GetColorName(DWORD uiColor)
{
	for ( int i = 0; i < MAXZONECOLORS; i++ )
	{
		if (m_pDataMgr->GetColor(i) == uiColor) {
			return m_pDataMgr->GetColorName(i);
		}
	}
	ASSERT(0);
	return NULL;
}

/*	ucTick == -1, no tick box
	ucTick == 0, tick box, not ticked
	ucTick == 1, tick box, ticked
*/

void CZoneTreeCtrl::AddCounterIncrement( VCA5_APP_COUNTER *pCounter, HTREEITEM hParent, BOOL bEnable )
{
	CString	s;
	unsigned int i;
	int	iNum = 0;
	int iNumUsededSubCounter = 0;
	HTREEITEM hItem, hParentInc;
	UINT uIcon = uIconItemCounter[0];
	VCA_TREELIST_LINE_T	tTreeLine;
	VCA5_APP_ZONE *pTrigZone;
	VCA5_APP_RULE *pTrigRule;
	unsigned char uNotSelectState = bEnable ? STATEIMAGE_CHECKBOX_UNCHECKED : STATEIMAGE_CHECKBOX_DISABLE;
	USES_CONVERSION;

	s.Format( _T("Increment") );
	hParentInc = InsertItem( s, uIcon, uIcon, hParent );
	// No tick box
	SetItemTick( hParentInc, -1 );

	//Insert the user data
	tTreeLine.eType = VCA_OP_COUNTER;
	tTreeLine.usNumColumns = 1;
	tTreeLine.pItem[0].usFlags = fStatic;
	tTreeLine.pItem[0].pData = &usINC;
	InsertUserData( hParentInc, &tTreeLine );
	
	for ( i = 0; i < pCounter->usNumSubCounters; i ++ )
	{
		unsigned short usCounterType = pCounter->pSubCounters[i].usSubCounterType;

		if ( (usCounterType&VCA5_COUNTER_INCREMENT) && (pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_TICKED))
		{
			pTrigRule = m_pDataMgr->GetRuleByInternalId(pCounter->pSubCounters[i].usTrigId);
			if(!pTrigRule) continue;
			if(!m_pDataMgr->CheckFeatureByRuleType(pTrigRule->usRuleType)) continue;

			s.Format( _T("Inc %d"), iNum );
			hItem =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentInc );
			s.Empty();
					
			if (pTrigRule && (pTrigRule->ucTicked)) {
				pTrigZone = m_pDataMgr->GetZoneById(pTrigRule->usZoneId);
				if (pTrigZone) {
					s.Format( _T("%s"), CA2T(pTrigRule->szRuleName, CP_UTF8 ));
					SetItem( hItem, 1, TVIF_USECOLORCB, s,0,0,0,0, pTrigZone->uiColor);	
				} else {
					pCounter->uiSubCounterStatuses[i] = 0;
					s.Format( _T("Error, invalid zone") );
				}
			} else {
				pCounter->uiSubCounterStatuses[i] = 0;
				s.Format( _T("Error, invalid rule") );
			}

			SetItemTick( hItem, (pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_TICKED) ? 1 : uNotSelectState);
			SetItem( hItem, 1, TVIF_TEXT, s, 0,0,0,0,0);

			//Insert the user data, 2 column
			tTreeLine.usNumColumns = 2;
			tTreeLine.pItem[0].usFlags = fTickBox;
			tTreeLine.pItem[0].pData = (void *) i;
			tTreeLine.pItem[1].usFlags = bEnable?fCombo:fStatic;
			tTreeLine.pItem[1].pData = pCounter;
			tTreeLine.pItem[1].eEditType = cCounterInc;
			InsertUserData( hItem, &tTreeLine );

			iNum++;
		}
		if (pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_TICKED) iNumUsededSubCounter++;
	}

	if (iNumUsededSubCounter >= VCA5_MAX_NUM_SUBCOUNTERS) return;
	
	// Append a new one after all counters
	s.Format( _T("Inc %d"), iNum );
	hItem =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentInc );
	SetItem( hItem, 1, TVIF_TEXT, _T(""), 0,0,0,0,0);

	memset( &pCounter->pSubCounters[pCounter->usNumSubCounters], 0, sizeof( VCA5_SUBCOUNTER ) );
	pCounter->uiSubCounterStatuses[pCounter->usNumSubCounters] = 0;
	SetItemTick( hItem, uNotSelectState);

	//Insert the user data, 2 column
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fTickBox;
	tTreeLine.pItem[0].pData = (void *) pCounter->usNumSubCounters;
	tTreeLine.pItem[1].usFlags = bEnable?fCombo:fStatic;
	tTreeLine.pItem[1].eEditType = cCounterInc;
	tTreeLine.pItem[1].pData = pCounter;
	InsertUserData( hItem, &tTreeLine );

	SetItemExpand( hParentInc, 1 );
}

void CZoneTreeCtrl::AddCounterDecrement( VCA5_APP_COUNTER *pCounter, HTREEITEM hParent, BOOL bEnable )
{
	CString	s;
	unsigned int i  = 0;
	int	iNum = 0, iNumUsedSubCounter = 0;
	HTREEITEM hItem, hParentInc;
	UINT uIcon = uIconItemCounter[0];
	VCA_TREELIST_LINE_T	tTreeLine;
	VCA5_APP_ZONE *pTrigZone;
	VCA5_APP_RULE *pTrigRule;
	unsigned char uNotSelectState = bEnable ? STATEIMAGE_CHECKBOX_UNCHECKED : STATEIMAGE_CHECKBOX_DISABLE;
	USES_CONVERSION;

	s.Format( _T("Decrement") );
	hParentInc = InsertItem( s, uIcon, uIcon, hParent );
	// No tick box
	SetItemTick( hParentInc, -1 );

	//Insert the user data
	tTreeLine.eType = VCA_OP_COUNTER;
	tTreeLine.usNumColumns = 1;
	tTreeLine.pItem[0].usFlags = fStatic;
	tTreeLine.pItem[0].pData = &usDEC;
	InsertUserData( hParentInc, &tTreeLine );

	for ( i = 0; i < pCounter->usNumSubCounters; i ++ )
	{
		unsigned short usCounterType = pCounter->pSubCounters[i].usSubCounterType;

		if ((usCounterType & VCA5_COUNTER_DECREMENT)&&(pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_TICKED))
		{
			pTrigRule = m_pDataMgr->GetRuleByInternalId(pCounter->pSubCounters[i].usTrigId);
			if(!pTrigRule) continue;
			if(!m_pDataMgr->CheckFeatureByRuleType(pTrigRule->usRuleType)) continue;

			s.Format( _T("Dec %d"), iNum );
			hItem =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentInc );

			s.Empty();
			if (pTrigRule && pTrigRule->ucTicked) {
				pTrigZone = m_pDataMgr->GetZoneById(pTrigRule->usZoneId);
				if (pTrigZone) {
					s.Format( _T("%s"), CA2T(pTrigRule->szRuleName, CP_UTF8 ));
					SetItem( hItem, 1, TVIF_USECOLORCB, s,0,0,0,0, pTrigZone->uiColor);
				} else {
					pCounter->uiSubCounterStatuses[i] = 0;
					s.Format( _T("Error, invalid zone") );
				}
			} else {
				pCounter->uiSubCounterStatuses[i] = 0;
				s.Format( _T("Error, invalid rule") );
			}

			SetItemTick( hItem, (pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_TICKED) ? 1 : uNotSelectState);
			SetItem( hItem, 1, TVIF_TEXT, s, 0,0,0,0,0);
		
			//Insert the user data, 2 column
			tTreeLine.usNumColumns = 2;
			tTreeLine.pItem[0].usFlags = fTickBox;
			tTreeLine.pItem[0].pData = (void *) i;
			tTreeLine.pItem[1].usFlags = bEnable?fCombo:fStatic;
			tTreeLine.pItem[1].pData = pCounter;
			tTreeLine.pItem[1].eEditType = cCounterDec;
			InsertUserData( hItem, &tTreeLine );

			iNum++;
		}
		if (pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_USED) iNumUsedSubCounter ++;
	}

	if (iNumUsedSubCounter  >= VCA5_MAX_NUM_SUBCOUNTERS) return;

	// Append a new one after all loader counters
	s.Format( _T("Dec %d"), iNum );
	hItem =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentInc );
	SetItem( hItem, 1, TVIF_TEXT, _T(""), 0,0,0,0,0);

	memset( &pCounter->pSubCounters[pCounter->usNumSubCounters], 0, sizeof( VCA5_SUBCOUNTER ) );
	pCounter->uiSubCounterStatuses[pCounter->usNumSubCounters] = 0;
	SetItemTick( hItem, uNotSelectState);

	//Insert the user data, 2 column
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fTickBox;
	tTreeLine.pItem[0].pData = (void *) pCounter->usNumSubCounters;
	tTreeLine.pItem[1].usFlags = bEnable?fCombo:fStatic;
	tTreeLine.pItem[1].eEditType = cCounterDec;
	tTreeLine.pItem[1].pData = pCounter;
	InsertUserData( hItem, &tTreeLine );

	SetItemExpand( hParentInc, 1 );
	return;
}


void CZoneTreeCtrl::AddCounterInstant( VCA5_APP_COUNTER *pCounter, HTREEITEM hParent, BOOL bEnable )
{
	CString	s;
	unsigned int i = 0;
	int	iNum = 0, iNumUsedSubCounter = 0;
	HTREEITEM hItem, hParentInc;
	UINT uIcon = uIconItemCounter[0];
	VCA_TREELIST_LINE_T	tTreeLine;
	VCA5_APP_ZONE *pTrigZone;
	VCA5_APP_RULE *pTrigRule;
	BOOL rs = FALSE;
	unsigned char uNotSelectState = bEnable ? STATEIMAGE_CHECKBOX_UNCHECKED : STATEIMAGE_CHECKBOX_DISABLE;
	USES_CONVERSION;

	s.Format( _T("Occupancy") );
	hParentInc = InsertItem( s, uIcon, uIcon, hParent );
	// No tick box
	SetItemTick( hParentInc, -1 );

	//Insert the user data
	tTreeLine.eType = VCA_OP_COUNTER;
	tTreeLine.usNumColumns = 1;
	tTreeLine.pItem[0].usFlags = fStatic;
	tTreeLine.pItem[0].pData = &usINS;
	InsertUserData( hParentInc, &tTreeLine );

	for ( i = 0; i < pCounter->usNumSubCounters; i ++ )
	{
		unsigned short usCounterType = pCounter->pSubCounters[i].usSubCounterType;

		if ((usCounterType&VCA5_COUNTER_INSTANT)&&(pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_TICKED))
		{
			pTrigRule = m_pDataMgr->GetRuleByInternalId(pCounter->pSubCounters[i].usTrigId);
			if(!pTrigRule) continue;
			if(!m_pDataMgr->CheckFeatureByRuleType(pTrigRule->usRuleType)) continue;

			s.Format( _T("Occ %d"), iNum );
			hItem =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentInc );
			
			s.Empty();
			if (pTrigRule && pTrigRule->ucTicked) {
				pTrigZone = m_pDataMgr->GetZoneById(pTrigRule->usZoneId);
				if (pTrigZone) {
					s.Format( _T("%s"), CA2T(pTrigRule->szRuleName, CP_UTF8 ));
					SetItem( hItem, 1, TVIF_USECOLORCB, s,0,0,0,0, pTrigZone->uiColor);	
				} else {
					pCounter->uiSubCounterStatuses[i] = 0;
					s.Format( _T("Error, invalid zone") );
				}
			} else {
				pCounter->uiSubCounterStatuses[i] = 0;
				s.Format( _T("Error, invalid rule") );
			}

			SetItemTick( hItem, (pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_TICKED) ? 1 : uNotSelectState);
			SetItem( hItem, 1, TVIF_TEXT, s, 0,0,0,0,0);

			//Insert the user data, 2 column
			tTreeLine.usNumColumns = 2;
			tTreeLine.pItem[0].usFlags = fTickBox;
			tTreeLine.pItem[0].pData = (void *) i;
			tTreeLine.pItem[1].usFlags = bEnable?fCombo:fStatic;
			tTreeLine.pItem[1].pData = pCounter;
			tTreeLine.pItem[1].eEditType = cCounterIns;
			InsertUserData( hItem, &tTreeLine );

			iNum++;
		}
		if (pCounter->uiSubCounterStatuses[i]&VCA5_APP_SUBCOUNTER_STATUS_TICKED) iNumUsedSubCounter++;
	}

	if (iNumUsedSubCounter >= VCA5_MAX_NUM_SUBCOUNTERS) return;


	// Append a new one after all loader counters
	s.Format( _T("Occ %d"), iNum );
	hItem =  InsertItem( s, TV_NOIMAGE, TV_NOIMAGE, hParentInc );
	SetItem( hItem, 1, TVIF_TEXT, _T(""), 0,0,0,0,0);

	memset( &pCounter->pSubCounters[pCounter->usNumSubCounters], 0, sizeof( VCA5_SUBCOUNTER ) );
	pCounter->uiSubCounterStatuses[pCounter->usNumSubCounters] = 0;
	SetItemTick( hItem, uNotSelectState);

	//Insert the user data, 2 column
	tTreeLine.usNumColumns = 2;
	tTreeLine.pItem[0].usFlags = fTickBox;
	tTreeLine.pItem[0].pData = (void *) pCounter->usNumSubCounters;
	tTreeLine.pItem[1].usFlags = bEnable?fCombo:fStatic;
	tTreeLine.pItem[1].eEditType = cCounterIns;
	tTreeLine.pItem[1].pData = pCounter;
	InsertUserData( hItem, &tTreeLine );

	SetItemExpand( hParentInc, 1 );
}

void CZoneTreeCtrl::AddCounterTree( VCA5_APP_COUNTER *pCounter, HTREEITEM hParent )
{
	CString s;
	
	if ( ( pCounter ) )
	{
		BuildCounterComboBoxByRule( pCounter );
		
		//SetColumnAutoEdit( 1,TVAE_COMBO | TVAE_FULLWIDTH,'|',_T("Test|Auto"));

		VCA5_APP_COUNTER CounterTemp;
		memcpy(&CounterTemp, pCounter, sizeof(VCA5_APP_COUNTER));

		//Reset subcount 
		CounterTemp.usNumSubCounters = 0;
		memset(CounterTemp.uiSubCounterStatuses, 0, sizeof(CounterTemp.uiSubCounterStatuses));
		memset(CounterTemp.pSubCounters, 0, sizeof(CounterTemp.pSubCounters));

		// instant is exclusive with increment and decrement and clean up unused sub counter
		DWORD uNumInst = 0;
		DWORD uNumUseded = 0;
		for (DWORD i = 0; i < pCounter->usNumSubCounters; i++) {
			if(pCounter->uiSubCounterStatuses[i] & VCA5_APP_SUBCOUNTER_STATUS_USED){
				
				CounterTemp.pSubCounters[CounterTemp.usNumSubCounters] =  pCounter->pSubCounters[i];
				CounterTemp.uiSubCounterStatuses[CounterTemp.usNumSubCounters] = pCounter->uiSubCounterStatuses[i];
				if (pCounter->pSubCounters[i].usSubCounterType == VCA5_COUNTER_INSTANT) {
					uNumInst++;
				}
				uNumUseded++;
				CounterTemp.usNumSubCounters++;
			}
		}
		memcpy(pCounter, &CounterTemp, sizeof(VCA5_APP_COUNTER));
		
		AddCounterIncrement( pCounter, hParent, (uNumInst == 0 || uNumUseded == 0) ? TRUE : FALSE);
		AddCounterDecrement( pCounter, hParent, (uNumInst == 0 || uNumUseded == 0) ? TRUE : FALSE);
		AddCounterInstant( pCounter, hParent, (uNumInst > 0 || uNumUseded == 0) ? TRUE : FALSE);
	}
}

BOOL CZoneTreeCtrl::IsDirty()
{
	BOOL bvalue = m_bImageChanged;
	m_bImageChanged = ! m_bImageChanged;
	return bvalue;
}

void CZoneTreeCtrl::SetRoot()
{
	m_hRoot = InsertItem( _T("Basic Rules"), 0, 0, TVI_ROOT);
}

//*****************************************************************************
//*
//*		OnSelChanged
//*
//*****************************************************************************
void CZoneTreeCtrl::OnSelChanged(NMHDR *pNmHdr,LRESULT *pResult)
{
NM_TREEVIEW *pNmTreeView = (NM_TREEVIEW*)pNmHdr;
TV_ITEM		 sItem;
TCHAR		 cUserData[256];
TCHAR		 cText[256]=_T("");	
COLORREF	 uColor;	
CString		 sCol;

	
	if(pNmTreeView->itemOld.hItem)						// Update User-Data
		{
//			i=USER_DATA_SIZE;
//			cUserData[sizeof(cUserData)/sizeof(cUserData[0])-1]=0;
//			memcpy(m_cTreeList.GetUserData(m_hSelect),cUserData,i);	
		}


	if(!pNmTreeView->itemNew.hItem)
	{
		m_hSelect	 = NULL;
		m_iSelCol	 = 0;
		m_uSelState	 = 0;
		m_uSelState	 = 0;
		cUserData[0] = 0;   
		sCol		 = _T("");

	}
	else
	{
		m_hSelect	= pNmTreeView->itemNew.hItem;
		m_iSelCol	= pNmTreeView->itemNew.cChildren;
		m_uSelState	= pNmTreeView->itemNew.state;
		
		sCol.Format(_T("%i"),m_iSelCol);

		sItem.mask		 = TVIF_TEXT|TVIF_HANDLE|TVIF_SUBITEM;
		sItem.pszText	 = cText;
		sItem.cchTextMax = sizeof(cText);
		sItem.hItem		 = m_hSelect;
		sItem.cChildren	 = m_iSelCol;
		
		GetItem(&sItem);


		uColor = GetItemBkColor(m_hSelect,m_iSelCol);
		
		uColor = GetItemTextColor(m_hSelect,m_iSelCol);

	}	
	
	*pResult = 0;



}


void CZoneTreeCtrl::OnCbStateChanged(NMHDR *pNmHdr,LRESULT *pResult)
{
	NM_TREEVIEW *pNmTreeView = (NM_TREEVIEW*)pNmHdr;
	TCHAR		 cText[256]=_T("");	
	CString		 sCol;
	int			uiEvent = 0;

	
	if ( pNmTreeView->itemNew.hItem )
	{
		m_hSelect	= pNmTreeView->itemNew.hItem;
		m_iSelCol	= pNmTreeView->itemNew.cChildren;
		m_uSelState	= pNmTreeView->itemNew.state;

		switch ( m_iTreeType )
		{
		case TREELIST_ZONE:
			{
				VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
				if ( pLine->pItem[0].usFlags & fTickBox )
				{
					VCA5_APP_RULE*	pRule = (VCA5_APP_RULE	 *) pLine->pItem[0].pData;
					int				iRuleType = pRule->usRuleType;
					
					if ((iRuleType == VCA5_RULE_TYPE_LINECOUNTER_A || iRuleType == VCA5_RULE_TYPE_LINECOUNTER_B) && pLine->usCanEditRuleName != 1)
					{
						VCA5_APP_RULE*	pRuleB = (VCA5_APP_RULE *) pLine->pItem[0].pData2;

						if (pLine->usNumColumns == 2) // width calibration has 2 columns
						{
							pRule->ucWidthCalibrationEnabled = (m_uSelState & 0x2000)? 1:0;
							pRuleB->ucWidthCalibrationEnabled = (m_uSelState & 0x2000)? 1:0;
						}
						else if (pLine->usNumColumns == 1) // shadow filter has 1 column
						{
							pRule->ucShadowFilterEnabled = (m_uSelState & 0x2000)? 1:0;
							pRuleB->ucShadowFilterEnabled = (m_uSelState & 0x2000)? 1:0;
						}

					}
					else if ( pLine->usCanEditRuleName == 1)
					{
						
						pRule->ucTicked = (m_uSelState & 0x2000)? 1:0;
						VCA5_APP_ZONE	*pZone	= m_pDataMgr->GetZone( m_iCurArea );
						VCA5_APP_RULE	*pRule = m_pDataMgr->GetRule( m_iCurArea, (VCA5_RULE_TYPE_E) iRuleType );
						
						
						if(pRule->ucTicked) {

							char tmp [32];
							wcstombs_s(0, tmp, 32, strTreeItem[iRuleType-1][0], _TRUNCATE );
							
							
							sprintf_s(pRule->szRuleName, 32, "%.10s-%.20s", pZone->szZoneName, tmp);
							
							if ( ( iRuleType == VCA5_RULE_TYPE_TAILGATING ) && ( pZone->usZoneStyle == VCA5_ZONE_STYLE_TRIPWIRE ) )
							{
								ValidTailgatingRule( pRule, pZone->usZoneId );
								if ( m_pDataMgr->CheckFeature( VCA5_FEATURE_LINECOUNTER ) )
								{
									pRule->tRuleDataEx.tTimer.usFlags = VCA5_RULE_TAILGATING_BYLINECOUNTER;
								}
							}

							if (pZone->uiStatus&VCA5_APP_AREA_STATUS_INITIAL_RULE) {
								pRule = m_pDataMgr->GetRule(m_iCurArea,  VCA5_RULE_TYPE_PRESENCE);
								pRule->ucTicked = FALSE;
								pZone->uiStatus &= ~VCA5_APP_AREA_STATUS_INITIAL_RULE;
								uiEvent |= VCA_ZONE_CHANGE;
							}
						}
						RedrawZoneTree(pZone);
					}else if ( _tcsncmp( pNmTreeView->itemNew.pszText, _T("Filter"), 6) == 0) { // Object Filter
						VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
						DWORD uTicked = (m_uSelState & 0x2000)? 1:0;

						*((BYTE *) pLine->pItem[0].pData) = uTicked ? 1 : 0;
						ApplyObjclsFilter();

						ActiveCombo( m_hSelect, &m_tObjClsIdCombo );
					}else {
						// Refresh counters for the zone
						VCA5_APP_COUNTERS *pCounters = m_pDataMgr->GetCounters();
						for (ULONG i = 0; i < pCounters->ulTotalCounters; i++) {
							VCA5_APP_COUNTER *pCounter = &pCounters->pCounters[i];

							//if (pCounter->uiZoneId == pZone->usZoneId) {
								for (USHORT j = 0; j < pCounter->usNumSubCounters; j++) {
									if (pCounter->pSubCounters[j].usTrigId == pRule->ulRuleId) {
										pCounter->uiSubCounterStatuses[j] &= ~VCA5_APP_SUBCOUNTER_STATUS_TICKED;
									}
								}
							//}
						}

						m_pDataMgr->RemoveRule(m_iCurArea, (VCA5_RULE_TYPE_E)iRuleType);
					}
					
					m_pDataMgr->FireEvent(VCA_RULE_UPDATE|uiEvent, this);
				} 
			}
			break;

		case TREELIST_COUNTER:
			{

				VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
				VCA5_APP_COUNTER *pCounter = (VCA5_APP_COUNTER *) pLine->pItem[1].pData;
				int iSubCounterIdx = (int) pLine->pItem[0].pData;
				if ( pCounter )
				{
					
					if ((m_pDataMgr->GetNumTickedCounter(pCounter)< VCA5_MAX_NUM_SUBCOUNTERS) && ( m_uSelState & 0x2000 ) ) {
						pCounter->uiSubCounterStatuses[iSubCounterIdx] |= VCA5_APP_SUBCOUNTER_STATUS_TICKED;
						ActiveCombo(m_hSelect, &m_tRuleCounterCombo);
					} else {
						pCounter->uiSubCounterStatuses[iSubCounterIdx] = 0;
						RedrawCounterTree(pCounter);
					}

					m_pDataMgr->FireEvent(VCA_COUNTER_UPDATE, this);
				}

			}
			break;
		
		case TREELIST_SHOWALL:
			{
				VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
				if ( pLine->pItem[m_iSelCol].usFlags & fTickBox )
				{
					RedrawAllZoneListTree();
				}
			}
			break;
		}
	}

}

#define IS_ITEM_EDITABLE( usFlag ) ((usFlag & fEdit) || (usFlag & fCombo) )

// Mouse lClick
void CZoneTreeCtrl::OnItemlClick( NMHDR *pNmHdr,LRESULT *pResult, HWND hWnd )
{
	NM_TREEVIEW *pNmTreeView = (NM_TREEVIEW*)pNmHdr;
	TCHAR		 cText[256]=_T("");	


	if ( pNmTreeView->itemNew.hItem )
	{
		m_hSelect	= pNmTreeView->itemNew.hItem;
		m_iSelCol	= pNmTreeView->itemNew.cChildren;
		m_uSelState	= pNmTreeView->itemNew.state;
		
		if ( m_iSelCol == 1 )
		{

			GetItemText( m_hSelect, cText, 80, 0 );
		
			switch ( m_iTreeType )
			{
			case TREELIST_ZONE:
//				if (( m_iSelCol == 1 ) && FindTreeItemIndex( cText, &iRuleId, &iRuleType, &iRuleOption ))
//				{
//					if ( iRuleOption>0 && (m_pDataMgr->CheckFeatureByRuleType(iRuleType)))
//						pEdit = EditLabel(m_hSelect,m_iSelCol,0);
//				}
//				else
				{
					VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
					if ( !pLine )
						break;

					if ( m_iSelCol + 1 > pLine->usNumColumns )
						break;
					switch ( pLine->pItem[m_iSelCol].usFlags )
					{
					case fEdit:
						{
							CEdit *pEdit = EditLabel( m_hSelect,m_iSelCol,0 );
							pEdit->SetFocus();
							//pEdit->Clear();
						}
						break;
					case fCombo:
						{
							switch ( pLine->pItem[m_iSelCol].eEditType ) 
							{
							case cZoneColor:
								ActiveCombo(m_hSelect, &m_tZoneColorCombo);
								break;
							case cZoneType:
								ActiveCombo(m_hSelect, &m_tZoneTypeCombo);
								break;
							case cZoneObjclsId:
								ActiveCombo(m_hSelect, &m_tObjClsIdCombo);
								break;
							case cZoneObjclsLogic:
								ActiveCombo(m_hSelect, &m_tObjClsLogicCombo);
								break;
							case cTailgatingMode:
								ActiveCombo(m_hSelect, &m_tTailGateModeCombo);
								break;
							case cColBin:
								{
									VCA5_APP_RULE* pRule = (VCA5_APP_RULE *) pLine->pItem[0].pData;
									BuildColorBinCombo( pRule );
 									ActiveCombo(m_hSelect, &m_tColorBinCombo);
 									break;
								}
							}
						}
						break;
					default:
						break;

					}
				}
				break;
			case TREELIST_COUNTER:
				{
					VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
					if ( !pLine )
						break;

					if ( m_iSelCol + 1 > pLine->usNumColumns )
						break;

					switch ( pLine->pItem[m_iSelCol].usFlags )
					{
					case fEdit:
						{
							CEdit *pEdit = EditLabel( m_hSelect,m_iSelCol,0 );
							pEdit->SetFocus();
	//						pEdit->Clear();
						}
						break;
					case fCombo:
						{
							switch ( pLine->pItem[m_iSelCol].eEditType ) 
							{
							case cCounterInc:
							case cCounterDec:
							case cCounterIns:
								ActiveCombo( m_hSelect, &m_tRuleCounterCombo );
								break;
							case cCounterColor:
								ActiveCombo(m_hSelect, &m_tZoneColorCombo);
								break;
							case cZoneType:
								ActiveCombo(m_hSelect, &m_tZoneTypeCombo);
								break;
							}							
						}
						break;
					default:
						break;

					}
				}
				break;
			default:
				break;
			}
		}
		else if ( m_iSelCol == 0 )
		{
			switch ( m_iTreeType )
			{
			case TREELIST_SHOWALL:
				{
					VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
					if ( pLine->pItem[m_iSelCol].usFlags & fTickBox )
					{
						switch( pLine->eType )
						{
						case VCA_OP_ZONE:
							{
								VCA5_APP_ZONE	*pZone = (VCA5_APP_ZONE *) pLine->pItem[m_iSelCol].pData;
								m_pDataMgr->ClearAllNotifyAreas();
								m_pDataMgr->SetNotifyArea(pZone->usZoneId, VCA5_APP_AREA_T_ZONE);
								m_pDataMgr->FireEvent(VCA_ZONE_UPDATE, this);
							}
							break;
						case VCA_OP_COUNTER:
							{
								VCA5_APP_COUNTER	*pCounter = (VCA5_APP_COUNTER *) pLine->pItem[m_iSelCol].pData;
								m_pDataMgr->ClearAllNotifyAreas();
								m_pDataMgr->SetNotifyArea(pCounter->usCounterId, VCA5_APP_AREA_T_COUNTER);
								m_pDataMgr->FireEvent(VCA5_APP_AREA_T_COUNTER, this);
							}
							break;
						}
					}
				}
				break;
			}
		
		}
	}
}

// Mouse DblClick
void CZoneTreeCtrl::OnItemDblClick(NMHDR *pNmHdr,LRESULT *pResult, HWND hWnd)
{
	NM_TREEVIEW *pNmTreeView = (NM_TREEVIEW*)pNmHdr;
	TCHAR		 cText[256]=_T("");	

	if ( pNmTreeView->itemNew.hItem )
	{
		m_hSelect	= pNmTreeView->itemNew.hItem;
		m_iSelCol	= pNmTreeView->itemNew.cChildren;
		m_uSelState	= pNmTreeView->itemNew.state;
		
		if ( m_iSelCol == 0 )
		{
			switch ( m_iTreeType )
			{
				case TREELIST_SHOWALL:
				{
					VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
					if ( pLine->pItem[m_iSelCol].usFlags & fTickBox )
					{
						switch( pLine->eType )
						{
							case VCA_OP_ZONE:
							{
								VCA5_APP_ZONE	*pZone = (VCA5_APP_ZONE *) pLine->pItem[m_iSelCol].pData;
								m_pDataMgr->SetSelectArea(pZone->usZoneId, VCA5_APP_AREA_T_ZONE);
								m_pDataMgr->FireEvent(VCA_ZONE_UPDATE, NULL);
							}
							break;
						
							case VCA_OP_COUNTER:
							{
								VCA5_APP_COUNTER	*pCounter = (VCA5_APP_COUNTER *) pLine->pItem[m_iSelCol].pData;
								m_pDataMgr->SetSelectArea(pCounter->usCounterId, VCA5_APP_AREA_T_COUNTER);
								m_pDataMgr->FireEvent(VCA_COUNTER_UPDATE, NULL);
							}
							break;
						}
					}
					
				}
				break;

				default:	break;
			}
		}
	}
}

void CZoneTreeCtrl::OnBeginLabelEdit (NMHDR *pNmHdr,LRESULT *pResult)
{
	NMTVDISPINFO   *pHeader = (NMTVDISPINFO*)pNmHdr;
	TCHAR					cText[256]=_T("");
	CString					s;
	
	m_bEditing = TRUE;
	switch ( m_iTreeType )
	{
	case TREELIST_ZONE:
		{
			VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
			if ( IS_ITEM_EDITABLE( pLine->pItem[1].usFlags ) )
			{
				GetItemText( m_hSelect, cText, 256, 1);

				switch( pLine->pItem[1].eEditType ){
				case edDirection:
				case edAcceptance:
					s.Format(_T("%s"), cText); 
					s.SetAt( s.GetLength()-1, '\0');
					SetItemText( m_hSelect, s, 1 );
					break;
				}
			}
		}
		break;
	case TREELIST_COUNTER:
		{
		}
		break;
	default:
		break;
	}
	
}

void CZoneTreeCtrl::ValidateAngle(	VCA5_RULE_DIRECTION *pDT )
{
	int startangle = pDT->sStartAngle;
	int finishangle = pDT->sFinishAngle;

	if ( ( finishangle > startangle ) && ( finishangle - startangle < 3600 ) )
	{
		do
		{
			if (startangle < 0)
			{
				startangle += 3600;
				finishangle += 3600;
			}

			if (startangle > 3600)
			{
				startangle -= 3600;
				finishangle -= 3600;
			} 
		}while ( ( startangle< 0 ) || (startangle > 3600));
		pDT->sStartAngle = startangle;
		pDT->sFinishAngle = finishangle;
	}

}

void CZoneTreeCtrl::ValidateSingleAngle( int *pAngle )
{
	do
	{
		*pAngle = ( *pAngle > 180 ) ? *pAngle-360 : *pAngle;
		*pAngle = ( *pAngle < -180 ) ? *pAngle+360 : *pAngle;
	} while ( ( *pAngle<-180) || ( *pAngle>180) );
}

void ValidateSpeed( VCA5_RULE_SPEED *pSF)
{
	if (pSF->usSpeedLo > pSF->usSpeedUp) {

	}
}

BOOL CZoneTreeCtrl::SetSubCounter( VCA5_SUBCOUNTER *pData, TCHAR* pText, HTREEITEM hParent )
{
	int i;
	CString s1, s2;
	s1.Format(_T("%s"), pText );

	VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( hParent );
	unsigned short *pType = (unsigned short *)pLine->pItem[0].pData;

	for ( i = 0; i < m_tRuleCounterCombo.uiNumComboItems ; i++)
	{
		s2.Format( _T("%s"), m_tRuleCounterCombo.pComboItem[i].pszComboString );
		if ( s1 == s2 )
		{
			pData->usSubCounterType = *pType;
			pData->usTrigId = m_tRuleCounterCombo.pComboItem[i].usTrigId;
			
			return TRUE;
		}
	}
	return FALSE;
}

#define SetCounterColor SetZoneColor

BOOL CZoneTreeCtrl::SetZoneColor( DWORD *puiColor, TCHAR* pText, HTREEITEM hParent )
{
	int i;
	CString s1, s2;
	s1.Format(_T("%s"), pText );

	for ( i = 0; i < m_tZoneColorCombo.uiNumComboItems ; i++)
	{
		s2.Format( _T("%s"), m_tZoneColorCombo.pComboItem[i].pszComboString );
		if ( s1 == s2 )
		{
			*puiColor = m_tZoneColorCombo.pComboItem[i].crColour;
			return TRUE;
		}
	}
	return FALSE;
}


int CZoneTreeCtrl::SetTailgatingMode( unsigned short *pusMode, TCHAR* pText, HTREEITEM hParent )
{
	int i;
	CString s1, s2;
	s1.Format(_T("%s"), pText );

	for ( i = 0; i < m_tTailGateModeCombo.uiNumComboItems ; i++)
	{
		s2.Format( _T("%s"), m_tTailGateModeCombo.pComboItem[i].pszComboString );
		if ( s1 == s2 )
		{
			*pusMode = m_tTailGateModeCombo.pComboItem[i].usMode;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CZoneTreeCtrl::SetZoneType( USHORT *pusZoneType, TCHAR* pText, HTREEITEM hParent )
{
	int i;
	CString s1, s2;
	s1.Format(_T("%s"), pText );

	for ( i = 0; i < m_tZoneTypeCombo.uiNumComboItems ; i++)
	{
		s2.Format( _T("%s"), m_tZoneTypeCombo.pComboItem[i].pszComboString );
		if ( s1 == s2 )
		{
			*pusZoneType = m_tZoneTypeCombo.pComboItem[i].usZoneType;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CZoneTreeCtrl::SetZoneObjclsId( SHORT *pcId, TCHAR* pText, HTREEITEM hParent )
{
	int i;
	CString s1, s2;
	s1.Format(_T("%s"), pText );

	for ( i = 0; i < m_tObjClsIdCombo.uiNumComboItems ; i++)
	{
		s2.Format( _T("%s"), m_tObjClsIdCombo.pComboItem[i].pszComboString );
		if ( s1 == s2 )
		{
			*pcId = m_tObjClsIdCombo.pComboItem[i].sObjClsId;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CZoneTreeCtrl::SetZoneObjclsLogic( unsigned char *pucLogic, TCHAR* pText, HTREEITEM hParent )
{
	int i;
	CString s1, s2;
	s1.Format(_T("%s"), pText );

	for ( i = 0; i < m_tObjClsLogicCombo.uiNumComboItems ; i++)
	{
		s2.Format( _T("%s"), m_tObjClsLogicCombo.pComboItem[i].pszComboString );
		if ( s1 == s2 )
		{
			*pucLogic = m_tObjClsLogicCombo.pComboItem[i].ucObjClsLogic;
			return TRUE;
		}
	}
	return FALSE;
}

int CZoneTreeCtrl::SetColorBin( unsigned short *pusColBin, TCHAR* pText, HTREEITEM hParent )
{
	int i;
	CString s1, s2;
	s1.Format(_T("%s"), pText );

	for ( i = 0; i < m_tColorBinCombo.uiNumComboItems ; i++)
	{
		s2.Format( _T("%s"), m_tColorBinCombo.pComboItem[i].pszComboString );
		if ( s1 == s2 )
		{
			*pusColBin = m_tColorBinCombo.pComboItem[i].usColBin;
			return TRUE;
		}
	}
	return FALSE;
}

int CZoneTreeCtrl::AddZoneObjCls( OBJCLS_FILTER_T *pOF )
{
	if ( pOF->cId != VALUE_NA )
	{
		if ( pOF->ucIsNew )
		{
			int	iZoneIdx;
			VCA5_APP_AREA_T_E eAreaType;
			VCA5_APP_ZONE *pZone;

			pOF->ucIsNew = 0;
			pOF->ucTicked = 1;
			
			m_pDataMgr->GetSelectAreaIdx(&iZoneIdx, &eAreaType);
			pZone = m_pDataMgr->GetZone(iZoneIdx);
			if (pZone!=NULL && eAreaType==VCA5_APP_AREA_T_ZONE) {
				//OBJCLS_FILTERS_T *pObjFilter = &m_pObjClsFilters[iZoneIdx];
				OBJCLS_FILTERS_T *pObjFilter = GetObjclsFilter(pZone->usZoneId);
				int idx = pObjFilter->iTotalTrkObjs++;
				pObjFilter->tObjclsFilter[idx].cId = VALUE_NA;
				pObjFilter->tObjclsFilter[idx].ucTicked = 0;
				pObjFilter->tObjclsFilter[idx].ucIsNew = 1;
			}
		}
		return TRUE;
	}
	else
		return FALSE;
}

void CZoneTreeCtrl::OnEndLabelEdit(NMHDR *pNmHdr,LRESULT *pResult)
{
	NMTVDISPINFO*	pHeader = (NMTVDISPINFO*)pNmHdr;
	TCHAR			cText[256]=_T("");	
	CString			sCol, sValue, s;
	
	VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
	switch ( m_iTreeType )
	{
		case TREELIST_ZONE:
		{
			if ( pHeader->item.pszText == NULL )
				break;

			switch ( pLine->pItem[1/*m_iSelCol*/].usFlags )
			{
				case fEdit:
				{
					switch( pLine->pItem[1].eEditType )
					{
						case edZoneName:
						case edCounterName:
						{
							USES_CONVERSION;
							if ( pHeader->item.pszText )
							{
								// BW: Prevent buffer overflow!

								strncpy ( (char *) pLine->pItem[1/*m_iSelCol*/].pData, CT2A( pHeader->item.pszText, CP_UTF8 ), VCA5_MAX_STR_LEN-1 );
								((char *)pLine->pItem[1/*m_iSelCol*/].pData)[VCA5_MAX_STR_LEN-1] = '\0';
								if ( !ValidateName( (char *) pLine->pItem[1/*m_iSelCol*/].pData ) )
								{
									AfxMessageBox( gszWarning[0] );
								}
							}
						}
						break;

						case edTimeThreshold:
						{
							VCA5_APP_RULE *pRule = (VCA5_APP_RULE *) pLine->pItem[0].pData;
							switch ( pRule->usRuleType )
							{
								case VCA5_RULE_TYPE_STOP:
								case VCA5_RULE_TYPE_DWELL:
								case VCA5_RULE_TYPE_TAILGATING:
								case VCA5_RULE_TYPE_SMOKE:
								case VCA5_RULE_TYPE_FIRE:
								{
									unsigned int *puiTimeThreshold = (unsigned int *) pLine->pItem[1].pData;
									*puiTimeThreshold = min( _ttoi( pHeader->item.pszText ), 60*60 );
								}
								break;

//								case VCA5_RULE_TYPE_ABOBJ:
								case VCA5_RULE_TYPE_RMOBJ:
								{
/*
									// BW: make sure that threshold does not exced maximum hold-on time
									#define MAXStatHoldonTime 120 //@@ need check NVC
									unsigned int *puiTimeThreshold = (unsigned int *) pLine->pItem[1].pData;
									*puiTimeThreshold = min( _ttoi( pHeader->item.pszText ), MAXStatHoldonTime );
*/
									unsigned int *puiTimeThreshold = (unsigned int *) pLine->pItem[1].pData;
									unsigned int uiOldTime = *puiTimeThreshold;
									*puiTimeThreshold = max(3, min( (unsigned int )_ttoi( pHeader->item.pszText ), 
										(unsigned int)m_pDataMgr->GetAdvancedInfo()->TrackerParams.ulSecsToHoldOn));

									if (*puiTimeThreshold < 10)
									{
										RECT itemRect;
										GetItemRect(m_hSelect, 0, &itemRect, TVIR_GETCOLUMN);		
										CPoint pnt = CPoint(itemRect.left + GetColumnWidth(0) + 10, itemRect.bottom - 10);
										ShowCustomToolTip(m_hWnd, pnt, L"WARNING: values less than 10 sec can\n result in a higher number of false alarms", 5000);
									}


								}
								break;
							}
						}
						break;

						case edDirection:
						case edAcceptance:
						{
							BOOL bCheck = FALSE;
							VCA5_RULE_DIRECTION *pDT = (VCA5_RULE_DIRECTION *) pLine->pItem[1].pData;
							int	oldstart = pDT->sStartAngle;
							int	oldfinish  = pDT->sFinishAngle;
							int iItem = _ttoi( pHeader->item.pszText );

							if ( pLine->pItem[1].eEditType == edDirection )
							{
								iItem = 90 - iItem;
								ValidateSingleAngle( &iItem );
								pDT->sStartAngle =  iItem * 10 -  ( oldfinish - oldstart ) / 2;
								pDT->sFinishAngle = pDT->sStartAngle + ( oldfinish - oldstart );
							}
							else if ( pLine->pItem[1].eEditType == edAcceptance )
							{
								if ( ( iItem <= 0 ) || ( iItem >= 360 ) )
								{
									AfxMessageBox( _T("acceptance angle range [1,359]"), 0, 0 );
									bCheck = TRUE;
								}
								else
								{
									pDT->sStartAngle = ( oldstart + oldfinish )/2 - iItem * 10 / 2;
									pDT->sFinishAngle = ( oldstart + oldfinish )/2 + iItem * 10 / 2;
								}
							}

							if ( bCheck )
							{
								pDT->sStartAngle = oldstart;
								pDT->sFinishAngle = oldfinish;
							}
							else
							{
								s.Format( _T("%s°"), pHeader->item.pszText );
								SetItemText( m_hSelect, s, 1);
								ValidateAngle( pDT );
							}
						}
						break;

						case edThres:
							{
								unsigned short* usThres = (unsigned short*) pLine->pItem[1].pData;
								int iItem = _ttoi( pHeader->item.pszText );

								if( ( iItem < 0 ) || ( iItem > 100 ) )
								{
									AfxMessageBox( _T("acceptance threshold range [0,100]"), 0, 0 );
								}
								else
								{
									*usThres = (unsigned short) ( (2.54*((float)iItem)) + 0.5 );
									//s.Format( _T("%s"), pHeader->item.pszText );
									//SetItemText( m_hSelect, s, 1);
								}
							}
							break;

						case edSpeedLow:
						case edSpeedUp:
						{
							BOOL bCheck = FALSE;
							VCA5_RULE_SPEED *pSF = (VCA5_RULE_SPEED *) pLine->pItem[1].pData;
							int oldlower = (int) pSF->usSpeedLo;
							int oldupper = (int) pSF->usSpeedUp;
							int iItem = _ttoi( pHeader->item.pszText );

							if ( pLine->pItem[1].eEditType == edSpeedLow )
							{
								if ( ( iItem < 0 ) || ( iItem > 65535 ) )
								{
									AfxMessageBox( _T("acceptance speed range [0,65535]"), 0, 0 );
									bCheck = TRUE;
								}
								else
									pSF->usSpeedLo = iItem;
							}
							else if ( pLine->pItem[1].eEditType == edSpeedUp )
							{
									if ( ( iItem < 0 ) || ( iItem > 65535 ) )
									{
										AfxMessageBox( _T("acceptance speed range [0,65535]"), 0, 0 );
										bCheck = TRUE;
									}
									else
										pSF->usSpeedUp = iItem;
							}

							if ( bCheck )
							{
								pSF->usSpeedLo = oldlower;
								pSF->usSpeedUp = oldupper;
							}
							else
							{
								s.Format( _T("%s°"), pHeader->item.pszText );
								SetItemText( m_hSelect, s, 1);
							}
						}
						break;

						case edLineCounter:
						{
							BOOL bCheck = FALSE;
							VCA5_RULE_LINECOUNTER *pLC_B = (VCA5_RULE_LINECOUNTER *) pLine->pItem[1].pData;
							VCA5_RULE_LINECOUNTER *pLC_A = (VCA5_RULE_LINECOUNTER *) pLine->pItem[1].pData2;

							double dItem = min(100, _wtof( pHeader->item.pszText ));
					
							pLC_A->ulCalibrationWidth = (unsigned int)(65535.0*dItem)/100;
							pLC_B->ulCalibrationWidth = (unsigned int)(65535.0*dItem)/100;

							s.Format( _T("%s"), pHeader->item.pszText );
							SetItemText( m_hSelect, s, 1);
						}
						break;

						case edEventName:
						{
							size_t size = 0;
							char *str = (char*)pLine->pItem[1].pData;
							strncpy ( str, CT2A( pHeader->item.pszText, CP_UTF8 ), VCA5_MAX_STR_LEN-1 );
							str[VCA5_MAX_STR_LEN-1] = '\0';

							if ( !ValidateName( str ) )
							{
								AfxMessageBox( gszWarning[0] );
							}
						}
						break;
					}

					
				}
				break;

				case fCombo:
				{
					switch ( pLine->pItem[1].eEditType )
					{
						case cColBin:
						{
							unsigned short *usColBin = (unsigned short *) pLine->pItem[1].pData;
							if ( !SetColorBin( usColBin, pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								s.Format( _T(" "));
								SetItemText( m_hSelect, s, 1);
							}
						}
						break;
						case cTailgatingMode:
						{
							unsigned short *pusMode = (unsigned short *) pLine->pItem[1/*m_iSelCol*/].pData;
							if ( !SetTailgatingMode( pusMode, pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								s.Format( _T(" "));
								SetItemText( m_hSelect, s, 1);
							}
						}
						break;

						case cZoneColor:
						{
							DWORD *puiColor = (DWORD *) pLine->pItem[1/*m_iSelCol*/].pData;
							if ( !SetZoneColor( puiColor, pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								s.Format( _T(" "));
								SetItemText( m_hSelect, s, 1);
							}
						}
						break;

						case cZoneType:
						{
							unsigned short *pusZoneType = (unsigned short *) pLine->pItem[1/*m_iSelCol*/].pData;
							if ( !SetZoneType( pusZoneType, pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								s.Format( _T(" "));
								SetItemText( m_hSelect, s, 1);
							}
						}
						break;

						case cZoneObjclsId:
						{
							OBJCLS_FILTER_T *pOF = (OBJCLS_FILTER_T *) pLine->pItem[1/*m_iSelCol*/].pData;
							if ( !SetZoneObjclsId( &pOF->cId, pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								s.Format( _T(" "));
								SetItemText( m_hSelect, s, 1);
							}
							else
							{
								AddZoneObjCls( pOF );
								ApplyObjclsFilter();
							}
						}
						break;

						case cZoneObjclsLogic:
						{
							unsigned char *pucLogic = (unsigned char *) pLine->pItem[1/*m_iSelCol*/].pData;
							if ( !SetZoneObjclsLogic( pucLogic, pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								s.Format( _T(" "));
								SetItemText( m_hSelect, s, 1);
							} else {
								ApplyObjclsFilter();
							}
						}
						break;
#ifdef TMP_ADVANCED_RULE
						case cZoneLogic:
						{
							unsigned char *pucLogic = (unsigned char *) pLine->pItem[1/*m_iSelCol*/].pData;
							if ( !SetZoneEventLogic( pucLogic, pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								s.Format( _T(" "));
								SetItemText( m_hSelect, s, 1);
							}
						}
						break;

						case cZoneMultiWire:
						{
							unsigned short *pusId = (unsigned short *) pLine->pItem[1/*m_iSelCol*/].pData;
							if ( !SetDoubleZoneId( pusId, pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								s.Format( _T(" "));
								SetItemText( m_hSelect, s, 1);
							}
							else
							{
								ValidDoubleId( *pusId, pZone->usZoneId );
							}
						}
						break;
#endif
					}
				}
				break;

			default:
				break;
			}
			m_pDataMgr->FireEvent(VCA_ZONE_UPDATE, this);
			RedrawZoneTree( m_pDataMgr->GetZone( m_iCurArea ) );
		}
		break;

		case TREELIST_COUNTER: 
		{
			VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
			switch ( pLine->pItem[1/*m_iSelCol*/].usFlags )
			{
			case fEdit:
				{
					USES_CONVERSION;
					if ( pHeader->item.pszText )
					{
						// BW: Prevent buffer overflow!
						// convert TCHAR to UTF8 encoding. 
						strncpy ( (char *) pLine->pItem[1/*m_iSelCol*/].pData, CT2A ( pHeader->item.pszText, CP_UTF8), VCA5_MAX_STR_LEN-1 );
						((char *)pLine->pItem[1/*m_iSelCol*/].pData)[VCA5_MAX_STR_LEN-1] = '\0';
						if ( !ValidateName( (char *) pLine->pItem[1/*m_iSelCol*/].pData ) )
						{
							AfxMessageBox( gszWarning[0] );
						}
					}
				}
				break;
			case fCombo:
				{
					switch (pLine->pItem[1].eEditType)
					{
					case cCounterColor:
						{
							DWORD *puiColor = (DWORD *) pLine->pItem[1/*m_iSelCol*/].pData;
							if ( !SetZoneColor( puiColor, pHeader->item.pszText, GetParentItem( m_hSelect) )) {
								s.Format( _T(""));
								SetItemText( m_hSelect, s, 1);
							}
						}
						break;
					default:
						{
							VCA5_APP_COUNTER *pCounter = (VCA5_APP_COUNTER *) pLine->pItem[1/*m_iSelCol*/].pData;
							int iSubCounterIdx = (int) pLine->pItem[0].pData;

							if ( m_pDataMgr->GetNumTickedCounter(pCounter) < VCA5_MAX_NUM_SUBCOUNTERS && 
								SetSubCounter( &pCounter->pSubCounters[iSubCounterIdx], pHeader->item.pszText, GetParentItem( m_hSelect) ) )
							{
								pCounter->uiSubCounterStatuses[iSubCounterIdx] |= VCA5_APP_SUBCOUNTER_STATUS_TICKED;
								pCounter->uiSubCounterStatuses[iSubCounterIdx] |= VCA5_APP_SUBCOUNTER_STATUS_USED;					
								pCounter->usNumSubCounters++;

								m_pDataMgr->FireEvent(VCA_COUNTER_UPDATE, this);
							}
							else
							{
								s.Format( _T(""));
								SetItemText( m_hSelect, s, 1);
							}
						}
						break;
					}
				}
				break;
			default:
				break;
			}
			/*m_cDataMgr.setStatus( VCA_ZONE_UPDATE );
			UpdateDataMgrSubject();*/
			RedrawCounterTree( m_pDataMgr->GetCounter( m_iCurArea ) );
		}
		break;
	default:
		break;
	}
	m_bEditing = FALSE;
}

void CZoneTreeCtrl::OnItemExpanding( NMHDR *pNmHdr,LRESULT *pResult )
{
	NM_TREEVIEW *pNmTreeView = (NM_TREEVIEW*)pNmHdr;
	TCHAR					cText[256]=_T("");
	CString					s;

	
	if ( pNmTreeView->itemNew.hItem )
	{
		m_hSelect	= pNmTreeView->itemNew.hItem;
		m_iSelCol	= pNmTreeView->itemNew.cChildren;
		m_uSelState	= pNmTreeView->itemNew.state;

		switch ( m_iTreeType )
		{
		case TREELIST_ZONE:
			{
				VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * ) GetUserData( m_hSelect );
				if ( pLine->pItem[0].usFlags & fExpand )
				{
					*pLine->pItem[0].pucExpanded = ( m_uSelState & TVIS_EXPANDED ) ? 1 : 0;
				}
			}
			break;
		case TREELIST_COUNTER:
			{
			}
			break;
		default:
			break;
		}
	}

}

void CZoneTreeCtrl::SetCurrentStatus( int iRuleId, int iRuleOption )
{
	m_iPreRule = m_iCurRule;
	m_iCurRule = iRuleId;

	m_iPreOption = m_iCurOption;
	m_iCurOption = iRuleOption;
}


#define MAXTEXTLEN 64
HTREEITEM CZoneTreeCtrl::GetItemByName(TCHAR* szParentName, TCHAR* szItemName)
{
	HTREEITEM hLoop; 
	hLoop = GetChildItem( m_hRoot );
//	hLoop = GetFirstSelected();
	do
	{
		hLoop = GetNextItem(hLoop, TVGN_NEXT);

		TCHAR szBuffer[MAXTEXTLEN+1];
        TV_ITEM item;
		memset(&item, 0, sizeof(TV_ITEM));

        item.hItem = hLoop;
        item.mask = TVIF_TEXT ;
        item.pszText = szBuffer;
        item.cchTextMax = MAXTEXTLEN;
		GetItem(&item);
		if (_tcscmp(szBuffer, szParentName) == 0){
			if(!szItemName)	return hLoop;

			hLoop = GetNextItem(hLoop, TVGN_CHILD);
			do{
				TCHAR szBuffer[MAXTEXTLEN+1];
				TV_ITEM item;
				memset(&item, 0, sizeof(TV_ITEM));

				item.hItem = hLoop;
				item.mask = TVIF_TEXT ;
				item.pszText = szBuffer;
				item.cchTextMax = MAXTEXTLEN;
				GetItem(&item);
				if (_tcscmp(szBuffer, szItemName) == 0){
					return hLoop;
				}


				hLoop = GetNextItem(hLoop, TVGN_NEXT);
			}while (hLoop!=NULL);
		}
    }while (hLoop!=NULL);
	 return NULL;

/*

    // If hItem is NULL, start search from root item.
    if (hItem == NULL)
        hItem = (HTREEITEM)SendMessage(hWnd, TVM_GETNEXTITEM,
                                       TVGN_ROOT, 0);
    while (hItem != NULL)
    {
        TCHAR szBuffer[MAXTEXTLEN+1];
        TV_ITEM item;
		memset(&item, 0, sizeof(TV_ITEM));

        item.hItem = hItem;
        item.mask = TVIF_TEXT | TVIF_CHILDREN;
        item.pszText = szBuffer;
        item.cchTextMax = MAXTEXTLEN;
        SendMessage(hWnd, TVM_GETITEM, 0, (LPARAM)&item);

		
        // Did we find it?
        if (lstrcmp(szBuffer, szItemName) == 0)
            return hItem;

        // Check whether we have child items.
        if ((item.cChildren != I_CHILDRENCALLBACK)  && (item.cChildren != 0))
        {
            // Recursively traverse child items.
            HTREEITEM hItemFound, hItemChild;

            hItemChild = (HTREEITEM)SendMessage(hWnd, TVM_GETNEXTITEM,
                                                TVGN_CHILD, (LPARAM)hItem);
			hItemFound = ::GetItemByName(hWnd, hItemChild, szItemName);

            // Did we find it?
            if (hItemFound != NULL)
                return hItemFound;
        }

        // Go to next sibling item.
        hItem = (HTREEITEM)SendMessage(hWnd, TVM_GETNEXTITEM,
                                       TVGN_NEXT, (LPARAM)hItem);
    }

    // Not found.
    return NULL;
*/
}


void CZoneTreeCtrl::FireOnEvent(DWORD uiEvent, DWORD uiContext)
{
	if  ( !IsWindow(m_hWnd) ) return;
	if ( m_bEditing && uiEvent != VCA_COUNTEROUT_UPDATE ){
		FinishEditingItem(m_hSelect);
		m_bEditing = FALSE;
	}

	m_pDataMgr->GetSelectAreaIdx(&m_iCurArea, &m_eCurAreaType);

	if (( VCA_COUNTEROUT_UPDATE  == uiEvent)) {// Only update counting result do not need to redraw zonetree
		VCA5_APP_COUNTER *pCounter = m_pDataMgr->GetCounter(m_iCurArea);
		if (pCounter) {
			VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * )GetUserData(m_hRoot);			
			if((TREELIST_COUNTER == m_iTreeType)&&(pLine->pItem[0].pData == pCounter) ){
				CString s;
				s.Format( _T("%d"), pCounter->iCounterResult );
				SetItem( m_hRoot, 1, TVIF_TEXT, s, 0,0,0,0,0);
			}
		}
	}else if(uiEvent == VCA_OBJCLS_UPDATE || uiEvent == VCA_LOAD){
		BuildObjclsFilter();
		RedrawAllZoneListTree();
	}else if(IVCAConfigureObserver::IsZoneEvent(uiEvent)) {
		VCA5_APP_ZONE *pZone = m_pDataMgr->GetZone( m_iCurArea );
		if ( pZone && uiEvent == VCA_ZONE_DEL ) {
			DeleteObjclsFilter(pZone->usZoneId);
		}
		RedrawZoneTree( m_pDataMgr->GetZone( m_iCurArea ) );
	}else if(IVCAConfigureObserver::IsCounterEvent(uiEvent) ) {
		RedrawCounterTree( m_pDataMgr->GetCounter( m_iCurArea ) );
	}else if(IVCAConfigureObserver::IsRuleEvent(uiEvent) ){
		VCA5_APP_ZONE *pZone = m_pDataMgr->GetZone(m_iCurArea);
		if(pZone){
			HTREEITEM hTreeItem;
			TCHAR	strTemp[128];
			TCHAR	*szParent,*szItem;

			VCA5_APP_RULE *pRule = m_pDataMgr->GetRule(m_iCurArea, VCA5_RULE_TYPE_DIRECTION);
			if(pRule){	//Update Direction filter value
				VCA_TREELIST_LINE_T *pLine = ( VCA_TREELIST_LINE_T * )GetUserData(m_hRoot);			
				if((TREELIST_ZONE == m_iTreeType)&&(pLine->pItem[0].pData == pZone) ){
					VCA5_RULE_DIRECTION* pDirection = &(pRule->tRuleDataEx.tDirection);
					int direction;
					
					szParent	= (TCHAR *)strTreeItem[VCA5_RULE_TYPE_DIRECTION-1][0];
					szItem		= (TCHAR *)strTreeItem[VCA5_RULE_TYPE_DIRECTION-1][1];
					hTreeItem = GetItemByName(szParent, szItem);
					if(hTreeItem){
						direction = (pDirection->sStartAngle+pDirection->sFinishAngle) /2 / 10;
						direction = 90 - direction;
						ValidateSingleAngle( &direction );
						_stprintf_s(strTemp, _T("%d°"), direction );
						SetItemText(hTreeItem, strTemp, 1);
					}

					szItem		= (TCHAR *)strTreeItem[VCA5_RULE_TYPE_DIRECTION-1][2];
					hTreeItem = GetItemByName(szParent, szItem);
					if(hTreeItem){
						_stprintf_s(strTemp, _T("%d°"), (pDirection->sFinishAngle-pDirection->sStartAngle)  / 10 );
						SetItemText(hTreeItem, strTemp, 1);
					}
				}
			}

			pRule = m_pDataMgr->GetRule(m_iCurArea, VCA5_RULE_TYPE_LINECOUNTER_A);
			if(!pRule){
				pRule = m_pDataMgr->GetRule(m_iCurArea, VCA5_RULE_TYPE_LINECOUNTER_B);
			}

			if(pRule){	//Update LineCounter filter value
				szParent	= (TCHAR *)strTreeItem[VCA5_RULE_TYPE_LINECOUNTER_B-1][3];
				szItem		= (TCHAR *)strTreeItem[VCA5_RULE_TYPE_LINECOUNTER_B-1][2];

				hTreeItem = GetItemByName(szParent, szItem);
				if(hTreeItem){
					int calibWidth = pRule->tRuleDataEx.tLineCounter.ulCalibrationWidth;
					_stprintf_s(strTemp, _T("%3.2f"), 100*(float)calibWidth/65535);
					SetItemText(hTreeItem, strTemp, 1);
				}
			}
		}
	}else if (NOT_IN_USE == m_iCurArea) {
		RedrawAllZoneListTree();
	}
}

void CZoneTreeCtrl::BuildObjclsFilter( )
{
	int		i,j;
	//initialize Object class Filter
	memset(m_pObjClsFilters, 0, sizeof(m_pObjClsFilters));
	for ( i = 0; i < VCA5_MAX_NUM_ZONES; i++ )
	{
		VCA5_APP_ZONE *pZone = m_pDataMgr->GetZone( i );
		int iTrkObjs = 0;
		OBJCLS_FILTERS_T *pObjFilters;

		if ( pZone )
		{
			pObjFilters = GetObjclsFilter(pZone->usZoneId);	
		pObjFilters->iTotalTrkObjs = 0;
		pObjFilters->ucExpanded = 0;

			BOOL bFound = FALSE;

			for ( j = 0; j < VCA5_RULE_TYPE_NUM; j++ )
			{
				VCA5_APP_RULE *pRule = m_pDataMgr->GetRule(i, (VCA5_RULE_TYPE_E) j);
				if ( pRule && pRule->ucTicked ) {
					bFound = TRUE;
					break;
				}
			}

			if ( bFound )
			{
				VCA5_APP_RULE *pRule = m_pDataMgr->GetRule(i, (VCA5_RULE_TYPE_E) j);
				pObjFilters->ucLogic = GET_TRKOBJ_LOGIC(pRule->ucTrkObjs[0]);
				for ( j = 0; j < pRule->usTotalTrkObjs; j++ )
				{
					if ( pObjFilters->ucLogic == GET_TRKOBJ_LOGIC(pRule->ucTrkObjs[j]) )
					{
						pObjFilters->tObjclsFilter[iTrkObjs].cId = GET_TRKOBJ_ID(pRule->ucTrkObjs[j]);
						pObjFilters->tObjclsFilter[iTrkObjs].ucTicked = 1;	
						iTrkObjs++;
					}
				}
			}
			
		pObjFilters->tObjclsFilter[iTrkObjs].cId = VALUE_NA;
		pObjFilters->tObjclsFilter[iTrkObjs].ucTicked = 0;
		pObjFilters->tObjclsFilter[iTrkObjs].ucIsNew = 1;
		iTrkObjs++;
			pObjFilters->iTotalTrkObjs = iTrkObjs;
		}

	}

	ApplyObjclsFilter();
}

void CZoneTreeCtrl::ApplyObjclsFilter( )
{
	ULONG i, j;
	int k;

	VCA5_APP_ZONES	*pZones = m_pDataMgr->GetZones();
	VCA5_APP_RULE	*pRule;

	for ( i = 0; i < pZones->ulTotalZones; i++ ) {
		if (pZones->pZones[i].uiStatus & VCA5_APP_AREA_STATUS_NOT_USED) {
			continue;
		}

		for ( j = 0; j < VCA5_RULE_TYPE_NUM; j++ ) {
			pRule = m_pDataMgr->GetRule(i, (VCA5_RULE_TYPE_E) j);
			//if (pRule && (pRule->uiStatus & VCA5_APP_RULE_STATUS_TICKED)) {
			if (pRule) {
				pRule->usTotalTrkObjs = 0;
				memset(pRule->ucTrkObjs, 0, sizeof(pRule->ucTrkObjs));
				//OBJCLS_FILTERS_T *pObjFilter = &m_pObjClsFilters[i];
				OBJCLS_FILTERS_T *pObjFilter = GetObjclsFilter(pZones->pZones[i].usZoneId);
				if ( pObjFilter->iTotalTrkObjs > 1 ){
					int		iTrkObjs = 0;
					for ( k = 0; k < pObjFilter->iTotalTrkObjs; k++ ){
						if ( pObjFilter->tObjclsFilter[k].ucTicked ){
							pRule->ucTrkObjs[iTrkObjs++] = MAKE_TRKOBJ( pObjFilter->ucLogic, pObjFilter->tObjclsFilter[k].cId );
						}
					}
					pRule->usTotalTrkObjs = iTrkObjs;
				}
			}
		}
	}
}

OBJCLS_FILTERS_T *CZoneTreeCtrl::GetObjclsFilter(USHORT usZoneId)
{
	// 1. get associated OBJCLS_FILTERS_T
	int i, j = -1;
	for (i = 0; i < (int)(sizeof(m_pObjClsFilters)/sizeof(m_pObjClsFilters[0])); i++) {
		if (m_pObjClsFilters[i].usZoneId==usZoneId && m_pObjClsFilters[i].ucValid) {
			return &m_pObjClsFilters[i];
		} else if (!m_pObjClsFilters[i].ucValid) {
			j = i;
		}
	}
	
	// 2. get empty OBJCLS_FILTERS_T
	if (j!=-1) {
		m_pObjClsFilters[j].ucValid = 1;
		m_pObjClsFilters[j].usZoneId = usZoneId;
		m_pObjClsFilters[j].iTotalTrkObjs = 1;
		m_pObjClsFilters[j].tObjclsFilter[0].cId = VALUE_NA;
		m_pObjClsFilters[j].tObjclsFilter[0].ucIsNew = 1;
		m_pObjClsFilters[j].tObjclsFilter[0].ucTicked = 0;
		return &m_pObjClsFilters[j];
	}

	ASSERT(0);
	return NULL;
}

void CZoneTreeCtrl::DeleteObjclsFilter(USHORT usZoneId)
{
	DWORD i;
	for (i = 0; i < (int)(sizeof(m_pObjClsFilters)/sizeof(m_pObjClsFilters[0])); i++) {
		if (m_pObjClsFilters[i].usZoneId==usZoneId && m_pObjClsFilters[i].ucValid) {
			m_pObjClsFilters[i].ucValid = 0;
			m_pObjClsFilters[i].iTotalTrkObjs = 0;
		}
	}
}

BOOL CZoneTreeCtrl::ValidateName( char *pszName )
{
	int i, index;
	int len = strlen( pszName );
	char szTmp[256] = "\0";
	BOOL bRet = TRUE;

	for ( i = 0, index = 0; i < len; i++ )
	{
		if ( strchr( gszFobiddenChar, pszName[i] ) != NULL )
			bRet = FALSE;
		else
		{
			szTmp[index] = pszName[i];
			index++;
		}
	}

	szTmp[index] = 0;
	strcpy( pszName, szTmp );
	if ( strlen( pszName ) == 0 )	strcpy( pszName, " " );
		
	return bRet;
}