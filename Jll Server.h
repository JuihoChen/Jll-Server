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
#include "PortTalk.h"	// Added by ClassView
#include "parallel.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CJllServerApp:
// See Jll Server.cpp for the implementation of this class
//

class CJllServerApp : public CWinApp
{
	DECLARE_DYNCREATE(CJllServerApp)
public:
	static BOOL CALLBACK Searcher(HWND hWnd, LPARAM lParam);
	BOOL AvoidMultipleInstances() const;
	void StoreProfileStrings(int nIndex = 0, LPCSTR pzText = NULL);
	CString LoadProfileStrings(int nIndex = 0);
	CJllServerApp();

	CString m_sStartingDir;
	BOOL m_bDetectSpkOn;
	CNibbleModeProto m_lptNibble;
	CPortTalk m_drvPortTalk;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJllServerApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CJllServerApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	afx_msg void OnEditDisablewarmpoll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CJllServerApp theApp;

#ifdef _DEBUG
#define _OutputDebugString(lpsz) ::OutputDebugString(lpsz)
#else
#define _OutputDebugString(lpsz) ((void)0)
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JLLSERVER_H__B0E189B0_A03E_4B7B_8E6C_3E90F50E625A__INCLUDED_)
