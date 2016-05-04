#include "stdafx.h"
#include "CustomCmdLineInfo.h"

CCustomCmdLineInfo::CCustomCmdLineInfo(void)
{
	m_eMode = eAppModeDemo;
}

CCustomCmdLineInfo::~CCustomCmdLineInfo(void)
{
}

void CCustomCmdLineInfo::ParseParam(const TCHAR *pszParam, BOOL bFlag, BOOL bLast)
{
	CString s = CString( pszParam );

	int iPos = s.Find( _T("mode:") );

	if( -1 != iPos )
	{
		CString sMode = s.Mid( s.Find(_T(":"))+1 );

		if( 0 == sMode.Compare( _T("c") ) )
		{
			m_eMode = eAppModeConfig;
		}
	}

	iPos = s.Find( _T("guid:") );
	if( -1 != iPos )
	{
		m_sGuid = s.Mid( s.Find(_T(":"))+1 );
	}

}