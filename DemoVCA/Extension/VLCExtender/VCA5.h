#pragma once

#include <initguid.h>
#include <VCA5CoreLib.h>

interface IVCA5;

class VCA5
{
public:
	VCA5(void);
	~VCA5(void);

	BOOL Open( char *pszLicense );
	BOOL Close();
	BOOL Process( unsigned char *pImage, BITMAPINFOHEADER *pBih, unsigned char *pResult, unsigned int *puiResultLen );
	BOOL SetConfig( char *pszBuf, unsigned int uiBufLen );
	BOOL GetConfig( char *pszBuf, unsigned int *puiBufLen );

private:
	BOOL CheckFormatChange( BITMAPINFOHEADER *pBih );

private:
	IVCA5				*m_pVCA5;
	VCA5_SYSTEM_INFO	m_systemInfo;
	BITMAPINFOHEADER	m_bih;

	BOOL				m_fConfigUpdated;
	BOOL				m_fVCAOpened;
	char				*m_pConfig;
	unsigned int		m_uiConfigLen;
};
