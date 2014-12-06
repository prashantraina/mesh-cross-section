
// IsoSelectDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


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
	CPoint dragAnchor;

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
	CSliderCtrl m_coarseSlider;
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	float m_selectedIso;
	afx_msg void OnBnClickedSaveObjButton();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
