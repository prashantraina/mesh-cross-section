
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

CCrossSectionApp::CCrossSectionApp() : update(true), vertexArray(0), vertexBuffer(0), indexBuffer(0)
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

	projMat = glm::perspective(45.0f, 4.0f / 3.0f, 0.01f, 20.0f);

	glm::mat4 viewProj = projMat * viewMat;

	VertexShader phongVS(L"phong.vertexshader");
	VertexShader crossSecVS(L"cross_sec.vertexshader");
	GeometryShader crossSecGS(L"cross_sec.geometryshader");
	FragmentShader phongFS(L"phong.fragmentshader");
	FragmentShader solidFS(L"solidcolor.fragmentshader");

	const char *captured[] = {
		"modelPos",
		"planeNumber"
	};

	crossSectionShader.reset(new GPUProgram(crossSecVS, solidFS, crossSecGS, 2, captured, GL_INTERLEAVED_ATTRIBS));

	planeNormals.reset(new glm::vec3[maxPlanes]);
	planePoints.reset(new glm::vec3[maxPlanes]);

	planeNormals[0] = glm::vec3(0, 0, 1);
	planePoints[0] = glm::vec3(0, 0, .1);
	planeNormals[1] = glm::vec3(0, 1, 0);
	planePoints[1] = glm::vec3(0, 0, 0);
	planeNormals[2] = glm::vec3(1, 0, 0);
	planePoints[2] = glm::vec3(0, 0, 0);
	planeNormals[3] = glm::vec3(1, 0, 0);
	planePoints[3] = glm::vec3(0.5f, 0, 0);
	planeNormals[4] = glm::vec3(1, 0, 0);
	planePoints[4] = glm::vec3(-0.5, 0, 0);
	planeNormals[5] = glm::vec3(0, 1, 0);
	planePoints[5] = glm::vec3(0, 0.5, 0);
	planeNormals[6] = glm::vec3(0, 1, 0);
	planePoints[6] = glm::vec3(0, -0.5, 0);
	planeNormals[7] = glm::vec3(0, 0, 1);
	planePoints[7] = glm::vec3(0, 0, -0.5);

	crossSectionShader->bind();

	numPlanes = 8;

	glUniformMatrix4fv((*crossSectionShader)["world"], 1, GL_FALSE, &worldMat[0][0]);
	glUniformMatrix4fv((*crossSectionShader)["viewProj"], 1, GL_FALSE, &viewProj[0][0]);
	//glUniform3fv((*phongShading)["eyePos"], 1, &eyePos.x);
	glUniform3fv((*crossSectionShader)["planeNormals"], numPlanes, &planeNormals[0].x);
	glUniform3fv((*crossSectionShader)["planePoints"], numPlanes, &planePoints[0].x);
	glUniform1ui((*crossSectionShader)["numPlanes"], numPlanes);

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
			//phongShading->bind();
			crossSectionShader->bind();

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

void CCrossSectionApp::SaveCrossSections(std::wstring path)
{
	static const GLsizeiptr max_nlines = numIndices / 3;
	static const GLsizeiptr max_nvertices = max_nlines * 2;
	static const GLsizeiptr max_nbytes = max_nvertices * ((3 * sizeof(float)) + sizeof(glm::uint));

	GLuint tfBuf, tfPrimQuery;
	glGenQueries(1, &tfPrimQuery);
	glGenBuffers(1, &tfBuf);
	glBindBuffer(GL_ARRAY_BUFFER, tfBuf);
	glBufferData(GL_ARRAY_BUFFER, max_nbytes, nullptr, GL_STATIC_DRAW);

	crossSectionShader->bind();

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, tfPrimQuery);

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfBuf);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_LINES);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);


	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	GLuint nprimitives;
	glGetQueryObjectuiv(tfPrimQuery, GL_QUERY_RESULT, &nprimitives);
	
	struct
	{
		glm::vec3 position;
		glm::uint planeNumber;
	}
	*vertex;

	static const float epsilon = 0.0000f;

	struct vertexHash
	{
		inline size_t operator()(const glm::vec3& vertex) const
		{
			return std::hash<float>()(vertex.x) ^
				std::hash<float>()(vertex.y) ^
				std::hash<float>()(vertex.z);
		}
	};

	static struct vertexEquality
	{
		inline bool operator()(const glm::vec3& vert1, const glm::vec3& vert2) const
		{
			return (fabsf(vert1.x - vert2.x) <= epsilon) &&
				(fabsf(vert1.y - vert2.y) <= epsilon) &&
				(fabsf(vert1.z - vert2.z) <= epsilon);
		}
	};

	vertex = reinterpret_cast<decltype(vertex)>(glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY));

	std::unique_ptr<std::vector<glm::uint32>[]> planePoints(new std::vector<glm::uint32>[numPlanes]);
	std::unordered_map<glm::vec3, glm::uint32, vertexHash, vertexEquality> vertexIndexMap;
	std::vector<glm::vec3> vertices;
	std::vector<std::vector<glm::uint32>> faces;

	glm::uint32 lastVertIndex = 0U;


	std::ostringstream strOut;

	for (size_t i = 0; i < nprimitives * 2U; i++)
	{
		const glm::vec3& position = vertex->position;

		if (vertexIndexMap.find(position) == vertexIndexMap.end())
		{
			vertices.push_back(position);
			vertexIndexMap[position] = ++lastVertIndex;
			planePoints[vertex->planeNumber].push_back(lastVertIndex);
			strOut << "v " << position.x << " " << position.y << " " << position.z << "\n";
		}
		else
		{
			planePoints[vertex->planeNumber].push_back(vertexIndexMap[vertex->position]);
		}
		vertex++;
	}

	/*for (std::vector<glm::uint32> *vec = &planePoints[0]; vec < &planePoints[numPlanes]; vec++)
	{
		if (vec->size() == 0)
			continue;

		/*bool **adjacency = new bool*[vertices.size()];
		for (int i = 0; i < vertices.size(); i++)
			adjacency[i] = new bool[vertices.size()];

		for(

		for (int i = 0; i < vertices.size(); i++)
			delete adjacency[i];
		delete adjacency;*//*
		//std::unique_ptr<glm::uint32[]> neighborMap(new glm::uint32[vertices.size()]);
		//memset(neighborMap.get(), 0, sizeof(glm::uint32) * vertices.size());
		std::unordered_map<glm::uint32, std::unordered_set<glm::uint32>> neighborMap;

		for (size_t pairI = 0; pairI < vec->size() - 1; pairI += 2)
		{
			glm::uint32& vert1 = (*vec)[pairI];
			glm::uint32& vert2 = (*vec)[pairI + 1];

			if (neighborMap.count(vert1) == 0)
			{
				neighborMap[vert1] = std::unordered_set<glm::uint32>();
			}
			if (neighborMap.count(vert2) == 0)
			{
				neighborMap[vert2] = std::unordered_set<glm::uint32>();
			}
			neighborMap[vert1].insert(vert2);
			neighborMap[vert2].insert(vert1);
		}

		std::vector<glm::uint32> sortedPoints;
		//std::unique_ptr<bool[]> traversed(new bool[vec->size()]);
		//memset(traversed.get(), 0, vec->size() * sizeof(bool));
		std::unordered_set<glm::uint32> traversed;

		for (size_t i = 0; i < vec->size(); i++)
		{
			if (traversed.count((*vec)[i]) > 0)
				continue;

			glm::uint32 vert = (*vec)[i];
			glm::uint32 prevVert = (*vec)[i];
			//size_t travIndex = i;
			do
			{
				sortedPoints.push_back(vert);
				traversed.insert(vert);
				std::vector<glm::uint32> neighbors(2);
				if (neighborMap.count(vert) == 0)
				{
					vert = 0;
					break;
				}
				auto& set = neighborMap[vert];
				if (set.size() == 1 && neighbors[0] == prevVert)
				{
					vert = 0;
					break;
				}
				std::copy(set.cbegin(), set.cend(), neighbors.begin());
				glm::uint32 nextVert = neighbors[0] == prevVert ? neighbors[1] : neighbors[0];
				prevVert = vert;
				vert = nextVert;
				//travIndex = vert - 1;
				//vert = neighborMap[travIndex];
			} while (vert != 0 && vert != (*vec)[i]);

			if (vert != 0 && sortedPoints.size() > 0)
			{
				faces.push_back(sortedPoints);
			}
			sortedPoints.clear();
		}

	}*/


	for (size_t i = 0; i < numPlanes; i++)
	{
		const auto& points = planePoints[i];
		if (points.empty())
			continue;

		assert(points.size() % 2 == 0);

		strOut << "g plane " << i << "\n";

		glm::vec3 sum(0);

		for (size_t j = 0; j < points.size() - 1; j += 2)
		{
			const auto& point1 = points[j];
			const auto& point2 = points[j + 1];
			sum += vertices[point1];
			sum += vertices[point2];

			strOut << "l " << point1 << " " << point2 << "\n";
		}

		glm::vec3 centroid = sum / (float)points.size();
		glm::uint32 centroidIndex;

		if (vertexIndexMap.find(centroid) == vertexIndexMap.end())
		{
			vertices.push_back(centroid);
			vertexIndexMap[centroid] = ++lastVertIndex;
			centroidIndex = lastVertIndex;
			strOut << "v " << centroid.x << " " << centroid.y << " " << centroid.z << "\n";
		}
		else
		{
			centroidIndex = vertexIndexMap[centroid];
		}

		strOut << "p " << centroidIndex << "\n";
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