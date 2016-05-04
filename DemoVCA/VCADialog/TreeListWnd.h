/*****************************************************************************
//*
//*		
//*		TreeListWnd.h
//*
//*
//*****************************************************************************/
#ifndef 	__TREELISTWND_H__
#define 	__TREELISTWND_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include	<commctrl.h>

typedef int (CALLBACK *PFNTVCOMPAREEX)(HWND hWnd,HTREEITEM hItem1,HTREEITEM hItem2,LPARAM lParam1,LPARAM lParam2,LPARAM lParam);

typedef struct  
	{ 
    HTREEITEM       hParent;
    PFNTVCOMPAREEX  lpfnCompare;
    LPARAM          lParam;
	}TVSORTEX; 

typedef struct  
	{ 
    UINT	mask; 
    int		fmt; 
    int		cx; 
    LPTSTR	pszText; 
    int		cchTextMax; 
    int		iImage;
	}TVCOLUMN; 




typedef		TVSORTEX			   *LPTVSORTEX;
typedef		TVSORTEX			   *LPTV_SORTEX;
typedef		TVSORTEX				TV_SORTEX;


#define 	TV_COLUMN				LV_COLUMN
#define 	TV_NOIMAGE				(-2)
#define 	TV_NOCOLOR				0xFFFFFFFF
#define 	TVIF_TEXTPTR			0x80000000
#define 	TVIF_TOOLTIPTIME		0x40000000
#define		TVIF_SUBITEM	        0x8000
#define 	TVIF_SUBNUMBER			0x4000
#define 	TVIS_UNTERLINE			0x0001
//bincheng	
#define		TVIS_DISABLED			0x0100
#define		TVIF_USECOLORCB			0x2000

#define 	TVSCW_AUTOSIZE			LVSCW_AUTOSIZE
#define		TVN_COMUMNCLICK			HDN_ITEMCLICK 
#define 	TVN_COMUMNDBLCLICK		HDN_ITEMDBLCLICK
#define 	TVI_BEFORE				((HTREEITEM) 0xFFFF0006)
#define 	TVI_AFTER				((HTREEITEM) 0xFFFF0005)
#define		TVI_ROW(n)		        ((HTREEITEM)(0xFFE00000+(n)))


// Konstanten f? Farben (TVM_SETBKCOLOR)
#define 	TVC_BK					0					// Hintergrundfarbe  
#define 	TVC_ODD					1					// Abwechselnte Farbe    (siehe TVS_EX_ALTERNATECOLOR)
#define 	TVC_EVEN				2					// Farbe von Eintrag	 (TVS_EX_ITEMLINES+TVS_EX_ALTERNATECOLOR)
#define 	TVC_FRAME				3					// Farbe f? Trennlinien (siehe TVS_EX_ITEMLINES)
#define 	TVC_TEXT				4					// Textfarbe
#define 	TVC_LINE				5					// Innere Farbe der Schaltfl?hen
#define 	TVC_BOX					6					// ??re Farbe der Schaltfl?hen
#define 	TVC_TRACK				7					// Farbe f? getrakte Texte
#define 	TVC_MARK				8					// Markierte Zeile
#define 	TVC_MARKODD				8					// Markierte Zeile (ungerade)
#define 	TVC_MARKEVEN			9					// Markierte Zeile (gerade)	
#define 	TVC_INSERT				10					// Farbe der Einf?emarke


// Konstanten f? GetNextItem (TVM_GETNEXTITEM)
#define 	TVGN_DROPHILITESUB		0x000C				// Ausgew?lte Spalte holen
#define 	TVGN_CARETSUB			0x000D				// Drophighilite Spalte holen
#ifndef		TVGN_NEXTSELECTED
#define 	TVGN_NEXTSELECTED		0x000E				// N?hster ausgew?lter Eintrag
#endif 	
#define 	TVGN_FOCUS				0x000F				// Eintrag der den Focus hat
#define 	TVGN_FOCUSSUB			0x0010				// Spalte  die den Focus hat
#define 	TVGN_NEXTSELCHILD		0x0011				// N?hster ausgew?lter Kindeintrag


// Konstanten f? InsertColumn (mask)
#define 	TVCF_FMT				LVCF_FMT			// Die Textausrichting einstellen
#define 	TVCF_IMAGE				LVCF_IMAGE			// Ein Icon der Spalte zu ortenen
#define 	TVCF_TEXT				LVCF_TEXT			// Den Text der Spalte setzen
#define 	TVCF_WIDTH				LVCF_WIDTH			// Fixe Breite festlegen
#define 	TVCF_VWIDTH				LVCF_ORDER			// Variable Breit festlegen

// Konstanten f? ColumnAutoEdit 
#define 	TVAE_NONE				0x0000				// Kein atomatisches editieren
#define 	TVAE_EDIT				0x0001				// Atomatisches editieren mit Edit-Fenster
#define 	TVAE_COMBO				0x0002				// Atomatisches editieren mit ComboBox
#define 	TVAE_CBLIST				0x0003				// Atomatisches editieren mit ComboListBox
#define 	TVAE_STEP				0x0004				// Einzelnes Wieterschalten mit Enter
#define 	TVAE_DBLCLICK			0x0010				// Editieren mit Doppelklick
#define 	TVAE_FULLWIDTH			0x0020				// Combobox ?er die volle Breite
#define 	TVAE_PTRLIST			0x0040				// Die Datenliste wird als Zeigerliste ?ergeben 
#define 	TVAE_ONLYRETURN			0x0080				// Das Editeren kann nur mit Return begonnen werden
#define 	TVAE_COL(c)				(((c)&0xFF)<< 8)	// Ist die Spalte f?s atomatische Editieren
#define 	TVAE_CHAR(c)			(((c)&0xFF)<<16)	// Ist das Trennzeichen f? die Datenliste
#define 	TVAE_COUNT(c)			(((c)&0xFF)<<24)	// Bestimmt die Anzahl der Eintr?e in der Datenliste (0=auto)

// Konstanten f? HitTest (flags)
#define 	TVHT_SUBTOCOL(u)		(((unsigned)u)>>24)	// Umwandeln von Spaltemasken zu Spaltenummern						
#define 	TVHT_SUBMASK			0xFF000000			// Maske in der die Spalte gespeichert wird	
#define 	TVHT_ONRIGHTSPACE		0x00800000			// Auf rechtem Rand nach den Extr?en
#define 	TVHT_ONSUBLABEL			0x00400000			// Koortinate ist auf dem Text eines Extraeintrages
#define 	TVHT_ONSUBICON			0x00200000			// Koortinate ist auf einem Extraeintrag
#define 	TVHT_ONSUBRIGHT			0x00100000			// Koortinate ist auf einem Extraeintrag rechts vom Text
#define 	TVHT_ONSUBITEM			(TVHT_ONSUBICON|TVHT_ONSUBLABEL)


// Konstanten f? GetItemRect (TVM_GETITEMRECT)
#define 	TVIR_COLTOSUB(u)		((u)<<24)	
#define 	TVIR_GETCOLUMN			0x00000080
#define 	TVIR_TEXT				0x00000001


// Konstanten f? EditLabel (TVM_GETITEMRECT)
#define 	TVIR_EDITCOMBOBOX		0x20000000			// Statt dem Edit-Fenster eine ComboBox einblenden	
#define 	TVIR_EDITCOMBOLIST		0x40000000			// Statt dem Edit-Fenster eine ComboBox nur mit Listenauswahl einblenden
#define 	TVIR_EDITFULL			0x80000000			// Das Edit-Fenster ?er die volle Breite einblenden


// Konstanten f? InsertColumn (mask)
#define 	TVCFMT_BITMAP_ON_RIGHT	LVCFMT_BITMAP_ON_RIGHT
#define 	TVCFMT_COL_HAS_IMAGES	LVCFMT_COL_HAS_IMAGES
#define 	TVCFMT_CENTER			LVCFMT_CENTER
#define 	TVCFMT_IMAGE			LVCFMT_IMAGE
#define 	TVCFMT_LEFT				LVCFMT_LEFT
#define 	TVCFMT_RIGHT			LVCFMT_RIGHT

// Konstanten f? uStyleEx
#define 	TVS_EX_BITCHECKBOX		0x00008000			// Bei Checkboxen nur das unterse Bit ?dern 
#define 	TVS_EX_ITEMLINES		0x00010000			// Um die Eintr?e Linien zeichnen
#define 	TVS_EX_ALTERNATECOLOR	0x00020000			// Die Farbe abwechselnd darstellen
#define 	TVS_EX_SUBSELECT		0x00040000			// Sub-Eitr?e seperat ausw?len
#define 	TVS_EX_FULLROWMARK		0x00080000			// Die ganze Zeile markieren wenn selektiert
#define 	TVS_EX_TOOLTIPNOTIFY	0x00100000			// Spezielle Notify-Abfrage bei Tooltips
#ifndef		TVS_EX_MULTISELECT
#define 	TVS_EX_MULTISELECT		0x00001000			// Eine Mehrfachauswahl erlauben
#endif 	
#ifndef		TVS_EX_AUTOHSCROLL 
#define 	TVS_EX_AUTOHSCROLL 		0x00002000			// Horizontale Scrollbar ausblenden
#endif 	

#define		TVN_ITEMTOOLTIP			(TVN_FIRST-32)		// Notify Nachricht f? Tooltip-Fenster 
#define     TVN_CBSTATECHANGED      (TVN_FIRST-33)		// Notify Nachricht f? ge?derted Checkbox

// Konstanten f? neue Nachrichten
#define 	TVM_GETHEADER			(TV_FIRST+92-1)
#define 	TVM_GETCOLUMNCOUNT		(TV_FIRST+92-2)
#define 	TVM_DELETECOLUMN		(TV_FIRST+92-3)
#define 	TVM_INSERTCOLUMN		(TV_FIRST+92-4)
#define 	TVM_SELECTSUBITEM		(TV_FIRST+92-5)
#define 	TVM_SELECTDROP			(TV_FIRST+92-6)
#define 	TVM_SETITEMBKCOLOR		(TV_FIRST+92-7)
#define 	TVM_GETITEMBKCOLOR		(TV_FIRST+92-8)
#define 	TVM_SETITEMTEXTCOLOR	(TV_FIRST+92-9)
#define 	TVM_GETITEMTEXTCOLOR	(TV_FIRST+92-10)
#define 	TVM_GETITEMOFROW		(TV_FIRST+92-11)
#define 	TVM_GETROWCOUNT			(TV_FIRST+92-12)
#define 	TVM_GETROWOFITEM		(TV_FIRST+92-13)	
#define 	TVM_SETCOLUMN			(TV_FIRST+92-14)
#define 	TVM_GETCOLUMN			(TV_FIRST+92-15)
#define 	TVM_SETCOLUMNWIDTH		(TV_FIRST+92-16)
#define 	TVM_GETCOLUMNWIDTH		(TV_FIRST+92-17)
#define 	TVM_SETUSERDATASIZE		(TV_FIRST+92-18)
#define 	TVM_GETUSERDATASIZE		(TV_FIRST+92-19)
#define 	TVM_GETUSERDATA			(TV_FIRST+92-20)
#define 	TVM_SORTCHILDRENEX		(TV_FIRST+92-21)
#define 	TVM_COLUMNAUTOEDIT		(TV_FIRST+92-22)
#define 	TVM_COLUMNAUTOICON		(TV_FIRST+92-23)
#ifndef		TVM_GETITEMSTATE
#define 	TVM_GETITEMSTATE		(TV_FIRST+39)
#endif 	
#ifndef		TVM_GETEXTENDEDSTYLE
#define 	TVM_GETEXTENDEDSTYLE	(TV_FIRST+45)
#endif 	
#ifndef		TVM_SETEXTENDEDSTYLE
#define 	TVM_SETEXTENDEDSTYLE	(TV_FIRST+44)
#endif 	
#ifndef		TVM_GETLINECOLOR
#define 	TVM_GETLINECOLOR		(TV_FIRST+41)
#endif 	
#ifndef		TVM_SETLINECOLOR
#define 	TVM_SETLINECOLOR		(TV_FIRST+40)
#endif 	


#ifndef		TVNRET_DEFAULT 
#define		TVNRET_DEFAULT			0
#endif 	
#ifndef		TVNRET_SKIPOLD 
#define		TVNRET_SKIPOLD			1
#endif 	
#ifndef		TVNRET_SKIPNEW 
#define		TVNRET_SKIPNEW			2
#endif 	

#define 	TreeList_DeleteAllItems(h)					((BOOL      )::SendMessage(h,TVM_DELETEITEM,0,TVI_ROOT))
#define		TreeList_DeleteItem(h,i)					((BOOL      )::SendMessage(h,TVM_DELETEITEM,0,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_Expand(h,i,c)						((BOOL      )::SendMessage(h,TVM_EXPAND,(WPARAM)(c),(LPARAM)(HTREEITEM)(i)))
#define		TreeList_GetHeader(h)						((HWND      )::SendMessage(h,TVM_GETHEADER,0,0))
#define		TreeList_DeleteColumn(h,i)     				((BOOL      )::SendMessage(h,TVM_DELETECOLUMN,(WPARAM)(int)(i),0))
#define		TreeList_InsertColumn(h,i,p)				((INT       )::SendMessage(h,TVM_INSERTCOLUMN,(WPARAM)(int)(i),(LPARAM)(const TV_COLUMN*)(p)))
#define 	TreeList_GetColumnCount(h)					((INT       )::SendMessage(h,TVM_GETCOLUMNCOUNT,0,0))
#define 	TreeList_HitTest(h,p)						((HTREEITEM )::SendMessage(h,TVM_HITTEST,0,(LPARAM)(LPTV_HITTESTINFO)(p)))
#define 	TreeList_GetItemOfRow(h,r)					((HTREEITEM )::SendMessage(h,TVM_GETITEMOFROW,0,r))
#define 	TreeList_GetRowOfItem(h,i)					((INT       )::SendMessage(h,TVM_GETROWOFITEM,0,(LPARAM)(i)))
#define 	TreeList_GetRowCount(h)						((INT       )::SendMessage(h,TVM_GETROWCOUNT ,0,0))
#define		TreeList_GetExtendedStyle(h)				((DWORD     )::SendMessage(h,TVM_GETEXTENDEDSTYLE,0,0))
#define		TreeList_SetExtendedStyle(h,d)	  			((DWORD     )::SendMessage(h,TVM_SETEXTENDEDSTYLE,0,d))
#define 	TreeList_SetExtendedStyleEx(h,d,m)			((DWORD     )::SendMessage(h,TVM_SETEXTENDEDSTYLE,m,d))
#define		TreeList_GetColor(h,i)						((COLORREF  )::SendMessage(h,TVM_GETBKCOLOR,(WPARAM)(i),0))
#define		TreeList_SetColor(h,i,c) 					((COLORREF  )::SendMessage(h,TVM_SETBKCOLOR,(WPARAM)(i),c))
#define		TreeList_GetItemBkColor(h,i,s)				((COLORREF  )::SendMessage(h,TVM_GETITEMBKCOLOR,(WPARAM)(i),s))
#define		TreeList_SetItemBkColor(h,i,s,c) 			((COLORREF  )::SendMessage(h,TVM_SETITEMBKCOLOR,((UINT)(i))|((s)<<24),c))
#define		TreeList_GetItemTextColor(h,i,s)			((COLORREF  )::SendMessage(h,TVM_GETITEMTEXTCOLOR,(WPARAM)(i),s))
#define		TreeList_SetItemTextColor(h,i,s,c) 			((COLORREF  )::SendMessage(h,TVM_SETITEMTEXTCOLOR,((UINT)(i))|((s)<<24),c))
#define 	TreeList_EnsureVisible(h,i)					((BOOL      )::SendMessage(h,TVM_ENSUREVISIBLE,0,(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_EnsureVisibleEx(h,i,s)				((BOOL      )::SendMessage(h,TVM_ENSUREVISIBLE,s,(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_SelectDropTargetEx(h,i,s)			((BOOL      )::SendMessage(h,TVM_SELECTDROP,(WPARAM)(s),(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_SelectSubItem(h,i,s)				((BOOL      )::SendMessage(h,TVM_SELECTSUBITEM,(WPARAM)(s),(LPARAM)(HTREEITEM)(i)))
#define 	TreeList_Select(h,i,c)						((BOOL      )::SendMessage(h,TVM_SELECTITEM,(WPARAM)(c),(LPARAM)(HTREEITEM)(i)))
#define		TreeList_EditLabel(h,i,s)				    ((HWND      )::SendMessage(h,TVM_EDITLABEL,s,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_EndEditLabelNow(h,c)				((BOOL      )::SendMessage(h,TVM_ENDEDITLABELNOW,(WPARAM) (c),0))
#define		TreeList_GetItem(h,p)						((BOOL      )::SendMessage(h,TVM_GETITEM,0,(LPARAM)(TV_ITEM*)(p)))
#define		TreeList_GetCount							((BOOL      )::SendMessage(h,TVM_GETCOUNT,0,0))
#define		TreeList_GetEditControl(h)					((HWND      )::SendMessage(h,TVM_GETEDITCONTROL,0,0))		
#define		TreeList_GetImageList(h,i)					((HIMAGELIST)::SendMessage(h,TVM_GETIMAGELIST,i,0))
#define		TreeList_GetUserData(h,i)					((LPVOID    )::SendMessage(h,TVM_GETUSERDATA,0,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_GetUserDataSize(h)					((INT       )::SendMessage(h,TVM_GETUSERDATASIZE,0,0))
#define		TreeList_SetUserDataSize(h,s)				((INT       )::SendMessage(h,TVM_SETUSERDATASIZE,0,s))
#define		TreeList_GetIndent							((UINT      )::SendMessage(h,TVM_GETINDENT,0,0))
#define		TreeList_GetVisibleCount					((UINT      )::SendMessage(h,TVM_GETVISIBLECOUNT,0,0))
#define		TreeList_InsertItem(h,p)					((HTREEITEM )::SendMessage(h,TVM_INSERTITEM,0,(LPARAM)(LPTV_INSERTSTRUCT)(p)))
#define		TreeList_CreateDragImage(h,i)				((HIMAGELIST)::SendMessage(h,TVM_CREATEDRAGIMAGE, 0, (LPARAM)(HTREEITEM)(i)))
#define		TreeList_CreateDragImageEx(h,i,s)			((HIMAGELIST)::SendMessage(h,TVM_CREATEDRAGIMAGE, s, (LPARAM)(HTREEITEM)(i)))
#define		TreeList_SetImageList(h,i,l)				((HIMAGELIST)::SendMessage(h,TVM_SETIMAGELIST,i,(LPARAM)(UINT)(HIMAGELIST)(l)))
#define		TreeList_SetIndent(h,i)					    ((BOOL      )::SendMessage(h,TVM_SETINDENT,(WPARAM)(i),0))
#define		TreeList_SetItem(h,p)					    ((BOOL      )::SendMessage(h,TVM_SETITEM,0,(LPARAM)(const TV_ITEM*)(p)))
#define		TreeList_SortChildren(h,i,r)				((BOOL      )::SendMessage(h,TVM_SORTCHILDREN  ,(WPARAM)r,(LPARAM)(HTREEITEM)(i)))
#define		TreeList_SortChildrenCB(h,p,r)				((BOOL      )::SendMessage(h,TVM_SORTCHILDRENCB,(WPARAM)r,(LPARAM)(LPTV_SORTCB)(p)))
#define		TreeList_SortChildrenEX(h,p,r)				((BOOL      )::SendMessage(h,TVM_SORTCHILDRENEX,(WPARAM)r,(LPARAM)(LPTV_SORTEX)(p)))
#define		TreeList_SetColumn(h,i,p)					((BOOL      )::SendMessage(h,TVM_SETCOLUMN,i,(LPARAM)(const TV_COLUMN*)(p)))
#define		TreeList_GetColumn(h,i,p)					((BOOL      )::SendMessage(h,TVM_GETCOLUMN,i,(LPARAM)(TV_COLUMN*)(p)))
#define		TreeList_SetColumnWidth(h,i,w)				((BOOL      )::SendMessage(h,TVM_SETCOLUMNWIDTH,i,w))
#define		TreeList_GetColumnWidth(h,i)				((INT       )::SendMessage(h,TVM_GETCOLUMNWIDTH,i,0))
#define 	TreeList_SetColumnAutoEdit(h,i,f,p)			((BOOL      )::SendMessage(h,TVM_COLUMNAUTOEDIT,(WPARAM)((f)&~TVAE_COL(-1))|TVAE_COL(i),(LPARAM)(p)))
#define 	TreeList_SetColumnAutoIcon(h,i,n)			((BOOL      )::SendMessage(h,TVM_COLUMNAUTOICON,i,n))
#define		TreeList_GetStyle(h)						((DWORD     )::GetWindowLong(h,GWL_STYLE))
#define		TreeList_SetStyle(h,d)	  					((DWORD     )::SetWindowLong(h,GWL_STYLE,d))
#define 	TreeList_SetStyleEx(h,d,m)					((DWORD     )::SetWindowLong(h,GWL_STYLE,((d)&(m))|(::GetWindowLong(h,GWL_STYLE)&~(m))))
#define		TreeList_GetItemRect(h,i,s,p,c)			    (*(HTREEITEM*)p =(i),(BOOL)::SendMessage(h,TVM_GETITEMRECT,(WPARAM)((c)|((s)<<8)),(LPARAM)(RECT*)(p)))

#define		TreeList_SelectItem(h,i)					TreeList_Select(h,i,TVGN_CARET)
#define 	TreeList_SelectDropTarget(h,i)				TreeList_Select(h,i,TVGN_DROPHILITE)
#define 	TreeList_SelectSetFirstVisible(h,i)			TreeList_Select(h,i,TVGN_FIRSTVISIBLE)

#define		TreeList_GetChild(h,i)						TreeView_GetNextItem(h, i,     TVGN_CHILD)
#define 	TreeList_GetParent(h, i)         			TreeView_GetNextItem(h, i,     TVGN_PARENT)
#define 	TreeList_GetNextSibling(h,i)    			TreeView_GetNextItem(h, i,     TVGN_NEXT)
#define 	TreeList_GetPrevSibling(h,i)    			TreeView_GetNextItem(h, i,     TVGN_PREVIOUS)
#define 	TreeList_GetNextSelected(h,i)			    TreeView_GetNextItem(h, i,     TVGN_NEXTSELECTED)
#define		TreeList_GetNextSelectedChild(h,i)			TreeView_GetNextItem(h, i,	   TVGN_NEXTSELCHILD)
#define 	TreeList_GetNextVisible(h,i)    			TreeView_GetNextItem(h, i,     TVGN_NEXTVISIBLE)
#define 	TreeList_GetPrevVisible(h,i)    			TreeView_GetNextItem(h, i,     TVGN_PREVIOUSVISIBLE)
#define 	TreeList_GetSelection(h)					TreeView_GetNextItem(h, NULL,  TVGN_CARET)
#define 	TreeList_GetDropHilight(h)					TreeView_GetNextItem(h, NULL,  TVGN_DROPHILITE)
#define 	TreeList_GetFirstVisible(h)				    TreeView_GetNextItem(h, NULL,  TVGN_FIRSTVISIBLE)
#define		TreeList_GetLastVisible(h)					TreeView_GetNextItem(h, NULL,  TVGN_LASTVISIBLE)
#define 	TreeList_GetRoot(h)							TreeView_GetNextItem(h, NULL,  TVGN_ROOT)
#define		TreeList_GetFocus(h)						TreeView_GetNextItem(h, NULL,  TVGN_FOCUS)
#define		TreeList_GetFocusColumn(h)					((int)TreeView_GetNextItem(h, NULL,  TVGN_FOCUSSUB))
#define		TreeList_GetSelectionColumn(h)				((int)TreeView_GetNextItem(h, NULL,  TVGN_CARETSUB))
#define		TreeList_GetDropHilightColumn(h)			((int)TreeView_GetNextItem(h, NULL,  TVGN_DROPHILITESUB))



#define WM_KEYDOWNEDIT	WM_USER+300

	void	ShowCustomToolTip(HWND hWnd, POINT pt, LPWSTR text, UINT timeout);

	int		TreeListRegister();
	void	TreeListUnregister();
	int		TreeList_CallBackProc( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam );
	int		TreeList_KeyPressed( HWND hWnd,WPARAM wParam,LPARAM lParam );
#endif 	
