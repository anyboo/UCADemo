// VLCExtender.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ExtenderDll.h"
#include "CoExtenderFactory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


UINT g_objCount = 0;
UINT g_lockCount = 0;

STDAPI DllGetClassObject( const IID &rclsid, const IID &riid, LPVOID *ppv )
{
	HRESULT hr;

	CoExtenderFactory *pFactory = NULL;

	// Only know how to make CoVLCExtenders
	if( rclsid != CLSID_CoExtender )
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Create a CoVLCExtender
	pFactory = new CoExtenderFactory();

	hr = pFactory->QueryInterface( riid, ppv );

	if( FAILED( hr ) )
	{
		delete pFactory;
	}

	return hr;
}

STDAPI DllCanUnloadNow()
{

	if( g_lockCount == 0 && g_objCount == 0 )
	{
		return S_OK;
	}

	return S_FALSE;
}
