
// IsoSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IsoSelect.h"
#include "IsoSelectDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIsoSelectDlg dialog



CIsoSelectDlg::CIsoSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIsoSelectDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hGLRenderContext = nullptr;
}

void CIsoSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIEWPORT, m_viewportCtl);
}

BEGIN_MESSAGE_MAP(CIsoSelectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CIsoSelectDlg message handlers

BOOL CIsoSelectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

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
		fprintf(stderr, "Failed to initialize GLEW\n");

		getchar();
		exit(-1);
	}

	glClearColor(0.4, 0.7f, 0.9f, 1.0f);

	theApp.InitScene();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIsoSelectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIsoSelectDlg::OnPaint()
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
HCURSOR CIsoSelectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIsoSelectDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	wglMakeCurrent(m_viewportCtl.GetDC()->GetSafeHdc(), NULL);
	wglDeleteContext(m_hGLRenderContext);
	m_hGLRenderContext = nullptr;
}


void CIsoSelectDlg::OnSize(UINT nType, int cx, int cy)
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


void CIsoSelectDlg::OnOK()
{
	// this overridden function must be left blank
}


void CIsoSelectDlg::OnCancel()
{
	// this must be called otherwise clicking the X in the window bar will not work
	PostQuitMessage(0);
}
