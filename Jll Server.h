// Jll Server.h : main header file for the JLL SERVER application
//

#if !defined(AFX_JLLSERVER_H__B0E189B0_A03E_4B7B_8E6C_3E90F50E625A__INCLUDED_)
#define AFX_JLLSERVER_H__B0E189B0_A03E_4B7B_8E6C_3E90F50E625A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CJllServerApp:
// See Jll Server.cpp for the implementation of this class
//

class CJllServerApp : public CWinApp
{
public:
	void FormatOutput(LPCTSTR lpszFormat, ...);
	void StoreProfileStrings();
	void LoadProfileStrings();
	CJllServerApp();
	DECLARE_DYNCREATE(CJllServerApp)

	CString m_sStartingDir;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJllServerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CJllServerApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline CJllServerApp* GetMyApp()
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_KINDOF( CJllServerApp, pApp );
	return (CJllServerApp*)pApp;
}

#ifdef _DEBUG
#define _OutputDebugString(lpsz) ::OutputDebugString(lpsz)
#else
#define _OutputDebugString(lpsz) ((void)0)
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JLLSERVER_H__B0E189B0_A03E_4B7B_8E6C_3E90F50E625A__INCLUDED_)
