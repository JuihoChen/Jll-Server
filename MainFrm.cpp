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
	ON_WM_CLOSE()
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
	_OutputDebugString( "CMainFrame constructor...\n" );

	m_pTheServer = 0;
	m_nTimerDetectGuest = 0;
}

CMainFrame::~CMainFrame()
{
	_OutputDebugString( "CMainFrame::Destructor called.\n" );
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
	_OutputDebugString( "Main frame>PreCreateWindow.\n" );
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
		MessageBeep( 0xFFFFFFFF );  // Beep

		GetMyApp()->m_lptNibble.EnterIdleCondition();

		if( !GetMyApp()->m_lptNibble.DetectTheGuest() )
			GetMyApp()->m_lptNibble.EnterIdleCondition();

		else
		{
			OnStopTimer( nTimerIdDetectGuest );
	 		GetMyApp()->FormatOutput( "the Guest is detected..." );
			GetActiveView()->GetDlgItem( IDC_BUTTON_FOR_DIR )->EnableWindow( FALSE );
			m_pTheServer->Begin( this );
		}
		break;
	}

	CFrameWnd::OnTimer(nIDEvent);
}

BOOL CMainFrame::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	OnStopTimer( nTimerIdAll );
	if( m_pTheServer )
	{
		delete m_pTheServer;
		m_pTheServer = 0;
	}
	if( m_ExceptDlg.m_hWnd )
	{
		m_ExceptDlg.DestroyWindow();
///		m_ExceptDlg.m_hWnd = 0;
	}
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
		TRACE0( "JLL Server: cannot allocate timer resource.\n" );
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
	switch( pMsg->message )
	{
	case UWM_COMMU_LOOP:
	case UWM_SERVER_END:
	 	GetMyApp()->FormatOutput( "the Guest is disconnected..." );
		GetActiveView()->GetDlgItem( IDC_BUTTON_FOR_DIR )->EnableWindow( TRUE );
		OnStartTimer( nTimerIdDetectGuest );
		return TRUE;

	case UWM_EXCEPT_BOX:
		if( m_ExceptDlg.m_hWnd ) {
			m_ExceptDlg.Invalidate( TRUE );
		}
		else {
			m_ExceptDlg.Create( IDD_EXCEPTBOX, this );
		}
		m_ExceptDlg.ShowWindow( SW_SHOW );
		m_ExceptDlg.UpdateWindow();
		return TRUE;

	case UWM_ADD_STRING:
		{
			CString* ps = (CString*) pMsg->lParam;
		 	GetMyApp()->FormatOutput( *ps );
			delete ps;
		}
		return TRUE;
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	_OutputDebugString( "Main frame>OnClose.\n" );
	
	if( m_pTheServer->IsRunning() )
	{
		// Play the system exclamation sound.
		MessageBeep( MB_ICONEXCLAMATION );
		// Create the message box. If the user clicks the Yes button,
		// destroy the main window. 
		if(	IDOK != MessageBox(
			"Server is still in connection with a Guest,\r\n"
			"Still continue to exit?",
			AfxGetAppName(),
			MB_ICONQUESTION | MB_OKCANCEL | MB_OK ) )
		{
			return;
		}
	}

	CFrameWnd::OnClose();
}
