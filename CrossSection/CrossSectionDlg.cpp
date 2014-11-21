
// CrossSectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CrossSection.h"
#include "CrossSectionDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCrossSectionDlg dialog



CCrossSectionDlg::CCrossSectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCrossSectionDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hGLRenderContext = nullptr;
}

void CCrossSectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIEWPORT, m_viewportCtl);
}

BEGIN_MESSAGE_MAP(CCrossSectionDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CCrossSectionDlg message handlers

BOOL CCrossSectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//set up OpenGL context
	// Set the pixel format for this DC
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// struct size 
		1,						// Version number
		PFD_DRAW_TO_WINDOW |    // Flags, draw to a window,
		PFD_SUPPORT_OPENGL |    // use OpenGL
		PFD_DOUBLEBUFFER,		// double buffered
		PFD_TYPE_RGBA,          // RGBA pixel values
		32,                     // 32-bit color
		0, 0, 0,                // RGB bits & shift sizes.
		0, 0, 0,                // Don't care about them
		0, 0,                   // No alpha buffer info
		0, 0, 0, 0, 0,          // No accumulation buffer
		32,                     // 32-bit depth buffer
		0,                      // No stencil buffer
		0,                      // No auxiliary buffers
		PFD_MAIN_PLANE,         // Layer type
		0,                      // Reserved (must be 0)
		0,                      // No layer mask
		0,                      // No visible mask
		0                       // No damage mask
	};

	int nMyPixelFormatID = ChoosePixelFormat(*m_viewportCtl.GetDC(), &pfd);

	SetPixelFormat(*m_viewportCtl.GetDC(), nMyPixelFormatID, &pfd);

	m_hGLRenderContext = wglCreateContext(*m_viewportCtl.GetDC());
	wglMakeCurrent(*m_viewportCtl.GetDC(), m_hGLRenderContext);


	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		AfxMessageBox(_T("Failed to initialize GLEW\n"));

		getchar();
		exit(-1);
	}

	glClearColor(0.4, 0.7f, 0.9f, 1.0f);

	theApp.InitScene();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCrossSectionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCrossSectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCrossSectionDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	wglMakeCurrent(m_viewportCtl.GetDC()->GetSafeHdc(), NULL);
	wglDeleteContext(m_hGLRenderContext);
	m_hGLRenderContext = nullptr;
}


void CCrossSectionDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_hGLRenderContext)
	{
		//m_viewportCtl.MoveWindow(10, 10, (cx * 4) / 5, cy - 20);

		CRect rc;
		m_viewportCtl.GetClientRect(&rc);

		glViewport(0, 0, rc.Width(), rc.Height());

		theApp.update = true;
	}
}


void CCrossSectionDlg::OnOK()
{
	// this overridden function must be left blank
}


void CCrossSectionDlg::OnCancel()
{
	// this must be called otherwise clicking the X in the window bar will not work
	PostQuitMessage(0);
}
