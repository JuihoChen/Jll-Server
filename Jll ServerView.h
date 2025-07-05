// Jll ServerView.h : interface of the CJllServerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_JLLSERVERVIEW_H__BDD13551_F6AB_429E_88B1_7093ED1DE02A__INCLUDED_)
#define AFX_JLLSERVERVIEW_H__BDD13551_F6AB_429E_88B1_7093ED1DE02A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ButtonStyle.h"

/////////////////////////////////////////////////////////////////////////////
// CJllServerView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CJllServerView : public CFormView
{
	DECLARE_DYNCREATE(CJllServerView)
protected:
	CJllServerView();           // protected constructor used by dynamic creation

// Form Data
public:
	//{{AFX_DATA(CJllServerView)
	enum { IDD = IDD_DIALOGBAR };
	CBitmapButton m_cButtonSetDir;
	CButtonStyle m_cButtonResetDir;
	CButton	m_cGroupFrame;
	CButton	m_cButtonForDir;
	CString	m_sStartingFolder;
	//}}AFX_DATA

// Attributes
public:
	CToolTipCtrl m_cToolTip;

// Operations
public:
	void EnableFolderChange(BOOL bEnable = TRUE);
	void InvalidateForNewLine();
	CJllServerDoc* GetDocument();
	int GetLinesInView() const { return m_nNumLines; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJllServerView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CJllServerView();
	static INT CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp,LPARAM pData);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CJllServerView)
	afx_msg void OnButtonForDir();
	afx_msg void OnButtonResetDir();
	afx_msg void OnButtonSetDir();
	//}}AFX_MSG
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pTTTStruct, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
private:
	int m_nTopForText;
	int m_nLineHeight;
	int m_nNumLines;
	BOOL m_f1stInitialUpdate;
};

#ifndef _DEBUG  // debug version in Jll ServerView.cpp
inline CJllServerDoc* CJllServerView::GetDocument()
   { return (CJllServerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JLLSERVERVIEW_H__BDD13551_F6AB_429E_88B1_7093ED1DE02A__INCLUDED_)
