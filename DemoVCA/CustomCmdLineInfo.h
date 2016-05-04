#pragma once
#include "afxwin.h"
#include "enums.h"

class CCustomCmdLineInfo : public CCommandLineInfo
{
public:
	CCustomCmdLineInfo(void);
	~CCustomCmdLineInfo(void);

	virtual void ParseParam( const TCHAR *pszParam, BOOL bFlag, BOOL bLast );

	CString GetGuid( ) { return m_sGuid; }
	eAppMode GetMode( ) { return m_eMode; }

private:
	eAppMode	m_eMode;
	CString		m_sGuid;
};
