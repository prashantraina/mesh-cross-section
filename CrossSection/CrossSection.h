
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
	GLuint indexBuffer;
	size_t numIndices;
	glm::uint32 numPlanes;
	glm::mat4 worldMat, viewMat, projMat;
	glm::vec3 eyePos;
	std::unique_ptr<GPUProgram> phongShading;
	std::unique_ptr<GPUProgram> crossSectionShader;
	static const glm::uint32 maxPlanes = 15;
	std::unique_ptr<glm::vec3[]> planeNormals;
	std::unique_ptr<glm::vec3[]> planePoints;

	void InitScene();
	void LoadMesh(const aiMesh * const *meshes, size_t numMeshes);
	void SaveCrossSections(std::wstring path);

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance();
};

extern CCrossSectionApp theApp;