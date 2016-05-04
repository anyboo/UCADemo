//*****************************************************************************
//*
//*		
//*		TreeListCtrl.h
//*
//*
//*****************************************************************************
#ifndef 	__TREELISTCTRL_H__
#define 	__TREELISTCTRL_H__


#include	"TreeListWnd.h"

class CHeaderCtrl;



//*****************************************************************************
//*
//*		CTreeListCtrl
//*
//*****************************************************************************
class CTreeListCtrl : public CTreeCtrl
{

	DECLARE_DYNAMIC(CTreeListCtrl)

public:

	CTreeListCtrl(){};
	virtual ~CTreeListCtrl();
	BOOL		 Create					(DWORD dwStyle, const RECT &sRect, CWnd* pParentWnd, UINT nId);
	int  		 InsertColumn  		 	(int nCol, LPCTSTR pColumnText, int nFormat=TVCFMT_LEFT, int nWidth=-1);
	int  		 InsertColumn  		 	(int nCol, const TV_COLUMN* pColumn );
	BOOL		 SetColumn     		 	(int nCol, const TV_COLUMN* pColumn );
	BOOL		 GetColumn     		 	(int nCol, TV_COLUMN* pColumn ) const;
	int			 GetColumnWidth		 	(int nCol) const;
	BOOL		 SetColumnWidth		 	(int nCol, int iWidth);
	BOOL 		 DeleteColumn		 	(int nCol);
	int 		 GetColumnCount			() const;
	CHeaderCtrl *GetHeaderCtrl		 	() const;
	CEdit		*EditLabel			 	(HTREEITEM hItem,int nCol=0,int iFull=0);
	CComboBox	*EditLabelCb		 	(HTREEITEM hItem,int nCol=0,int iFull=0,int iList=0);
	CImageList  *CreateDragImage	 	(HTREEITEM hItem,int nCol=0); 	
	DWORD		 GetStyle			 	() const;
	DWORD		 SetStyle			 	(DWORD dwStyle);
	DWORD		 SetStyle			 	(DWORD dwStyle,DWORD dwMask);
	DWORD		 GetExtendedStyle	 	() const;
	DWORD		 SetExtendedStyle	 	(DWORD dwStyle);
	DWORD		 SetExtendedStyle	 	(DWORD dwStyle,DWORD dwMask);
	COLORREF	 GetColor			 	(int iIndex) const;
	COLORREF	 SetColor			 	(int iIndex ,COLORREF uColor);
	COLORREF	 GetItemBkColor		 	(HTREEITEM hItem,int nCol=0) const;
	COLORREF	 SetItemBkColor		 	(HTREEITEM hItem,int nCol,COLORREF uColor);
	COLORREF	 GetItemTextColor	 	(HTREEITEM hItem,int nCol=0) const;
	COLORREF	 SetItemTextColor	 	(HTREEITEM hItem,int nCol,COLORREF uColor);
	BOOL		 EnsureVisible		 	(HTREEITEM hItem,int nCol);
	BOOL		 EnsureVisible		 	(HTREEITEM hItem);
	BOOL		 SelectDropTarget	 	(HTREEITEM hItem,int nCol=0);
	BOOL		 SelectItem			 	(HTREEITEM hItem,int nCol=0);
	BOOL		 GetItemRect		 	(HTREEITEM hItem,int nCol,RECT *pRect,int iCode);
	BOOL		 GetItemRect		 	(HTREEITEM hItem,		  RECT *pRect,int iCode);
	BOOL		 GetItemText		 	(HTREEITEM hItem,LPTSTR pBuffer,int iMax,int nCol=0);
	LPCTSTR		 GetItemText		 	(HTREEITEM hItem,                        int nCol=0);
	BOOL		 SetItemText		 	(HTREEITEM hItem,LPCTSTR pText          ,int nCol=0);
	int 		 GetItemImageEx		 	(HTREEITEM hItem                        ,int nCol=0);
	BOOL		 GetItemImageEx	     	(HTREEITEM hItem,int *pImage            ,int nCol=0);
	BOOL		 SetItemImageEx		 	(HTREEITEM hItem,int  nImage            ,int nCol=0);
	UINT 		 GetItemState        	(HTREEITEM hItem,UINT nStateMask        ,int nCol=0);
	BOOL		 SetItemState        	(HTREEITEM hItem,         UINT nState,UINT nStateMask);
	BOOL		 SetItemState        	(HTREEITEM hItem,int nCol,UINT nState,UINT nStateMask);
	BOOL		 SetItem			 	(HTREEITEM hItem,int nCol,UINT nMask, LPCTSTR pText,int nImage,int nSelectedImage,UINT nState,UINT nStateMask, LPARAM lParam=0);
	BOOL		 SetItem			 	(TVITEM* pItem);
	BOOL		 SortChildrenEX		 	(LPTVSORTEX pSort,BOOL bRecusive=FALSE);
	BOOL		 SortChildrenCB		 	(LPTVSORTCB pSort,BOOL bRecusive=FALSE);
	BOOL		 SortChildren		 	(HTREEITEM hItem ,BOOL bRecusive=FALSE);
	LPVOID		 GetUserData		 	(HTREEITEM hItem) const;
	INT			 SetUserDataSize     	(int iSize);
	INT			 GetUserDataSize	 	() const;
	HTREEITEM	 GetFirstSelected	 	();
	HTREEITEM	 GetNextSelected	 	(HTREEITEM hItem);
	HTREEITEM	 GetNextSelectedChild	(HTREEITEM hItem);
	HTREEITEM	 GetFocusItem		 	();
	int			 SetColumnAutoEdit		(int nCol,int iMode=TVAE_EDIT);
	int			 SetColumnAutoEdit		(int nCol,int iMode,            LPTSTR  *pList,int iMax=0);
	int			 SetColumnAutoEdit		(int nCol,int iMode,            LPCTSTR *pList,int iMax=0);
	int			 SetColumnAutoEdit		(int nCol,int iMode,            LPCTSTR  pText,int iMax=0);
	int			 SetColumnAutoEdit		(int nCol,int iMode,TCHAR cChar,LPCTSTR  pText,int iMax=0);
	int			 SetColumnAutoIcon		(int nCol,int iIcon);
	int 		 GetFocusColumn		 	(); 
	int 		 GetSelectionColumn	 	(); 
	int 		 GetDropHilightColumn	();

//bincheng
	 BOOL		 FinishEditingItem(HTREEITEM hItem) const;
	// ListCtrl Functions

	int			 GetRowCount		 	() const;
	int			 GetRowOfItem		 	(HTREEITEM hItem) const;
	HTREEITEM	 GetItemOfRow		 	(int iRow) const;
	CEdit		*ListEditLabel		 	(int iRow,int nCol=0,int iFull=0);
	CComboBox	*ListEditLabelCb	 	(int iRow,int nCol=0,int iFull=0,int iList=0);
	CImageList  *ListCreateDragImage 	(int iRow,int nCol=0); 	
	BOOL		 ListDeleteItem		 	(int iRow);
	int 		 ListInsertItem		 	(int iRow,		   LPCTSTR pText,int nImage=-1,int nState=-1,int iMask=TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK);	
	BOOL		 ListSetItem		 	(int iRow,		   LPCTSTR pText,int nImage=-1,int nState=-1,int iMask=TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK);	
	BOOL		 ListSetItem		 	(int iRow,int iCol,LPCTSTR pText,int nImage=-1,int nState=-1,int iMask=TVIS_BOLD|TVIS_UNTERLINE|TVIS_SELECTED|TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK);	
	BOOL		 ListSetColor		 	(int iRow,int iCol,COLORREF  uBkColor=TV_NOCOLOR,COLORREF  uTextColor=TV_NOCOLOR); 
	BOOL		 ListGetColor		 	(int iRow,int iCol,COLORREF &uBkColor           ,COLORREF &uTextColor);
	LPCTSTR		 ListGetItemText	 	(int iRow,              int nCol=0);
	BOOL		 ListSetItemText	 	(int iRow,LPCTSTR pText,int nCol=0);
	UINT 		 ListGetItemState    	(int iRow,UINT nStateMask     ,int nCol=0);
	BOOL		 ListSetItemState    	(int iRow,         UINT nState,UINT nStateMask);
	BOOL		 ListSetItemState    	(int iRow,int nCol,UINT nState,UINT nStateMask);
	BOOL		 ListSelectItem		 	(int iRow,int nCol=0);
	BOOL		 ListEnsureVisible	 	(int iRow,int nCol  );
	BOOL		 ListEnsureVisible	 	(int iRow);
	int 		 ListGetItemImageEx	 	(int iRow             ,int nCol=0);
	BOOL		 ListGetItemImageEx	 	(int iRow,int *pImage ,int nCol=0);
	BOOL		 ListSetItemImageEx	 	(int iRow,int  nImage ,int nCol=0);
	int			 ListGetItemImage    	(int iRow,int &nImage,int &nSelectedImage);
	BOOL		 ListSetItemImage    	(int iRow,int  nImage,int  nSelectedImage);
	LPVOID		 ListGetUserData	 	(int iRow) const;
	int			 ListGetNextSelected 	(int iRow);
	COLORREF	 ListGetItemBkColor	 	(int iRow,int nCol=0) const;
	COLORREF	 ListSetItemBkColor		(int iRow,int nCol,COLORREF uColor);
	COLORREF	 ListGetItemTextColor	(int iRow,int nCol=0) const;
	COLORREF	 ListSetItemTextColor	(int iRow,int nCol,COLORREF uColor);
	BOOL		 ListSelectDropTarget	(int iRow,int nCol=0);
	BOOL		 ListGetItemRect		(int iRow,int nCol,RECT *pRect,int iCode);
	BOOL		 ListGetItemRect		(int iRow,		   RECT *pRect,int iCode);
	int			 ListGetFirstSelected 	();
	int			 ListGetFocusItem	 	();
	
	
	
};


//*****************************************************************************
//*
//*		Inline Funktions
//*
//*****************************************************************************

//bincheng
inline  BOOL		 CTreeListCtrl::FinishEditingItem(HTREEITEM hItem) const					{ ASSERT(::IsWindow(m_hWnd)); return TreeList_EndEditLabelNow(m_hWnd,hItem); }

inline	LPVOID		 CTreeListCtrl::GetUserData(HTREEITEM hItem) const							{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetUserData(m_hWnd,hItem);}
inline	INT			 CTreeListCtrl::GetUserDataSize() const										{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetUserDataSize(m_hWnd);}
inline	INT			 CTreeListCtrl::SetUserDataSize(int iSize)									{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetUserDataSize(m_hWnd,iSize);}
inline	BOOL		 CTreeListCtrl::GetColumn   (int nCol,       TV_COLUMN *pColumn) const		{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetColumn(m_hWnd,nCol,pColumn);}
inline	BOOL		 CTreeListCtrl::SetColumn   (int nCol, const TV_COLUMN *pColumn)			{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColumn(m_hWnd,nCol,pColumn);}
inline	INT 		 CTreeListCtrl::InsertColumn(int nCol, const TV_COLUMN *pColumn)			{ ASSERT(::IsWindow(m_hWnd)); return TreeList_InsertColumn(m_hWnd,nCol,pColumn); }
inline	BOOL		 CTreeListCtrl::DeleteColumn(int nCol)										{ ASSERT(::IsWindow(m_hWnd)); return TreeList_DeleteColumn(m_hWnd,nCol); }
inline	int 		 CTreeListCtrl::GetColumnCount() const										{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetColumnCount(m_hWnd); }
inline	BOOL		 CTreeListCtrl::SortChildren  (HTREEITEM  hItem,BOOL bRecusive)				{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SortChildren  (m_hWnd,hItem,bRecusive); }
inline	BOOL		 CTreeListCtrl::SortChildrenCB(LPTVSORTCB pSort,BOOL bRecusive)				{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SortChildrenCB(m_hWnd,pSort,bRecusive); }
inline	BOOL		 CTreeListCtrl::SortChildrenEX(LPTVSORTEX pSort,BOOL bRecusive)				{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SortChildrenEX(m_hWnd,pSort,bRecusive); }
inline	INT			 CTreeListCtrl::GetColumnWidth(int nCol) const								{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetColumnWidth(m_hWnd,nCol); }
inline	BOOL		 CTreeListCtrl::SetColumnWidth(int nCol, int iWidth)						{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColumnWidth(m_hWnd,nCol,iWidth); }
inline	HTREEITEM	 CTreeListCtrl::GetItemOfRow(int iRow) const								{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetItemOfRow(m_hWnd,iRow); }
inline	INT			 CTreeListCtrl::GetRowOfItem(HTREEITEM hItem) const							{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetRowOfItem(m_hWnd,hItem); }
inline	INT			 CTreeListCtrl::GetRowCount() const											{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetRowCount(m_hWnd); }
inline	DWORD		 CTreeListCtrl::GetStyle() const											{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetStyle(m_hWnd); }
inline	DWORD		 CTreeListCtrl::SetStyle(DWORD dwStyle)										{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetStyle(m_hWnd,dwStyle); }
inline	DWORD		 CTreeListCtrl::SetStyle(DWORD dwStyle,DWORD dwMask)						{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetStyleEx(m_hWnd,dwStyle,dwMask); }
inline	DWORD		 CTreeListCtrl::GetExtendedStyle() const									{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetExtendedStyle(m_hWnd); }
inline	DWORD		 CTreeListCtrl::SetExtendedStyle(DWORD dwStyle)								{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetExtendedStyle(m_hWnd,dwStyle); }
inline	DWORD		 CTreeListCtrl::SetExtendedStyle(DWORD dwStyle,DWORD dwMask)				{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetExtendedStyleEx(m_hWnd,dwStyle,dwMask); }
inline	COLORREF	 CTreeListCtrl::GetColor(int iIndex) const									{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetColor(m_hWnd,iIndex); }
inline	COLORREF	 CTreeListCtrl::SetColor(int iIndex,COLORREF uColor)						{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColor(m_hWnd,iIndex,uColor); }
inline	COLORREF	 CTreeListCtrl::GetItemBkColor  (HTREEITEM hItem,int nCol) const			{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetItemBkColor(m_hWnd,hItem,nCol); }
inline	COLORREF	 CTreeListCtrl::SetItemBkColor  (HTREEITEM hItem,int nCol,COLORREF uColor)	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetItemBkColor(m_hWnd,hItem,nCol,uColor); }
inline	COLORREF	 CTreeListCtrl::GetItemTextColor(HTREEITEM hItem,int nCol) const			{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetItemTextColor(m_hWnd,hItem,nCol); }
inline	COLORREF	 CTreeListCtrl::SetItemTextColor(HTREEITEM hItem,int nCol,COLORREF uColor)	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetItemTextColor(m_hWnd,hItem,nCol,uColor); }
inline	BOOL		 CTreeListCtrl::EnsureVisible   (HTREEITEM hItem,int nCol)					{ ASSERT(::IsWindow(m_hWnd)); return TreeList_EnsureVisibleEx(m_hWnd,hItem,nCol); }
inline	BOOL		 CTreeListCtrl::SelectDropTarget(HTREEITEM hItem,int nCol)					{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SelectDropTargetEx(m_hWnd,hItem,nCol); }
inline	BOOL		 CTreeListCtrl::SelectItem (HTREEITEM hItem,int nCol)						{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SelectSubItem(m_hWnd,hItem,nCol); }
inline	BOOL		 CTreeListCtrl::GetItemRect(HTREEITEM hItem,int nCol,RECT *pRect,int iCode)	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetItemRect(m_hWnd,hItem,nCol,pRect,iCode); }			
inline	HTREEITEM	 CTreeListCtrl::GetNextSelectedChild(HTREEITEM hItem) 						{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetNextSelectedChild(m_hWnd,hItem); }	
inline	HTREEITEM	 CTreeListCtrl::GetNextSelected(HTREEITEM hItem) 							{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetNextSelected(m_hWnd,hItem); }
inline	HTREEITEM	 CTreeListCtrl::GetFirstSelected()											{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetNextSelected(m_hWnd,TVI_ROOT); }
inline	HTREEITEM	 CTreeListCtrl::GetFocusItem()												{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetFocus(m_hWnd); }
inline	INT 		 CTreeListCtrl::GetFocusColumn() 											{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetFocusColumn(m_hWnd); }														
inline	INT 		 CTreeListCtrl::GetSelectionColumn() 										{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetSelectionColumn(m_hWnd); }
inline	INT 		 CTreeListCtrl::GetDropHilightColumn() 										{ ASSERT(::IsWindow(m_hWnd)); return TreeList_GetDropHilightColumn(m_hWnd); }	
inline	CEdit		*CTreeListCtrl::EditLabel  (HTREEITEM hItem,int nCol,int iFull)				{ ASSERT(::IsWindow(m_hWnd)); return (CEdit      *)CEdit      ::FromHandle(TreeList_EditLabel(m_hWnd,hItem,(nCol&0x1FFFFFFF)|((iFull)? TVIR_EDITFULL:0))); }
inline	CComboBox	*CTreeListCtrl::EditLabelCb(HTREEITEM hItem,int nCol,int iFull,int iList)	{ ASSERT(::IsWindow(m_hWnd)); return (CComboBox  *)CEdit      ::FromHandle(TreeList_EditLabel(m_hWnd,hItem,(nCol&0x1FFFFFFF)|((iFull)? TVIR_EDITFULL:0)|((iList)? TVIR_EDITCOMBOLIST:TVIR_EDITCOMBOBOX))); }
inline	CImageList  *CTreeListCtrl::CreateDragImage(HTREEITEM hItem,int nCol)					{ ASSERT(::IsWindow(m_hWnd)); return (CImageList *)CImageList ::FromHandle(TreeList_CreateDragImageEx(m_hWnd,hItem,nCol)); }
inline	CHeaderCtrl *CTreeListCtrl::GetHeaderCtrl() const										{ ASSERT(::IsWindow(m_hWnd)); return (CHeaderCtrl*)CHeaderCtrl::FromHandle(TreeList_GetHeader(m_hWnd)); }
inline	int			 CTreeListCtrl::SetColumnAutoEdit(int n,int m							  )	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColumnAutoEdit(m_hWnd,n, m                                          ,NULL ); }			
inline	int			 CTreeListCtrl::SetColumnAutoEdit(int n,int m,        LPTSTR  *pList,int x)	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColumnAutoEdit(m_hWnd,n,(m|TVAE_COUNT(x)             )| TVAE_PTRLIST,pList); }			
inline	int			 CTreeListCtrl::SetColumnAutoEdit(int n,int m,        LPCTSTR *pList,int x)	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColumnAutoEdit(m_hWnd,n,(m|TVAE_COUNT(x)             )| TVAE_PTRLIST,pList); }			
inline	int			 CTreeListCtrl::SetColumnAutoEdit(int n,int m,        LPCTSTR  pText,int x)	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColumnAutoEdit(m_hWnd,n,(m|TVAE_COUNT(x)             )&~TVAE_PTRLIST,pText); }			
inline	int			 CTreeListCtrl::SetColumnAutoEdit(int n,int m,TCHAR c,LPCTSTR  pText,int x)	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColumnAutoEdit(m_hWnd,n,(m|TVAE_COUNT(x)|TVAE_CHAR(c))&~TVAE_PTRLIST,pText); }			
inline	int			 CTreeListCtrl::SetColumnAutoIcon(int n,int iIcon                         )	{ ASSERT(::IsWindow(m_hWnd)); return TreeList_SetColumnAutoIcon(m_hWnd,n,iIcon); }			

inline	BOOL		 CTreeListCtrl::SetItemState(HTREEITEM hItem,UINT nState,UINT nMask)		{ return SetItemState(hItem,0,nState,nMask);}
inline	BOOL		 CTreeListCtrl::EnsureVisible(HTREEITEM hItem)								{ return EnsureVisible(hItem,0xFFFFFFFF); }		
inline	BOOL		 CTreeListCtrl::GetItemRect(HTREEITEM hItem,RECT *pRect,int iCode)			{ return GetItemRect(hItem,pRect,iCode); }
inline	BOOL		 CTreeListCtrl::SetItem(TVITEM* pItem)										{ return CTreeCtrl::SetItem(pItem); }

// ListCtrl Functions

inline	BOOL		 CTreeListCtrl::ListSetItem(int iRow,LPCTSTR pTxt,int nImg,int nSt,int iMsk){ return ListSetItem(iRow,0,pTxt,nImg,nSt,iMsk);				}
inline	CEdit		*CTreeListCtrl::ListEditLabel	    (int iRow,int nCol,int iFull)			{ return EditLabel	     (GetItemOfRow(iRow),nCol,iFull);		}
inline	CComboBox	*CTreeListCtrl::ListEditLabelCb	    (int iRow,int nCol,int iFull,int iList)	{ return EditLabelCb	 (GetItemOfRow(iRow),nCol,iFull,iList);	}
inline	CImageList  *CTreeListCtrl::ListCreateDragImage (int iRow,int nCol)	 					{ return CreateDragImage (GetItemOfRow(iRow),nCol);				}
inline	LPCTSTR		 CTreeListCtrl::ListGetItemText     (int iRow,                     int nCol){ return GetItemText     (GetItemOfRow(iRow)             ,nCol);}
inline	BOOL		 CTreeListCtrl::ListSetItemText     (int iRow,LPCTSTR pText       ,int nCol){ return SetItemText     (GetItemOfRow(iRow),pText       ,nCol);}
inline	UINT 		 CTreeListCtrl::ListGetItemState    (int iRow,UINT nStateMask     ,int nCol){ return GetItemState    (GetItemOfRow(iRow),nStateMask,  nCol);}		
inline	BOOL		 CTreeListCtrl::ListSetItemState    (int iRow,         UINT nStat,UINT nMsk){ return SetItemState    (GetItemOfRow(iRow),     nStat,nMsk);	}
inline	BOOL		 CTreeListCtrl::ListSetItemState    (int iRow,int nCol,UINT nStat,UINT nMsk){ return SetItemState    (GetItemOfRow(iRow),nCol,nStat,nMsk);	}
inline	BOOL		 CTreeListCtrl::ListSelectItem	    (int iRow,int nCol)						{ return SelectItem	     (GetItemOfRow(iRow),nCol);				}
inline	BOOL		 CTreeListCtrl::ListEnsureVisible   (int iRow,int nCol)						{ return EnsureVisible   (GetItemOfRow(iRow),nCol);				}
inline	BOOL		 CTreeListCtrl::ListEnsureVisible   (int iRow)								{ return EnsureVisible   (GetItemOfRow(iRow));					}
inline	LPVOID		 CTreeListCtrl::ListGetUserData	    (int iRow) const						{ return GetUserData     (GetItemOfRow(iRow));					}
inline	int			 CTreeListCtrl::ListGetItemImageEx  (int iRow             ,int nCol)		{ return GetItemImageEx  (GetItemOfRow(iRow)       ,nCol);		}
inline	BOOL		 CTreeListCtrl::ListGetItemImageEx  (int iRow,int *pImage ,int nCol)		{ return GetItemImageEx  (GetItemOfRow(iRow),pImage,nCol);		}
inline	BOOL		 CTreeListCtrl::ListSetItemImageEx  (int iRow,int  nImage ,int nCol)		{ return SetItemImageEx  (GetItemOfRow(iRow),nImage,nCol);		}
inline	int			 CTreeListCtrl::ListGetItemImage    (int iRow,int &nImage,int &nSelImage)	{ return GetItemImage    (GetItemOfRow(iRow),nImage,nSelImage);	}
inline	BOOL		 CTreeListCtrl::ListSetItemImage    (int iRow,int  nImage,int  nSelImage)	{ return SetItemImage    (GetItemOfRow(iRow),nImage,nSelImage);	}
inline	int			 CTreeListCtrl::ListGetNextSelected (int iRow)								{ return GetRowOfItem    (GetNextSelected(GetItemOfRow(iRow)));	}
inline	int			 CTreeListCtrl::ListGetFirstSelected(        )								{ return GetRowOfItem    (GetFirstSelected());					}
inline	COLORREF	 CTreeListCtrl::ListGetItemBkColor  (int iRow,int nCol) const				{ return GetItemBkColor  (GetItemOfRow(iRow),nCol);				}
inline	COLORREF	 CTreeListCtrl::ListSetItemBkColor  (int iRow,int nCol,COLORREF uColor)		{ return SetItemBkColor  (GetItemOfRow(iRow),nCol,uColor);		}
inline	COLORREF	 CTreeListCtrl::ListGetItemTextColor(int iRow,int nCol) const				{ return GetItemTextColor(GetItemOfRow(iRow),nCol);				}
inline	COLORREF	 CTreeListCtrl::ListSetItemTextColor(int iRow,int nCol,COLORREF uColor)		{ return SetItemTextColor(GetItemOfRow(iRow),nCol,uColor);		}
inline	BOOL		 CTreeListCtrl::ListSelectDropTarget(int iRow,int nCol)						{ return SelectDropTarget(GetItemOfRow(iRow),nCol);				}
inline	BOOL		 CTreeListCtrl::ListGetItemRect		(int iRow,int nCol,RECT *pRec,int iCode){ return GetItemRect	 (GetItemOfRow(iRow),nCol,pRec,iCode);	}
inline	BOOL		 CTreeListCtrl::ListGetItemRect		(int iRow,		   RECT *pRec,int iCode){ return GetItemRect	 (GetItemOfRow(iRow),     pRec,iCode);	}
inline	int			 CTreeListCtrl::ListGetFocusItem ()											{ return GetRowOfItem    (GetFocusItem());						}


#endif 	

