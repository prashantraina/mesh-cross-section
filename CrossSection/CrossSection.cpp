
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
	glEnable(GL_DEPTH_TEST);

	worldMat = glm::mat4(1);

	eyePos = glm::vec3(3);

	viewMat = glm::lookAt(eyePos, glm::vec3(0), glm::vec3(0, 1, 0));

	projMat = glm::perspective(45.0f, 4.0f / 3.0f, 0.01f, 10.0f);

	glm::mat4 viewProj = projMat * viewMat;

	VertexShader phongVS(L"phong.vertexshader");
	FragmentShader phongFS(L"phong.fragmentshader");

	phongShading.reset(new GPUProgram(phongVS, phongFS));

	phongShading->bind();

	glUniformMatrix4fv((*phongShading)["world"], 1, GL_FALSE, &worldMat[0][0]);
	glUniformMatrix4fv((*phongShading)["viewProj"], 1, GL_FALSE, &viewProj[0][0]);
	glUniform3fv((*phongShading)["eyePos"], 1, &eyePos.x);
}

BOOL CCrossSectionApp::OnIdle(LONG lCount)
{
	if (m_pMainWnd->IsIconic())
		return FALSE;

	if (update)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (vertexArray != 0)
		{
			phongShading->bind();

			glBindVertexArray(vertexArray);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
		}

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
	glDeleteBuffers(1, &indexBuffer);

	return CWinApp::ExitInstance();
}

void CCrossSectionApp::LoadMesh(const aiMesh * const *meshes, size_t numMeshes)
{
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);

	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &indexBuffer);
	glGenVertexArrays(1, &vertexArray);

	struct vertex
	{
		aiVector3D position;
		aiVector3D normal;
	};

	size_t numVertices = 0;

	numIndices = 0;

	for (ptrdiff_t i = 0; i < numMeshes; i++)
	{
		numVertices += meshes[i]->mNumVertices;
		numIndices += meshes[i]->mNumFaces * 3UL;
	}

	std::unique_ptr<vertex[]> vertices(new vertex[numVertices]);
	std::unique_ptr<glm::uint32[]> indices(new glm::uint32[numIndices]);

	ptrdiff_t vertI = 0, indexI = 0, lastMeshNumVerts = 0;

	for (ptrdiff_t meshI = 0; meshI < numMeshes; meshI++)
	{
		const aiMesh * const & mesh = meshes[meshI];

		for (ptrdiff_t i = 0; i < mesh->mNumVertices; i++)
		{
			vertices[vertI].position = mesh->mVertices[i];
			vertices[vertI].normal = -mesh->mNormals[i];
			vertices[vertI].normal.Normalize();
			vertI++;
		}

		for (ptrdiff_t i = 0; i < mesh->mNumFaces; i++)
		{
			indices[indexI++] = lastMeshNumVerts + mesh->mFaces[i].mIndices[0];
			indices[indexI++] = lastMeshNumVerts + mesh->mFaces[i].mIndices[1];
			indices[indexI++] = lastMeshNumVerts + mesh->mFaces[i].mIndices[2];
		}
		lastMeshNumVerts += mesh->mNumVertices;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindVertexArray(vertexArray);

	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(vertex), vertices.get(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), nullptr);//position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<char*>(offsetof(vertex, normal)));//normal

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(glm::uint32), indices.get(), GL_STATIC_DRAW);

	update = true;
}