#pragma once

#include "../resource.h"
#include "VCACOnfigureObserver.h"

// CCalibrationCtl dialog
class CVCADataMgr;

class CCalibrationCtl : public CDialog, public IVCAConfigureObserver
{
	DECLARE_DYNAMIC(CCalibrationCtl)

public:
	CCalibrationCtl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCalibrationCtl();

	void SetDataMgr( CVCADataMgr *pDataMgr ) { m_pDataMgr = pDataMgr; }

// Dialog Data
	enum { IDD = IDD_CALIBRATIONCTL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

protected:
	virtual void FireOnEvent( DWORD uiEvent, DWORD uiContext );

	void UpdateValues( );
	void ApplyChanges();


private:
	
	CVCADataMgr	*m_pDataMgr;
	BOOL		m_bAlive;
	BOOL		m_bApplyData;

protected:
	afx_msg void OnEnChangeEdHeight();
	afx_msg void OnEnChangeEdHeight2();
	afx_msg void OnEnChangeEdTilt();
	afx_msg void OnEnChangeEdFov();
	afx_msg void OnEnChangeEdPan();
	afx_msg void OnEnChangeEdRoll();
	afx_msg void OnCbnSelchangeCalibUnitsCombo();
	afx_msg void OnBnClickedButtonRestore();
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

};



