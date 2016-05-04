#pragma once

#include "VCAConfig.h"
#include <vector>

class VCAMediaBundle : public IVCAMediaBundle
{
protected:
	VCAMediaBundle();
	virtual ~VCAMediaBundle();

public:
	// IUnknown
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface( const IID &riid, void **ppvObject );

	// IVCAMediaBundle
	STDMETHODIMP AddSample( IVCAMediaSample *pSample );
	STDMETHODIMP GetSamplePtr( int iSample, IVCAMediaSample **ppSample );
	STDMETHODIMP GetNumSamples( int &iNumSamples );

	static IVCAMediaBundle *CreateBundle();

protected:

	UINT	m_refCnt;
	std::vector< IVCAMediaSample *> m_samples;
};

//------------------------------------------------------------------

class VCAMediaSample : public IVCAMediaSample
{
protected:
	VCAMediaSample();
	virtual ~VCAMediaSample();

public:

	static IVCAMediaSample *CreateMediaSample();

	// IUnknown
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface( const IID &riid, void **ppvObject );

	// IMediaSample
	STDMETHODIMP Create( int iLen );
	STDMETHODIMP GetDataPtr( void **ppvMem, int &iLen );
	STDMETHODIMP GetHeader( VCACONFIG_MEDIASAMPLEHDR &hdr );
	STDMETHODIMP SetHeader( VCACONFIG_MEDIASAMPLEHDR &hdr );

protected:

	UINT	m_refCnt;

	VCACONFIG_MEDIASAMPLEHDR	m_hdr;
	unsigned char				*m_pData;
	int							m_iDataLen;

};