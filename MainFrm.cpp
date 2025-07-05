// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Jll Server.h"
#include "Jll ServerDoc.h"
#include "Jll ServerView.h"
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
	ON_WM_SYSCOMMAND()
	ON_COMMAND(IDR_TASKBAR_MENU_SHOW, OnTaskbarMenuShow)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DISABLEWARMPOLL, OnUpdateEditDisablewarmpoll)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_CPY_PROGRS, OnCopyProgress)
	ON_MESSAGE(UWM_EXCEPT_BOX, OnExceptionBox)
	ON_MESSAGE(UWM_SERVER_END, OnServerEndJob)
	ON_MESSAGE(UWM_ADD_STRING, OnAddString)
	///ON_UPDATE_COMMAND_UI(ID_PERCENT_DONE, OnUpdatePercentDone)
	ON_COMMAND_RANGE(ID_DETECT_SPK_ON, ID_DETECT_SPK_OFF, OnDetectSpkOn)
	ON_REGISTERED_MESSAGE(UWM_ARE_YOU_ME, OnAreYouMe)
	ON_REGISTERED_MESSAGE(UWM_TASKBAR_CREATED, OnTaskBarCreated)
	ON_REGISTERED_MESSAGE(UWM_NOTIFY_ICON, OnNotifyIcon)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_PERCENT_DONE,
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

void CMainFrame::FormatOutput( LPCTSTR lpszFormat, ... )
{
	// Find the document owned by this application.
	CJllServerDoc* pDoc = (CJllServerDoc*)GetActiveDocument();
	ASSERT_KINDOF( CJllServerDoc, pDoc );

	ASSERT( AfxIsValidString( lpszFormat ) );
	va_list argList;
	va_start( argList, lpszFormat );
	pDoc->FormatOutputV( lpszFormat, argList );
	va_end( argList );
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	_OutputDebugString( "Main frame>OnCreate.\n" );

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadBitmap(IDR_MAINFRAME))
		////!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
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

	OnDetectSpkOn( GetMyApp()->m_bDetectSpkOn ? ID_DETECT_SPK_ON : ID_DETECT_SPK_OFF );

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Reserve adequate space for a progress bar.
	int nIndex, cxWidth;
	UINT nID, nStyle;
	nIndex = m_wndStatusBar.CommandToIndex( ID_PERCENT_DONE );
	m_wndStatusBar.GetPaneInfo( nIndex, nID, nStyle, cxWidth );
	m_wndStatusBar.SetPaneInfo( nIndex, nID, nStyle, cxWidth * 5 );
	///m_cProgressBar.Create( _T("100%"), 100, 100, TRUE, nIndex );

	// Prepare data to system tray.
	SetupTrayIcon();

	// Put the version string in about box onto the frame title.
	CString sz;
	CDialog aboutDlg;
	aboutDlg.Create( IDD_ABOUTBOX, this );
	aboutDlg.GetDlgItemText( IDC_STATIC_VERSION, sz );
	aboutDlg.DestroyWindow();
	SetTitle( sz );

	return 0;
}

void CMainFrame::SetupTrayIcon()
{
	MouseMsgHandlerPtr *aHandler = new MouseMsgHandlerPtr[2];

	aHandler[0] = new CLeftMouseDblClickMsgHandler();
	aHandler[1] = new CRightMouseClickMsgHandler( IDR_TASKBAR_MENU );

	m_hIconDisconnect = ::LoadIcon( ::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_DISCONN) );
	m_hIconConnected  = ::LoadIcon( ::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_CONNECT) );

	SetIcon( m_hIconDisconnect, TRUE );
	SetIcon( m_hIconDisconnect, FALSE );

	m_cTrayIcon.SetTrayIcon( m_hWnd, IDR_MAINFRAME, m_hIconDisconnect, AfxGetAppName() );
	m_cTrayIcon.SetMouseMsgHandler( aHandler, 2 );
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	_OutputDebugString( "Main frame>PreCreateWindow.\n" );

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	///cs.style &= ~FWS_ADDTOTITLE;

	return TRUE;
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
	if( IsWindow( m_ExceptDlg.GetSafeHwnd() ) )
	{
		m_ExceptDlg.DestroyWindow();
		m_ExceptDlg.m_hWnd = 0;			// Invalidate Handle evev though redundant!
	}
	return CFrameWnd::DestroyWindow();
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
	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdatePercentDone(CCmdUI* pCmdUI)
{
	// It is necessary to enable the percent-done status bar pane, so that
	// the text will be displayed.
	pCmdUI->Enable();
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	switch( nIDEvent )
	{
	case nTimerIdDetectGuest:

		if( GetMyApp()->m_bDetectSpkOn )
		{
			MessageBeep( 0xFFFFFFFF );		// Beep
		}

		GetMyApp()->m_drvPortTalk.ParAllocPort();	// ask Parport.sys to allocate port
		GetMyApp()->m_lptNibble.EnterIdleCondition();

		if( !GetMyApp()->m_lptNibble.DetectTheGuest() )
		{
			GetMyApp()->m_lptNibble.EnterIdleCondition();
			GetMyApp()->m_drvPortTalk.ParFreePort();  // free parallel port from Parport.sys
		}
		else
		{
			OnStopTimer( nTimerIdDetectGuest );
	 		FormatOutput( "the Guest is detected... %s",
				CTime::GetCurrentTime().Format( "at %H:%M:%S on %A, %B %d, %Y" ) );
			GetActiveView()->EnableFolderChange( FALSE );
			m_pTheServer->Begin( this );
			m_cTrayIcon.ModifyIcon( m_hIconConnected, m_pTheServer->GetWorkDir() );
		}

		break;
	}

	CFrameWnd::OnTimer(nIDEvent);
}

UINT CMainFrame::OnStartTimer(UINT nIDEvent)
{
	UINT nTimer = 0;

	switch( nIDEvent )
	{
	case nTimerIdDetectGuest:
		if( m_nTimerDetectGuest == 0)
		{
			nTimer = m_nTimerDetectGuest = SetTimer( nTimerIdDetectGuest, 1000, NULL );
		}
		break;
	default:
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

void CMainFrame::CheckReTimerToDetectGuest()
{
	if( !m_pTheServer->IsRunning() && m_nTimerDetectGuest == 0 )
	{
		GetActiveView()->EnableFolderChange( TRUE );

		// Reenable timer to detect the guest after this Dialog.
		OnStartTimer( nTimerIdDetectGuest );	
	}
}

LRESULT CMainFrame::OnExceptionBox( WPARAM wParam, LPARAM lParam )
{
	if( IsWindow( m_ExceptDlg.GetSafeHwnd() ) ) {
		m_ExceptDlg.Invalidate( TRUE );
	}
	else {
		m_ExceptDlg.Create( IDD_EXCEPTBOX, this );
	}
	m_ExceptDlg.UpdateData( FALSE );			// Update data in CEdit control.
	m_ExceptDlg.ShowWindow( SW_SHOW );
	m_ExceptDlg.SetForegroundWindow();
	m_ExceptDlg.UpdateWindow();
	return 0;
}

LRESULT CMainFrame::OnServerEndJob( WPARAM wParam, LPARAM lParam )
{
	FormatOutput( "the Guest is disconnected... %s", CTime::GetCurrentTime().Format( "at %H:%M:%S" ) );
	GetActiveView()->EnableFolderChange( TRUE );
	GetMyApp()->m_drvPortTalk.ParFreePort();	// free parallel port from Parport.sys
///v0.18***	OnStartTimer( nTimerIdDetectGuest );
	m_cTrayIcon.ModifyIcon( m_hIconDisconnect, m_pTheServer->GetWorkDir() );
	return 0;
}

LRESULT CMainFrame::OnAddString( WPARAM wParam, LPARAM lParam )
{
	CString* ps = (CString*) lParam;
 	FormatOutput( *ps );
	delete ps;
	return 0;
}

LRESULT CMainFrame::OnCopyProgress( WPARAM wParam, LPARAM lParam )
{
	static DWORD dwFileSize = 0;
	UINT nPercentDone = 100;

	if( wParam == 0 )
	{
		dwFileSize = lParam;
		nPercentDone = 0;
	}
	else if( dwFileSize > (DWORD) lParam )
	{
		nPercentDone = (lParam * 100) / dwFileSize;
	}

	CString strPercentDone;
	strPercentDone.Format( "%i%%", nPercentDone );
	//m_wndStatusBar.SetPaneText( m_wndStatusBar.CommandToIndex( ID_PERCENT_DONE ), strPercentDone );
	//m_wndStatusBar.UpdateWindow();
	if( !IsWindow( m_cProgressBar.GetSafeHwnd() ) )
	{
		m_cProgressBar.Create( _T("100%"), 100, 100, TRUE, m_wndStatusBar.CommandToIndex( ID_PERCENT_DONE ) );
	}
	m_cProgressBar.SetText( strPercentDone );
	m_cProgressBar.SetPos( nPercentDone );

	return 0;
}

// Registered message (messages >= 0xC000) must be handled in a MESSAGE_MAP entry.
// Routine PreTranslateMessage cannot intercept this cateory of messages.
LRESULT CMainFrame::OnAreYouMe( WPARAM, LPARAM )
{
	return UWM_ARE_YOU_ME;
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
		CString s = "Server is still in connection with a Guest,\r\n"
					"Still continue to exit?";
		if(	IDOK != MessageBox( s, AfxGetAppName(), MB_ICONQUESTION | MB_OKCANCEL | MB_OK ) )
			return;
	}

	CFrameWnd::OnClose();
}

void CMainFrame::OnDetectSpkOn(UINT nID) 
{
	// Set the toolbar to show only partial commmand list
	m_wndToolBar.SetButtons( NULL, 11 );
	
	m_wndToolBar.SetButtonInfo( 0, ID_FILE_NEW, TBBS_BUTTON, 0 );
	m_wndToolBar.SetButtonInfo( 1, ID_FILE_OPEN, TBBS_BUTTON, 1 );
	m_wndToolBar.SetButtonInfo( 2, ID_FILE_SAVE, TBBS_BUTTON, 2 );
	m_wndToolBar.SetButtonInfo( 3, ID_SEPARATOR, TBBS_SEPARATOR, 6 );
	m_wndToolBar.SetButtonInfo( 4, ID_EDIT_CUT, TBBS_BUTTON, 3 );
	m_wndToolBar.SetButtonInfo( 5, ID_EDIT_COPY, TBBS_BUTTON, 4 );
	m_wndToolBar.SetButtonInfo( 6, ID_EDIT_PASTE, TBBS_BUTTON, 5 );
	m_wndToolBar.SetButtonInfo( 7, ID_SEPARATOR, TBBS_SEPARATOR, 6 );
	m_wndToolBar.SetButtonInfo( 8, ID_APP_ABOUT, TBBS_BUTTON, 7 );
	m_wndToolBar.SetButtonInfo( 9, ID_SEPARATOR, TBBS_SEPARATOR, 6 );

	if( nID == ID_DETECT_SPK_ON )
	{
		m_wndToolBar.SetButtonInfo( 10, ID_DETECT_SPK_OFF, TBBS_BUTTON, 9 );
		GetMyApp()->m_bDetectSpkOn = 1;
	}
	else
	{
		m_wndToolBar.SetButtonInfo( 10, ID_DETECT_SPK_ON, TBBS_BUTTON, 8 );
		GetMyApp()->m_bDetectSpkOn = 0;
	}

	// invalidate the call update handlers before painting
	m_wndToolBar.Invalidate();	
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
	// In WM_SYSCOMMAND messages, the four low-order bits of the nID parameter are
	// used internally by Windows. When an application tests the value of nID, it
	// must combine the value 0xFFF0 with the nID value by using the bitwise-AND
	// operator to obtain the correct result.
	if( (nID & 0xFFF0) == SC_MINIMIZE )
	{
		HICON hIcon = m_pTheServer->IsRunning() ? m_hIconConnected : m_hIconDisconnect;
		m_cTrayIcon.HideWindow( hIcon );
	}
	else
		CFrameWnd::OnSysCommand( nID, lParam );
}

LRESULT CMainFrame::OnTaskBarCreated(WPARAM wParam, LPARAM lParam)
{
	_OutputDebugString( "Main frame>OnTaskBarCreated.\n" );

	return m_cTrayIcon.OnTaskBarCreated( wParam, lParam );
}

void CMainFrame::OnNotifyIcon(WPARAM wParam, LPARAM lParam) 
{
	m_cTrayIcon.OnNotifyIcon( wParam, lParam );
}

void CMainFrame::OnTaskbarMenuShow() 
{
	m_cTrayIcon.RestoreWindow();
}

void CMainFrame::OnUpdateEditDisablewarmpoll(CCmdUI* pCmdUI) 
{
	DWORD myData = 0;
	HKEY phkResult;
	CString myKey;
	myKey.LoadString( ID_EDIT_DISABLEWARMPOLL );
	myKey = myKey.Mid( myKey.Find( '\\' ) + 1 );
	LONG res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey, 0, KEY_READ, &phkResult );
	if( res == ERROR_SUCCESS )
	{
		DWORD mySize = sizeof myData;
		DWORD myType = REG_DWORD;

		res = RegQueryValueEx(
				phkResult,					// handle to key to query
				"DisableWarmPoll",			// address of name of value to query
				NULL,						// reserved
				&myType,					// address of buffer for value type
				(LPBYTE)&myData,			// address of data buffer
				&mySize	);					// address of data buffer size

		if( res != ERROR_SUCCESS )
			myData = 0;
 	}
	RegCloseKey( phkResult );
	pCmdUI->SetCheck( myData != 0 );
}
