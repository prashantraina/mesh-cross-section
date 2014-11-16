
// IsoSelect.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "IsoSelect.h"
#include "IsoSelectDlg.h"
#include "marchingcubes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIsoSelectApp

BEGIN_MESSAGE_MAP(CIsoSelectApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CIsoSelectApp construction

CIsoSelectApp::CIsoSelectApp()
{
	this->update = true;

	// Place all significant initialization in InitInstance
}


// The one and only CIsoSelectApp object

CIsoSelectApp theApp;


// CIsoSelectApp initialization

BOOL CIsoSelectApp::InitInstance()
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


	//AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	m_pTheDialog = new CIsoSelectDlg;
	m_pMainWnd = m_pTheDialog;
	m_pTheDialog->Create(IDD_ISOSELECT_DIALOG, NULL);
	//INT_PTR nResponse = dlg->DoModal();
	/*if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}*/

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	return TRUE;
}

void CIsoSelectApp::InitScene()
{
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LEQUAL);

	worldMat = glm::mat4(1.0f);
	viewMat = glm::lookAt(
		glm::vec3(512, 512, 512),	//eye
		glm::vec3(0, 0, 0),			//at
		glm::vec3(0, 1, 0));		//up
	projMat = glm::perspective(45.0f, 4.0f / 3.0f, 1.0f, 1000.0f);

	VertexShader vs(L"instanceCube.vertexshader");
	FragmentShader fs(L"solidColor.fragmentshader");
	GeometryShader gs(L"marchingCubes.geometryshader");

	marchingCubes.reset(new GPUProgram(vs, fs, gs));

	marchingCubes->bind();
	glUniformMatrix4fv((*marchingCubes)["world"], 1, GL_FALSE, &worldMat[0][0]);
	glUniformMatrix4fv((*marchingCubes)["view"], 1, GL_FALSE, &viewMat[0][0]);
	glUniformMatrix4fv((*marchingCubes)["projection"], 1, GL_FALSE, &projMat[0][0]);
	glUniform3i((*marchingCubes)["dims"], 256, 256, 256);


	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	float dummy[] = { 0.0f };
	glBufferData(GL_ARRAY_BUFFER, sizeof(float), dummy, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glVertexAttribPointer(0, //location
		1,				// number of components
		GL_FLOAT,		// type
		GL_FALSE,		// normalized?
		sizeof(float), // stride
		nullptr      // array buffer offset
		);

	triTableTex.reset(new Texture2D(GL_TEXTURE1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		16, 256, GL_R32I, GL_RED_INTEGER, GL_INT, tri_table));

	std::ifstream inDump(L"../600k.dmp", std::ios::binary);
	if (inDump.is_open())
	{
		inDump.seekg(0, std::ios::end);
		std::streamsize size = inDump.tellg();
		inDump.seekg(0, std::ios::beg);

		size_t BytecodeLength = size;
		char *Bytecode = new char[BytecodeLength];

		if (!inDump.read(Bytecode, size))
		{
			delete[] Bytecode;
			inDump.close();
			throw "unable to read file";
		}

		volumeTex.reset(new Texture3D(GL_TEXTURE0, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
			GL_CLAMP_TO_EDGE, 256, 256, 256, GL_R32F, GL_RED, GL_FLOAT, Bytecode));

		delete[] Bytecode;
		inDump.close();
	}
	else
	{
		throw "unable to open file";
	}

	triTableTex->BindToUniform((*marchingCubes)["tritable"]);
	volumeTex->BindToUniform((*marchingCubes)["volume"]);

	glUniform1f((*marchingCubes)["iso"], -0.4f);
}


BOOL CIsoSelectApp::OnIdle(LONG lCount)
{
	if (m_pMainWnd->IsIconic())
		return FALSE;

	if (update)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		marchingCubes->bind();

		glBindVertexArray(vertexArray);
		glEnableVertexAttribArray(0);
		glPointSize(1.0f);
		glDrawArraysInstanced(GL_POINTS, 0, 1, 1 << 24);

		::SwapBuffers(*m_pTheDialog->m_viewportCtl.GetDC());

		update = false;
		return TRUE;
	}

	return FALSE;
}


int CIsoSelectApp::ExitInstance()
{
	if (m_pMainWnd)
	{
		delete m_pMainWnd;
	}

	marchingCubes.reset();

	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);

	return CWinApp::ExitInstance();
}
