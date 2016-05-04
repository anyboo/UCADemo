#pragma once
#include "unknwn.h"

#include <initguid.h>
// {C2815118-52A8-459c-BDDB-7E1D593CC3FD}
DEFINE_GUID(CLSID_CoExtender, 
0xc2815118, 0x52a8, 0x459c, 0xbd, 0xdb, 0x7e, 0x1d, 0x59, 0x3c, 0xc3, 0xfd);


interface IVCAConfig;



class CoExtenderFactory :
	public IClassFactory
{
public:
	CoExtenderFactory(void);
	~CoExtenderFactory(void);

	// IUnknown
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface( REFIID riid, void **ppvObject );
	STDMETHODIMP CreateInstance( IUnknown *pUnkOuter, const IID &riid, void **ppvObject );
	STDMETHODIMP LockServer( BOOL fLock );


private:
	ULONG	m_refCount;
};
