#include "StdAfx.h"
#include "CoExtenderFactory.h"
#include "CoExtender.h"

extern UINT g_lockCount;
extern UINT g_objCount;

CoExtenderFactory::CoExtenderFactory(void)
{
	m_refCount = 0;
	g_objCount++;
}

CoExtenderFactory::~CoExtenderFactory(void)
{
	g_objCount--;
}

STDMETHODIMP_(ULONG) CoExtenderFactory::AddRef()
{
	return ++m_refCount;
}

STDMETHODIMP_(ULONG) CoExtenderFactory::Release()
{
	if( 0 == --m_refCount )
	{
		delete this;
		return 0;
	}

	return m_refCount;
}

STDMETHODIMP CoExtenderFactory::QueryInterface(const IID &riid, void **ppvObject)
{
	if( riid == IID_IUnknown )
	{
		*ppvObject = (IUnknown *)this;
	}
	else
	if( riid == IID_IClassFactory )
	{
		*ppvObject = (IClassFactory *)this;
	}

	if( *ppvObject )
	{
		((IUnknown *)*ppvObject)->AddRef();
		return S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP CoExtenderFactory::CreateInstance(IUnknown *pUnkOuter, const IID &riid, void **ppvObject)
{
	if( NULL != pUnkOuter )
	{
		return CLASS_E_NOAGGREGATION;
	}

	CoExtender *pVLCObj = NULL;
	pVLCObj = new CoExtender();

	HRESULT hr = pVLCObj->QueryInterface( riid, ppvObject );

	if( FAILED( hr ) )
	{
		delete pVLCObj;
	}

	return hr;
}

STDMETHODIMP CoExtenderFactory::LockServer(BOOL fLock)
{
	if( fLock )
	{
		++g_lockCount;
	}
	else
	{
		--g_lockCount;
	}

	return S_OK;
}