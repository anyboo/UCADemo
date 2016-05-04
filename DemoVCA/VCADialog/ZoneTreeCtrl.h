#pragma once

#include "VCAConfigure.h"
#include "TreeListCtrl.h"
#include "VCADataMgr.h"
#include "VCAConfigureObserver.h"

#define TREELIST_ZONE		0x01
#define TREELIST_COUNTER	0x02
#define TREELIST_SHOWALL	0x04
#define TREELIST_NULL		0x08

#define MAX_NUM_COLUMNS		3
#define MAX_NUM_LINES		50
#define MAX_NUM_COMBOITEMS	( VCA5_MAX_NUM_ZONES * (VCA5_RULE_TYPE_RMOBJ - VCA5_RULE_TYPE_PRESENCE + 1) )

#define NOT_IN_USE	-1
#define NUM_ICONS					23
#define NUM_PREDEFINED_OBJICONS		4

typedef struct
{
	UINT		uICON;
	LPCTSTR		strName;
}
OBJCLS_ICON_T;


enum {
	VCA5_TRIG_RULE,
	VCA5_TRIG_ZONE
};

// For save all the item data of current treelist
typedef enum
{
	fStatic		= 0x0001,
	fEdit		= 0x0002,
	fCombo		= 0x0004,
	fTickBox	= 0x0010,
	fExpand		= 0x0100,
	fData		= 0x1000
}
VCA_TREE_ITEM_E;

typedef enum
{
	// for fCombo
	cZoneColor	= 0x0001,
	cZoneType,
	cZoneObjclsId,
	cZoneObjclsLogic,
#ifdef TMP_ADVANCED_RULE
	cZoneLogic,
	cZoneMultiWire,
#endif
	cCounterColor,
	cCounterInc,
	cCounterDec,
	cCounterIns,
	cTailgatingMode,
	cColBin,
// for fEdit
	edZoneName,
	edCounterName,
	edSpeedLow,
	edSpeedUp,
	edDirection,
	edAcceptance,
	edTimeThreshold,
	edLineCounter,
	edEventName,
	edThres,
}
VCA_TREE_ITEM_EDITTYPE_E;


enum
{
	STATEIMAGE_CHECKBOX_NONE = -1,
	STATEIMAGE_CHECKBOX_UNCHECKED = 0,
	STATEIMAGE_CHECKBOX_CHECKED = 1,
	STATEIMAGE_CHECKBOX_DISABLE = 3,
};
/*
typedef struct
{
	unsigned short	usFlags;
	unsigned short	usFlagsEx; // only valid when fCombo is set in usFlags
	void *			pData;
}
VCA_TREELIST_ITEM_T;

typedef struct
{
	HTREEITEM			hParent;
	unsigned short		usNumColumns;
	VCA_TREELIST_ITEM_T pItem[MAX_NUM_COLUMNS];
}
VCA_TREELIST_LINE_T;
*/

typedef struct
{
	unsigned short				usFlags;	// VCA_TREE_ITEM_E
	VCA_TREE_ITEM_EDITTYPE_E	eEditType;	// Only valid when usFlags&fCombo > 0 or usFlags&fEdit > 0
	void*						pData;		// Only valid when usFlags&fData > 0
	void*						pData2;		// Only valid when usFlags&fData > 0
	unsigned char*				pucExpanded;// Only valid when usFlags&fExpand > 0
	unsigned char*				pucTicked;	// Only valid when usFlags&fTickBox > 0
}
VCA_TREELIST_ITEM_T;

typedef struct
{
//	HTREEITEM			hParent;
	VCA_OP_E			eType;
	unsigned short		usCanEditRuleName;
	unsigned short		usNumColumns;
	VCA_TREELIST_ITEM_T pItem[MAX_NUM_COLUMNS];
}
VCA_TREELIST_LINE_T;


typedef struct
{
	HTREEITEM			hRoot;
	unsigned short		usNumLines;
	VCA_TREELIST_ITEM_T pItem[MAX_NUM_LINES];
}
VCA_TREELIST_T;

// For the combo box when setting up counter
typedef struct
{
	TCHAR*			pszComboString;
	COLORREF		crColour;
	union {
		struct {
			unsigned short	usType;
			unsigned short	usTrigId;
		};
		UINT			uiColor;
		USHORT			usZoneType;
		SHORT			sObjClsId;
		UCHAR			ucObjClsLogic;
		USHORT			usMode;
		USHORT			usColBin;
	};
}

TREELIST_COMBO_ITEM_T;

typedef struct
{
	unsigned short	uiNumComboItems;
	TREELIST_COMBO_ITEM_T	pComboItem[VCA5_MAX_NUM_ZONES*VCA5_MAX_NUM_RULES];
}
TREELIST_COMBO_T;

typedef struct
{
	SHORT			cId;
	unsigned char	ucTicked;
	unsigned char	ucIsNew;
}
OBJCLS_FILTER_T;

typedef struct
{
	short int		iTotalTrkObjs;
	unsigned char	ucLogic;
	OBJCLS_FILTER_T tObjclsFilter[VCA5_MAX_NUM_CLSOBJECTS+2];
	unsigned char	ucExpanded;
	unsigned char	ucValid;
	USHORT			usZoneId;
}
OBJCLS_FILTERS_T;

class CZoneTreeCtrl : 	public CTreeListCtrl, public IVCAConfigureObserver
{
public:
	enum {MAX_TREECTRL_WIDTH = 250};
	CZoneTreeCtrl(void);
	virtual ~CZoneTreeCtrl(void);
	
	BOOL Setup(CWnd* pWnd, CVCADataMgr *pDataMgr);
	void Destroy();

	void SetDirty()			{  m_bImageChanged = TRUE; };
	void SetRoot( );
	void OnSelChanged		( NMHDR *pNmHdr, LRESULT *pResult );
	void OnCbStateChanged	( NMHDR *pNmHdr, LRESULT *pResult );
	void OnItemlClick		( NMHDR *pNmHdr, LRESULT *pResult,HWND hWnd );
	void OnItemDblClick		( NMHDR *pNmHdr, LRESULT *pResult,HWND hWnd );
	void OnBeginLabelEdit	( NMHDR *pNmHdr, LRESULT *pResult );
	void OnEndLabelEdit		( NMHDR *pNmHdr, LRESULT *pResult );
	void OnItemExpanding	( NMHDR *pNmHdr, LRESULT *pResult );

	void SetItemTick( HTREEITEM hItem, unsigned char ucTick );
	void SetItemExpand( HTREEITEM hItem, unsigned char ucExpand );
	void RedrawAllZoneListTree( );

	virtual BOOL IsDirty();
	void FireOnEvent(DWORD uiEventType, DWORD uiContext);

protected:
	
	//void RedrawSettingsTree( VCA5_APP_ZONE *pZone );
	void RedrawZoneTree( VCA5_APP_ZONE *pZone );
	void RedrawCounterTree( VCA5_APP_COUNTER *pCounter );

/************************************** For Zones Tree ************************************/
// Zone functions
	void RemoveZoneFromTree(VCA5_APP_ZONE *pZone);
	void FindMatchZoneInTree(VCA5_APP_ZONE *pZone);
	HTREEITEM GetItemByName(TCHAR* szParentName, TCHAR* szItemName);

// Event functions
	void AddRuleToZone( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule);
	void Add_ObjectFilter_Tree( VCA5_APP_ZONE *pZone );
	void Add_Stock_BASICT_Tree  ( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule );
	void Add_Stock_DIRECTIONT_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule );
	void Add_Stock_TIMERT_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule );
	void Add_Stock_SPEED_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule );
	void Add_Stock_LINECOUNTER_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule );
	void Add_Stock_COLSIG_Tree( VCA5_APP_ZONE *pZone, VCA5_APP_RULE *pRule );
	void AddPredefinedZoneRules(VCA5_APP_ZONE *pZone);
	int  CheckZoneObjCls( OBJCLS_FILTER_T *pOF );
	void SetCurrentStatus( int iRuleId, int iRuleOption );
	void ValidateAngle(	VCA5_RULE_DIRECTION *pDT );
	void ValidateSingleAngle( int *pAngle );
	int  ValidTailgatingRule(VCA5_APP_RULE *pRule, unsigned short usZoneId );
	void ValidateSpeed( VCA5_RULE_SPEED *pSF);

/************************************** For Counter Tree ************************************/
	void AddCounterTree( VCA5_APP_COUNTER *pCounter, HTREEITEM hParent );
	void AddCounterIncrement( VCA5_APP_COUNTER *pCounter, HTREEITEM hParent, BOOL bEnable=TRUE);
	void AddCounterDecrement( VCA5_APP_COUNTER *pCounter, HTREEITEM hParent, BOOL bEnable=TRUE);
	void AddCounterInstant( VCA5_APP_COUNTER *pCounter, HTREEITEM hParent, BOOL bEnable=TRUE);

	void BuildCounterComboBoxByRule( VCA5_APP_COUNTER *pCounter );
	TCHAR* BuildColorBinCombo( VCA5_APP_RULE *pRule );
	void BuildZoneColorCombo();
	TCHAR* BuildTailgatingModeCombo( VCA5_APP_RULE *pRule );
	TCHAR *BuildZoneTypeCombo( VCA5_APP_ZONE *pZone );
	void BuildObjclsLogicCombo( );
	void BuildObjclsIdCombo();

	void ActiveCombo( HTREEITEM hItem, TREELIST_COMBO_T *pComboTree );
	BOOL SetSubCounter( VCA5_SUBCOUNTER *pData, TCHAR* pText, HTREEITEM hParent );
	BOOL SetZoneColor( DWORD *puiColor, TCHAR* pText, HTREEITEM hParent );
	BOOL SetZoneType( USHORT *pusZoneType, TCHAR* pText, HTREEITEM hParent );
	BOOL SetZoneObjclsId( SHORT *pcId, TCHAR* pText, HTREEITEM hParent );
	BOOL SetZoneObjclsLogic( unsigned char *pucLogic, TCHAR* pText, HTREEITEM hParent );
	int SetColorBin( unsigned short *pusColBin, TCHAR* pText, HTREEITEM hParent );
	BOOL AddZoneObjCls( OBJCLS_FILTER_T *pOF );
	int	SetTailgatingMode( unsigned short *pusMode, TCHAR* pText, HTREEITEM hParent );

	BOOL ValidateName( char *pszName );
	void InsertUserData( HTREEITEM hItem, VCA_TREELIST_LINE_T *pTreeLine );

	LPCTSTR	GetColorName(DWORD uiColor);

	void BuildObjclsFilter();
	void ApplyObjclsFilter();
	OBJCLS_FILTERS_T *GetObjclsFilter(USHORT usZoneId);
	void DeleteObjclsFilter(USHORT usZoneId);
protected:
	HTREEITEM		m_hRoot;
	HTREEITEM		m_hSelect;
	int				m_iTreeType;
	int				m_iSelCol;
	HICON			m_hIcon;
	unsigned		m_uSelState;
	int				m_iCurArea;
	VCA5_APP_AREA_T_E	m_eCurAreaType;
	int				m_iPreRule;
	int				m_iCurRule;
	int				m_iPreOption;
	int				m_iCurOption;
	BOOL			m_bImageChanged;
	BOOL			m_bEditing;
	BOOL			m_bObjectFilter;
	LPTSTR			m_lpszBufferEdit;
	ULONG			m_uEngFunction;

	TREELIST_COMBO_T	m_tRuleCounterCombo;
	TREELIST_COMBO_T	m_tZoneColorCombo;
	TREELIST_COMBO_T	m_tZoneTypeCombo;
	TREELIST_COMBO_T	m_tColorBinCombo;
	TREELIST_COMBO_T	m_tObjClsIdCombo;
	TREELIST_COMBO_T	m_tObjClsLogicCombo;
	TREELIST_COMBO_T	m_tTailGateModeCombo;


	CWnd			*m_pParentWnd;
	CVCADataMgr		*m_pDataMgr;
	CImageList		m_ImageList;

	OBJCLS_FILTERS_T	m_pObjClsFilters[VCA5_MAX_NUM_ZONES];
};

static unsigned short usINC = VCA5_COUNTER_INCREMENT;
static unsigned short usDEC = VCA5_COUNTER_DECREMENT;
static unsigned short usINS = VCA5_COUNTER_INSTANT;
