// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__8EE8C44E_55F7_4D4D_85C9_16E2DC5F1A4B__INCLUDED_)
#define AFX_MAINFRM_H__8EE8C44E_55F7_4D4D_85C9_16E2DC5F1A4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "except.h"
#include "ProgressBar.h"
#include "TrayIcon.h"

#define UWM_CPY_PROGRS	(WM_APP+10)		// Message to report progress in copy
#define UWM_EXCEPT_BOX	(WM_APP+11)		// Message to popup a dialog for exception
#define UWM_SERVER_END	(WM_APP+12)		// Message to indicate end of server thread
#define UWM_ADD_STRING	(WM_APP+13)		// Message to add string to CJllServerDoc

#define UWM_ARE_YOU_ME_MSG	_T("UWM_ARE_YOU_ME-{5DC84998-8E67-4369-B279-F77AB0B099B2}")

const UINT UWM_ARE_YOU_ME = ::RegisterWindowMessage( UWM_ARE_YOU_ME_MSG );

class CDCServer;
class CJllServerView;

class CMainFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CMainFrame)

protected: // create from serialization only
	CMainFrame();

// Attributes
public:
	enum {
		nTimerIdAll = -1,
		nTimerIdDetectGuest = 1
	};
	CDCServer* m_pTheServer;
	CExceptDlg m_ExceptDlg;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	CJllServerView* GetActiveView() const;
	void FormatOutput( LPCTSTR lpszFormat, ... );
	UINT StartTimer(UINT nIDEvent);
	void StopTimer(UINT nIDEvent);
	void CheckReTimerToDetectGuest();
	void SetupTrayIcon();
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar	m_wndStatusBar;
	CToolBar	m_wndToolBar;
	CProgressBar m_cProgressBar;
	CTrayIcon	m_cTrayIcon;
	HICON		m_hIconDisconnect, m_hIconConnected;
	UINT		m_nTimerDetectGuest;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnTaskbarMenuShow();
	afx_msg void OnUpdateEditDisablewarmpoll(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg void OnUpdatePercentDone(CCmdUI* pCmdUI);
	afx_msg void OnDetectSpkOn(UINT nID);
	afx_msg LRESULT OnExceptionBox(WPARAM, LPARAM);
	afx_msg LRESULT OnServerEndJob(WPARAM, LPARAM);
	afx_msg LRESULT OnAddString(WPARAM, LPARAM);
	afx_msg LRESULT OnCopyProgress(WPARAM, LPARAM);
	afx_msg LRESULT OnAreYouMe(WPARAM, LPARAM);
	afx_msg LRESULT OnTaskBarCreated(WPARAM, LPARAM);
	afx_msg void OnNotifyIcon(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

inline CMainFrame* GetMyMainFrame()
{
	CMainFrame* pFrame = (CMainFrame*)(GetMyApp()->m_pMainWnd);
	ASSERT_KINDOF( CMainFrame, pFrame );
	return (CMainFrame*)pFrame;
}

inline CJllServerView* CMainFrame::GetActiveView() const
{
	ASSERT_KINDOF( CJllServerView, m_pViewActive );
	return (CJllServerView*)m_pViewActive;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__8EE8C44E_55F7_4D4D_85C9_16E2DC5F1A4B__INCLUDED_)
