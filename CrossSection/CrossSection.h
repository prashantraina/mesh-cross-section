
// CrossSection.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "CrossSectionDlg.h"
#include "../IsoSelect/glhelpers.h"


// CCrossSectionApp:
// See CrossSection.cpp for the implementation of this class
//

class CCrossSectionApp : public CWinApp
{
public:
	CCrossSectionApp();
	bool update;
	CCrossSectionDlg *m_pTheDialog;
	GLuint vertexBuffer;
	GLuint vertexArray;
	glm::mat4 worldMat, viewMat, projMat;

	void InitScene();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance();
};

extern CCrossSectionApp theApp;