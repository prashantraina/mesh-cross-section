
// CrossSection.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCrossSectionApp:
// See CrossSection.cpp for the implementation of this class
//

class CCrossSectionApp : public CWinApp
{
public:
	CCrossSectionApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCrossSectionApp theApp;