#if !defined(AFX_BUT_H__6B04ABE1_44A1_11D6_B990_86147FA52342__INCLUDED_)
#define AFX_BUT_H__6B04ABE1_44A1_11D6_B990_86147FA52342__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ButtonStyle .h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CButtonStyle window

class CButtonStyle : public CButton
{
// Construction
public:
	CButtonStyle();
	virtual ~CButtonStyle();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonStyle)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetIcons(UINT, UINT, UINT, UINT);
	HICON m_hIcon1, m_hIcon2, m_hIcon3, m_hIcon4;
	HICON m_hSave;

	// Generated message map functions
protected:
	//{{AFX_MSG(CButtonStyle)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_BUT_H__6B04ABE1_44A1_11D6_B990_86147FA52342__INCLUDED_)
