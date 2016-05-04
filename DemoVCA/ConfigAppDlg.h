#pragma once

#include "AppDlg.h"
#include "./AlarmListCtrl/AlarmListCtrl.h"
#include "./VCADialog/VCADialog.h"

#include <initguid.h>
#include "./Extension/VCAConfig.h"

#include <vector>
#include <map>

class CCustomCmdLineInfo;
class ConfigEngine;
class IVCAVideoSource;

// CConfigAppDlg dialog

class CConfigAppDlg : public CAppDlg, public IVCAConfig
{
	DECLARE_DYNAMIC(CConfigAppDlg)

public:
	CConfigAppDlg( CCustomCmdLineInfo *pInfo, CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigAppDlg();

// Dialog Data
	enum { IDD = IDD_CONFIGAPP_DIALOG };

// From CAppDlg
protected:
	virtual UINT GetNumEngine();
	virtual CEngine *GetEngine( int iEngId );
	virtual VCA5_APP_VIDEOSRC_INFO *GetSrcInfo( int iEngId );
	virtual BOOL CreateEngine( int iEngId );
	virtual BOOL DestroyEngine( int iEngId );
	virtual void OnConfigUpdated( int iEngId );

// From IVCAConfig
protected:
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( const IID &riid, void **ppvObject );
	virtual ULONG STDMETHODCALLTYPE Release();

	virtual STDMETHODIMP OnConfigUpdated( IVCADataSource *pSrc, int iChannel );
	virtual STDMETHODIMP DeliverMedia( IVCADataSource *pSrc, int iChannel, IVCAMediaBundle *pBun );

protected:
	BOOL LoadConfig( int iEngId );

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

protected:
	AlarmListCtrl	m_AlarmList;
	CVCADialog		*m_pVCADlg;
	UINT			m_uiRefCount;
	CCustomCmdLineInfo	*m_pCmdLineInfo;
	IVCADataSource	*m_pVCADataSrc;

	std::map< int, VCA5_APP_VIDEOSRC_INFO > m_srcInfo;
	std::map< int, ConfigEngine *>	m_engines;
	std::vector< IVCAVideoSource *>	m_videoSources;

	char			*m_pszConfigBuf;
};
