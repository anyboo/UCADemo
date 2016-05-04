#pragma once

#include "APPConfigure.h"
#include "VCAConfigure.h"

class CVCAEngine;
class CVCASystem
{
public:
	CVCASystem();
	virtual ~CVCASystem();

	BOOL	Setup();
	void	Endup();
	

	BOOL	Open( int iEngId );
	BOOL	Close( int iEngId );

	BOOL	Run( int iEngId );
	void	Stop( int iEngId );

	
	IVCA5*		GetVCA5Lib(){return m_pVCA5API;}
	ULONG		GetNumEngine() {return m_OpenEngCnt;}
	CVCAEngine*	GetEngine(ULONG i) {
		return (VCA5_MAX_NUM_ENGINE > i)?m_pEngines[i] : NULL;
	}

	
private:
	BOOL			m_bSetup;
	DWORD			m_OpenEngCnt;	
	
	HMODULE			m_hVCA5Lib;
	IVCA5			*m_pVCA5API;

	CVCAEngine			*m_pEngines[VCA5_MAX_NUM_ENGINE];
};
