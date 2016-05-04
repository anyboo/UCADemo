#pragma once
#include <map>

class CConfigPage;

// CConfigTabCtrl

class CConfigTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CConfigTabCtrl)

public:
	CConfigTabCtrl();
	virtual ~CConfigTabCtrl();

	void Activate( BOOL bActivate );
	void AddTab( LPCTSTR lpszTabName, unsigned int uiResourceId, CConfigPage *pDlg );
	void Apply();

	virtual BOOL DeleteAllItems();

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSelChange( NMHDR *pNotifyStruct, LRESULT *pResult );
	afx_msg void OnSize( UINT nType, int cx, int cy );

protected:

	void CreateContents( unsigned int uId );
	void SizeActiveDlg();

	struct TABENTRY
	{
		unsigned int	uiResourceId;
		CConfigPage			*pDlg;
	};

	std::map< unsigned int, TABENTRY>	m_Tabs;

	CConfigPage	*m_pActiveDlg;
};


