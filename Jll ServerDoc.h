// Jll ServerDoc.h : interface of the CJllServerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_JLLSERVERDOC_H__2C2408AA_FF57_4C74_AE7F_DB0672A2ADDC__INCLUDED_)
#define AFX_JLLSERVERDOC_H__2C2408AA_FF57_4C74_AE7F_DB0672A2ADDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CJllServerDoc : public CDocument
{
protected: // create from serialization only
	CJllServerDoc();
	DECLARE_DYNCREATE(CJllServerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJllServerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	void FormatOutput(LPCTSTR lpszFormat, ...);
	void FormatOutputV(LPCTSTR lpszFormat, va_list argList);
	virtual ~CJllServerDoc();
	CStringList m_slMessages;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CJllServerDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JLLSERVERDOC_H__2C2408AA_FF57_4C74_AE7F_DB0672A2ADDC__INCLUDED_)
