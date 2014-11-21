
// CrossSection.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CrossSection.h"
#include "CrossSectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCrossSectionApp

BEGIN_MESSAGE_MAP(CCrossSectionApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCrossSectionApp construction

CCrossSectionApp::CCrossSectionApp() : update(true)
{
}


// The one and only CCrossSectionApp object

CCrossSectionApp theApp;


// CCrossSectionApp initialization

BOOL CCrossSectionApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	m_pTheDialog = new CCrossSectionDlg;
	m_pMainWnd = m_pTheDialog;
	m_pTheDialog->Create(IDD_CROSSSECTION_DIALOG, NULL);

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	
	return TRUE;
}

void CCrossSectionApp::InitScene()
{
}

BOOL CCrossSectionApp::OnIdle(LONG lCount)
{
	if (m_pMainWnd->IsIconic())
		return FALSE;

	if (update)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		::SwapBuffers(*m_pTheDialog->m_viewportCtl.GetDC()); 
		update = false;
		return TRUE;
	}

	return FALSE;
}


int CCrossSectionApp::ExitInstance()
{
	delete m_pMainWnd;

	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);

	return CWinApp::ExitInstance();
}
