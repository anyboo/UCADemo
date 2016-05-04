// CVCAConfigure.h: interface for the CVCAConfigure class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NVCCONFIGURE_H__53903C5A_C23A_4F35_8D86_244A21943406__INCLUDED_)
#define AFX_NVCCONFIGURE_H__53903C5A_C23A_4F35_8D86_244A21943406__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include ".\PUBLIB\INC\VCA5CoreLib.h"
#include "VCAConfigure.h"

class CNVCConfigure
{
public:
	CNVCConfigure();
	~CNVCConfigure();
	
	HRESULT		Load(TCHAR * szFilename);
	HRESULT		Save(TCHAR * szFilename);
	
	void	SetZones(VCA5_APP_ZONES* pZones){
		memcpy(&m_Zones, pZones, sizeof(VCA5_APP_ZONES));
	}
	void	SetRules(VCA5_APP_RULES* pRules){
		memcpy(&m_Zones, pRules, sizeof(VCA5_APP_RULES));
	}

	void	SetCounters(VCA5_APP_COUNTERS* pCounter){
		memcpy(&m_Counters, pCounter, sizeof(VCA5_APP_COUNTERS));
	}	

	void	SetClibInfo(VCA5_CALIB_INFO* pCalbInfo){
		memcpy(&m_CalibInfo, pCalbInfo, sizeof(VCA5_CALIB_INFO));
	}

	void	SetClsObjects(VCA5_APP_CLSOBJECTS* pClsObjes){
		memcpy(&m_ClsObjects, pClsObjes, sizeof(VCA5_APP_CLSOBJECTS));
	}	


	VCA5_APP_ZONES*			GetZones(){return &m_Zones;}
	VCA5_APP_RULES*			GetRules(){return &m_Rules;}
	VCA5_APP_COUNTERS*		GetCounters(){return &m_Counters;}	
	VCA5_CALIB_INFO*		GetClibInfo(){return &m_CalibInfo;}
	VCA5_APP_CLSOBJECTS*	GetClsObjects(){return &m_ClsObjects;}	

private:
	BOOL					m_bLoad;
	
	VCA5_APP_ZONES			m_Zones;
	VCA5_APP_RULES			m_Rules;
	VCA5_APP_COUNTERS		m_Counters;
	VCA5_CALIB_INFO			m_CalibInfo;
	VCA5_APP_CLSOBJECTS		m_ClsObjects;
};


#endif // !defined(AFX_NVCCONFIGURE_H__53903C5A_C23A_4F35_8D86_244A21943406__INCLUDED_)
