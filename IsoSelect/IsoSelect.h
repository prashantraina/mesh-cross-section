
// IsoSelect.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "IsoSelectDlg.h"

// CIsoSelectApp:
// See IsoSelect.cpp for the implementation of this class
//

class CIsoSelectApp : public CWinApp
{
public:
	CIsoSelectApp();
	bool update;
	CIsoSelectDlg *m_pTheDialog;

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance();
};

extern CIsoSelectApp theApp;