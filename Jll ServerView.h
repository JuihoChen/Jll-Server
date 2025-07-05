// Jll ServerView.h : interface of the CJllServerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_JLLSERVERVIEW_H__BDD13551_F6AB_429E_88B1_7093ED1DE02A__INCLUDED_)
#define AFX_JLLSERVERVIEW_H__BDD13551_F6AB_429E_88B1_7093ED1DE02A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


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
	CButton	m_cButtonForDir;
	CString	m_sStartingFolder;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void InvalidateForNewLine();
	CJllServerDoc* GetDocument();
	int GetLinesInView() const { return m_nNumLines; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJllServerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CJllServerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CJllServerView)
	afx_msg void OnButtonForDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_nTopForText;
	int m_nLineHeight;
	int m_nNumLines;
};

#ifndef _DEBUG  // debug version in Jll ServerView.cpp
inline CJllServerDoc* CJllServerView::GetDocument()
   { return (CJllServerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JLLSERVERVIEW_H__BDD13551_F6AB_429E_88B1_7093ED1DE02A__INCLUDED_)
