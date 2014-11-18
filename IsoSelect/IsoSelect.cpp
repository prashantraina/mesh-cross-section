
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


	glm::vec3 eye(dim * 2);

	worldMat = glm::mat4(1.0f);
	viewMat = glm::lookAt(
		eye,	//eye
		glm::vec3(0, 0, 0),			//at
		glm::vec3(0, 1, 0));		//up
	projMat = glm::perspective(45.0f, 4.0f / 3.0f, 1.0f, 1000.0f);

	VertexShader vs(L"instanceCube.vertexshader");
	//FragmentShader fs(L"solidColor.fragmentshader");
	FragmentShader fs(L"phong.fragmentshader");
	GeometryShader gs(L"marchingCubes.geometryshader");

	const char *captured[] = {
		"position_model",
		"normal_model"
	};

	marchingCubesTF.reset(new GPUProgram(vs, fs, gs, 2, captured, GL_INTERLEAVED_ATTRIBS));

	marchingCubes.reset(new GPUProgram(vs, fs, gs));

	marchingCubes->bind();
	glUniformMatrix4fv((*marchingCubes)["world"], 1, GL_FALSE, &worldMat[0][0]);
	glUniformMatrix4fv((*marchingCubes)["view"], 1, GL_FALSE, &viewMat[0][0]);
	glUniformMatrix4fv((*marchingCubes)["projection"], 1, GL_FALSE, &projMat[0][0]);
	glUniform3i((*marchingCubes)["dims"], dim, dim, dim);
	glUniform3fv((*marchingCubes)["eyePos"], 1, &eye.x);


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

	marchingCubesTF->bind();
	glUniform1f((*marchingCubesTF)["iso"], -0.4f);
	triTableTex->BindToUniform((*marchingCubesTF)["tritable"]);
	volumeTex->BindToUniform((*marchingCubesTF)["volume"]);
	glUniformMatrix4fv((*marchingCubesTF)["world"], 1, GL_FALSE, &worldMat[0][0]);
	glUniformMatrix4fv((*marchingCubesTF)["view"], 1, GL_FALSE, &viewMat[0][0]);
	glUniformMatrix4fv((*marchingCubesTF)["projection"], 1, GL_FALSE, &projMat[0][0]);
	glUniform3i((*marchingCubesTF)["dims"], dim, dim, dim);
	glUniform3fv((*marchingCubesTF)["eyePos"], 1, &eye.x);

}

void CIsoSelectApp::SetIsoSurface(float value)
{
	marchingCubes->bind();
	glUniform1f((*marchingCubes)["iso"], value);
	marchingCubesTF->bind();
	glUniform1f((*marchingCubesTF)["iso"], value);
}


BOOL CIsoSelectApp::OnIdle(LONG lCount)
{
	if (m_pMainWnd->IsIconic())
		return FALSE;

	if (update)
	{
		LARGE_INTEGER perfFreq, start, end;
		::QueryPerformanceFrequency(&perfFreq);
		::QueryPerformanceCounter(&start);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		marchingCubes->bind();

		glBindVertexArray(vertexArray);
		glEnableVertexAttribArray(0);
		glPointSize(1.0f);
		glDrawArraysInstanced(GL_POINTS, 0, 1, dim * dim * dim);

		glFinish();
		::SwapBuffers(*m_pTheDialog->m_viewportCtl.GetDC());
		::QueryPerformanceCounter(&end);

		wchar_t buf[100];
		wsprintf(buf, L"Frame time: %d ms", (int)((end.QuadPart - start.QuadPart) / (perfFreq.QuadPart / 1000.0)));

		//AfxMessageBox(buf, MB_OK);

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

void CIsoSelectApp::SaveMesh(std::wstring path)
{
	static const GLsizeiptr max_ntriangles = dim * dim * dim * 5;
	static const GLsizeiptr max_nvertices = max_ntriangles * 3;
	static const GLsizeiptr max_nbytes = max_nvertices * (4 + 3) * sizeof(float);

	GLuint tfBuf, tfPrimQuery;
	glGenQueries(1, &tfPrimQuery);
	glGenBuffers(1, &tfBuf);
	glBindBuffer(GL_ARRAY_BUFFER, tfBuf);
	glBufferData(GL_ARRAY_BUFFER, max_nbytes, nullptr, GL_STATIC_DRAW);

	marchingCubesTF->bind();

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, tfPrimQuery);
	
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfBuf);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_TRIANGLES); 
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindVertexArray(vertexArray); 
	glDrawArraysInstanced(GL_POINTS, 0, 1, dim * dim * dim);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);


	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	GLuint nprimitives;
	glGetQueryObjectuiv(tfPrimQuery, GL_QUERY_RESULT, &nprimitives);

	struct
	{
		glm::vec4 position;
		glm::vec3 normal;
	}
	*vertex;

	static const float epsilon = 0.001f;

	struct vertexHash
	{
		inline size_t operator()(const glm::vec4& vertex) const
		{
			return std::hash<float>()(vertex.x) ^
				std::hash<float>()(vertex.y) ^ 
				std::hash<float>()(vertex.z);
		}
	};

	struct normalHash
	{
		inline size_t operator()(const glm::vec3& normal) const
		{
			return std::hash<float>()(normal.x) ^
				std::hash<float>()(normal.y) ^
				std::hash<float>()(normal.z);
		}
	};

	static struct vertexEquality
	{
		inline bool operator()(const glm::vec4& vert1, const glm::vec4& vert2) const
		{
			return (fabsf(vert1.x - vert2.x) <= epsilon) &&
				(fabsf(vert1.y - vert2.y) <= epsilon) &&
				(fabsf(vert1.z - vert2.z) <= epsilon);
		}
	};

	static struct normalEquality
	{
		inline bool operator()(const glm::vec3& norm1, const glm::vec3& norm2) const
		{
			return (fabsf(norm1.x - norm2.x) <= epsilon) &&
				(fabsf(norm1.y - norm2.y) <= epsilon) &&
				(fabsf(norm1.z - norm2.z) <= epsilon);
		}
	};

	std::unordered_map<glm::vec4, glm::uint32, vertexHash, vertexEquality> vertexIndexMap;
	std::unordered_map<glm::vec3, glm::uint32, normalHash, normalEquality> normalIndexMap;
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::uint32> indices;
	std::vector<glm::uint32> normalIndices;

	vertex = reinterpret_cast<decltype(vertex)>(glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY));

	glm::uint32 lastVertIndex = 0U;
	glm::uint32 lastNormalIndex = 0U;

	for (ptrdiff_t i = 0; i < nprimitives * 3; i++)
	{
		if (vertexIndexMap.find(vertex->position) == vertexIndexMap.end())
		{
			vertices.push_back(vertex->position);
			vertexIndexMap[vertex->position] = ++lastVertIndex;
			indices.push_back(lastVertIndex);
		}
		else
		{
			indices.push_back(vertexIndexMap[vertex->position]);
		}

		if (normalIndexMap.find(vertex->normal) == normalIndexMap.end())
		{
			normals.push_back(vertex->normal);
			normalIndexMap[vertex->normal] = ++lastNormalIndex;
			normalIndices.push_back(lastNormalIndex);
		}
		else
		{
			normalIndices.push_back(normalIndexMap[vertex->normal]);
		}

		vertex++;
	}

	std::ostringstream strOut;

	for (const auto& vert : vertices)
	{
		strOut << "v " << vert.x << " " << vert.y << " " << vert.z << "\n";
	}

	for (const auto& norm : normals)
	{
		strOut << "vn " << norm.x << " " << norm.y << " " << norm.z << "\n";
	}

	for (size_t face = 0U; face < nprimitives; face++)
	{
		const glm::uint32& vert1 = indices[face * 3];
		const glm::uint32& vert2 = indices[face * 3 + 1];
		const glm::uint32& vert3 = indices[face * 3 + 2];
		const glm::uint32& norm1 = normalIndices[face * 3];
		const glm::uint32& norm2 = normalIndices[face * 3 + 1];
		const glm::uint32& norm3 = normalIndices[face * 3 + 2];

		strOut << "f " << vert1 << "//" << norm1 << " " << vert2 << "//" << norm2 << " " << vert3 << "//" << norm3 << "\n";
	}

	std::ofstream fOut(path, std::ios::trunc);
	if (!fOut.is_open())
	{
		AfxMessageBox((L"Unable to open file: " + path).c_str(), MB_OK | MB_ICONEXCLAMATION);
	}
	else
	{
		fOut << strOut.str();
		fOut.close();
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	glDeleteBuffers(1, &tfBuf);
	glDeleteQueries(1, &tfPrimQuery);
}