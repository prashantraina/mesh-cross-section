
// IsoSelectDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CIsoSelectDlg dialog
class CIsoSelectDlg : public CDialogEx
{
// Construction
public:
	CIsoSelectDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ISOSELECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	HGLRC m_hGLRenderContext;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// The picture control which holds the OpenGL viewport
	CStatic m_viewportCtl;
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	virtual void OnCancel();
};
