// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Jll Server.h"
#include "MainFrm.h"
#include "direct.h"
#include <conio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	_OutputDebugString( "CMainFrame constructor..." );

	m_nTimerDetectGuest = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	_OutputDebugString( "Main frame>PreCreateWindow." );
	///cs.style |= WS_VSCROLL;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	switch( nIDEvent )
	{
	case nTimerIdDetectGuest:
		MessageBeep(0xFFFFFFFF);  // Beep

		GetMyApp()->m_lptNibble.EnterIdleCondition();

		if( !GetMyApp()->m_lptNibble.DetectTheGuest() )
			GetMyApp()->m_lptNibble.EnterIdleCondition();

		else
		{
			OnStopTimer( nTimerIdDetectGuest );
			PostMessage( WM_USER_COMMLOOP );
			GetMyApp()->m_pTheServer->Begin();
		}
		break;
	}

	CFrameWnd::OnTimer(nIDEvent);
}

BOOL CMainFrame::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	OnStopTimer( nTimerIdAll );
	return CFrameWnd::DestroyWindow();
}

UINT CMainFrame::OnStartTimer(UINT nIDEvent)
{
	UINT nTimer;

	switch( nIDEvent )
	{
	case nTimerIdDetectGuest:
		nTimer = m_nTimerDetectGuest = SetTimer( nTimerIdDetectGuest, 1000, NULL );
		break;
	default:
		nTimer = 0;
		break;
	}

	// Timers are a limited global resource; therefore it is important
	// that an application check the value returned by the SetTimer
	// method to verify that a timer is actually available.

	if( nTimer == 0 )
	{
		TRACE0( "JLL Server: cannot allocate timer resource." );
	}
	return nTimer;
}

void CMainFrame::OnStopTimer(UINT nIDEvent)
{
	switch( nIDEvent )
	{
	case nTimerIdAll:
		if( m_nTimerDetectGuest ) KillTimer( m_nTimerDetectGuest );
		m_nTimerDetectGuest = 0;
		break;
	case nTimerIdDetectGuest:
		if( m_nTimerDetectGuest ) KillTimer( m_nTimerDetectGuest );
		m_nTimerDetectGuest = 0;
		break;
	}
}

/****************************************************************************
	Function:	PreTranslateMessage()
	Purpose:	Look for the message that indicates the path has changed so
				the edit box can be updated properly.
				NOTE: DON'T ADD YOUR OWN MESSAGE HANDLER AND MESSAGE MAP ENTRY.
				For whatever reason, this causes an access violation crash.
				If you do it this way, everything works just fine.
	Inputs:		MSG* pMsg -- The message information
	Outputs:	BOOL -- Whether or not the operation succeeded
****************************************************************************/
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if( WM_USER_COMMLOOP == pMsg->message )
	{
		GetMyApp()->FormatOutput( "the Guest is detected..." );
		return TRUE;
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}
