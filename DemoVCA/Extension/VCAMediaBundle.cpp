#include "StdAfx.h"

#include "VCAMediaBundle.h"

VCAMediaBundle::VCAMediaBundle()
{
	m_refCnt = 0;
}

VCAMediaBundle::~VCAMediaBundle()
{
	// Empty out bundle
	std::vector< IVCAMediaSample *>::iterator it = m_samples.begin();
	while( it != m_samples.end() )
	{
		(*it)->Release();

		it = m_samples.erase( it );
	}
}

//static
IVCAMediaBundle *VCAMediaBundle::CreateBundle()
{
	IUnknown *pUnk = new VCAMediaBundle();
	IVCAMediaBundle *pBun;
	pUnk->QueryInterface( IID_IVCAMediaBundle, (void **)&pBun );

	return pBun;
}

STDMETHODIMP_(ULONG) VCAMediaBundle::AddRef( )
{
	return InterlockedIncrement((LPLONG)&m_refCnt);
}

STDMETHODIMP_(ULONG) VCAMediaBundle::Release()
{
	InterlockedDecrement( (LPLONG) &m_refCnt );

	if( 0 == m_refCnt )
	{
		delete this;
		return 0;
	}

	return m_refCnt;
}

STDMETHODIMP VCAMediaBundle::QueryInterface(const IID &riid, void **ppvObject)
{
	*ppvObject = NULL;

	if( IID_IUnknown == riid )
	{
		*ppvObject = (IUnknown *)this;
	}
	else
	if( IID_IVCAMediaBundle == riid )
	{
		*ppvObject = (IVCAMediaBundle *)this;
	}

	if( *ppvObject )
	{
		((IUnknown*)*ppvObject)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP VCAMediaBundle::AddSample(IVCAMediaSample *pSample)
{
	if( pSample )
	{
		pSample->AddRef();

		// And store it
		m_samples.push_back( pSample );
	}

	return S_OK;
}

STDMETHODIMP VCAMediaBundle::GetNumSamples(int &iNumSamples)
{
	iNumSamples = m_samples.size();

	return S_OK;
}

STDMETHODIMP VCAMediaBundle::GetSamplePtr( int iSample, IVCAMediaSample **ppSample)
{
	if( iSample < (int)m_samples.size() )
	{
		*ppSample = m_samples[iSample];

		(*ppSample)->AddRef();

		return S_OK;
	}

	return E_FAIL;
}

//--------------------------------------------------------------------------------------------

VCAMediaSample::VCAMediaSample()
{
	m_refCnt	= 0;
	m_pData		= 0;
	m_iDataLen	= 0;

	memset( &m_hdr, 0, sizeof( VCACONFIG_MEDIASAMPLEHDR ) );
}

VCAMediaSample::~VCAMediaSample()
{
	if( m_pData )
	{
		_aligned_free( m_pData );
	}
}

STDMETHODIMP_(ULONG) VCAMediaSample::AddRef()
{
	return InterlockedIncrement( (LPLONG)&m_refCnt );
}

STDMETHODIMP_(ULONG) VCAMediaSample::Release()
{
	ASSERT( m_refCnt < 10 );

	InterlockedDecrement( (LPLONG)&m_refCnt ); 

	if( 0 == m_refCnt )
	{
		delete this;
		return 0;
	}

	return m_refCnt;
}

// static
IVCAMediaSample *VCAMediaSample::CreateMediaSample( )
{
	IUnknown *pUnk = new VCAMediaSample;
	IVCAMediaSample *pSamp = 0;
	pUnk->QueryInterface( IID_IVCAMediaSample, (void **)&pSamp );

	return pSamp;
}

STDMETHODIMP VCAMediaSample::QueryInterface(const IID &riid, void **ppvObject)
{
	*ppvObject = 0;
	if( IID_IUnknown == riid )
	{
		*ppvObject = (IUnknown *)this;
	}
	else
	if( IID_IVCAMediaSample == riid )
	{
		*ppvObject = (IVCAMediaSample *)this;
	}

	if( *ppvObject )
	{
		((IUnknown *)*ppvObject)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP VCAMediaSample::Create(int iLen)
{
	// Double-create???
	ASSERT( 0 == m_pData );

	if( m_pData )
	{
		_aligned_free( m_pData );
	}

	m_pData = (unsigned char *)_aligned_malloc( iLen, 16 );

	if( m_pData )
	{
		m_iDataLen = iLen;
		return S_OK;
	}

	return E_OUTOFMEMORY;
}

STDMETHODIMP VCAMediaSample::SetHeader( VCACONFIG_MEDIASAMPLEHDR &hdr )
{
	m_hdr = hdr;

	return S_OK;
}

STDMETHODIMP VCAMediaSample::GetHeader(VCACONFIG_MEDIASAMPLEHDR &hdr)
{
	hdr = m_hdr;

	return S_OK;
}

STDMETHODIMP VCAMediaSample::GetDataPtr(void **ppvMem, int &iLen)
{
	if( m_pData )
	{
		*ppvMem = m_pData;
		iLen = m_iDataLen;

		return S_OK;
	}

	return E_POINTER;
}