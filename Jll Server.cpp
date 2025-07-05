// Jll Server.cpp : Defines the class behaviors for the application.
//
//     Copyright (c) 2004, Compal Electronics, Inc.  All rights reserved.
//
//     Ver.    Date             Logs
//     -----   ------------   -------------
//     v0.00   OCT 01, 2004   to create.
//     v0.10   OCT 19, 2004   derive from CFileDialog to create a new class that allows
//                            the user to select a directory. (adapted from DIRPK)
//     v0.11   OCT 22, 2004   create a worker thread (background) as the Server.
//     v0.12   NOV 01, 2004   complete XP-DOS communication, and keep on debugging.
//     v0.13   NOV 01, 2004   some fast-computers (like small tank) have to delay more.
//     v0.14   NOV 03, 2004   debug the communication tie and add some new features.
//     v0.15   NOV 05, 2004   realize m_nPollCounter as a counter instead of a flag.
//     v0.16   NOV 08, 2004   found contention with some parallel interlink of XP.
//             NOV 10, 2004   fix assertion caused by some old MS-DOS files.
//                            fix for some text not displayed when scrolling bar acts.
//                            add CRC feature to "Send Data" command.
//             NOV 11, 2004   put the version string in about box onto the frame title.
//     v0.17   NOV 16, 2004   avoid multiple instances (codes from Joseph M. Newcomer).
//             NOV 17, 2004   create an event notifying worker thread been killed.
//             NOV 22, 2004   add ability for timeout check in block transfer.
//     v0.18   NOV 25, 2004   drop-in status bar progress control (from Chris Maunder).
//                            not calling PreTranslateMessage makes WM_TIMER workable.
//             NOV 28, 2004   ports transparent bitmap buttons (from bhushan_at ).
//             NOV 29, 2004   add tooltips to buttons by two different approaches.
//             DEC 03, 2004   a system tray control is added to CFrameWnd.
//     v0.19   DEC 06, 2004   use a different icon for SetIcon.
//             DEC 09, 2004   allocate parallel port (parport.sys) via PortTalk driver.
//             DEC 27, 2004   add option in menu to edit "DisableWarmPoll" registry setting.

#include "stdafx.h"
#include "Jll Server.h"
#include "Jll ServerDoc.h"
#include "Jll ServerView.h"
#include "MainFrm.h"
#include "parallel.h"
#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJllServerApp

IMPLEMENT_DYNCREATE(CJllServerApp, CWinApp)

BEGIN_MESSAGE_MAP(CJllServerApp, CWinApp)
	//{{AFX_MSG_MAP(CJllServerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_EDIT_DISABLEWARMPOLL, OnEditDisablewarmpoll)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// The one and only CJllServerApp object

CJllServerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CJllServerApp construction

CJllServerApp::CJllServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	_OutputDebugString( "We start here...\n" );
}

/////////////////////////////////////////////////////////////////////////////
// Some coded copied from articles of The Code Project
BOOL CALLBACK CJllServerApp::Searcher( HWND hWnd, LPARAM lParam )
{
	DWORD result;
	LRESULT ok = ::SendMessageTimeout(
		hWnd,
		UWM_ARE_YOU_ME,
		0, 0,
		SMTO_BLOCK /*| SMTO_ABORTIFHUNG*/ | SMTO_NOTIMEOUTIFNOTHUNG,
		200,
		&result );

	if( ok == 0 )
		return TRUE;	// ignore this and continue
	if( result == UWM_ARE_YOU_ME )
	{ /* found it */
		*(HWND*)lParam = hWnd;
		return FALSE;	// stop search
	} /* found it */
	return TRUE;		// continue search
}

BOOL CJllServerApp::AvoidMultipleInstances() const
{
	HANDLE hMutexOneInstance = ::CreateMutex( NULL, FALSE, UWM_ARE_YOU_ME_MSG );
	BOOL bAlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS ||
							GetLastError() == ERROR_ACCESS_DENIED);

	// The call fails with ERROR_ACCESS_DENIED if the Mutex was
	// created in a different users session because of passing
	// NULL for the SECURITY_ATTRIBUTES on Mutex creation);

	if( bAlreadyRunning )
	{ /* kill this */
		HWND hOther = NULL;
		EnumWindows( Searcher, (LPARAM) &hOther );

		if( hOther != NULL )
		{ /* pop up */
			::SetForegroundWindow( hOther );

			if( ::IsIconic( hOther ) )
			{ /* restore */
				::ShowWindow( hOther, SW_RESTORE );
			} /* restore */

			else if( !::IsWindowVisible( hOther ) )
			{ /* restore from system tray */
				DWORD result;
				LRESULT ok = ::SendMessageTimeout(
					hOther,
					UWM_NOTIFY_ICON,
					IDR_MAINFRAME,
					WM_LBUTTONDBLCLK,
					SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG,
					200,
					&result );
			} /* restore from system tray */
		} /* pop up */

		// Display something that tells the user
		// the app is already running.
		///AfxMessageBox( "Application is already running." );

		return FALSE;
	} /* kill this */

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CJllServerApp initialization

BOOL CJllServerApp::InitInstance()
{
    CString s;

	// Check only running on WinXP/NT systems.
	if( GetVersion() & 0x80000000 )
	{
		s = "This application is only allowed to be running on Windows XP or NT.";
		MessageBox( NULL, s, AfxGetAppName(), MB_ICONSTOP | MB_OK );
		return FALSE; // and fail
	}

	// Only one instance is allowed to be running.
	if( !AvoidMultipleInstances() )
		return FALSE;

	// Check if the parallel port setup okay?
	m_lptNibble.Setup();

	if( !m_lptNibble.PortIsPresent() )
	{
		s = "This application needs a printer port on the system.";
		MessageBox( NULL, s, AfxGetAppName(), MB_ICONSTOP | MB_OK );
		return FALSE; // and fail
	}

	// Try to open PortTalk driver to touch LPT ports...
	switch( m_drvPortTalk.OpenPortTalk() )
	{
	case CPortTalk::Success:
		break;
	case CPortTalk::InvalidHandleLPT1:
		s = "Couldn't get device object pointer to parallel port 0 (Parport.sys).",
		MessageBox( NULL, s, AfxGetAppName(), MB_ICONSTOP | MB_OK );
		return FALSE;
	case CPortTalk::InvalidHandleValue:
		s = "Couldn't access PortTalk Driver.",
		MessageBox( NULL, s, AfxGetAppName(), MB_ICONSTOP | MB_OK );
		return FALSE;
	case CPortTalk::IoControlError:
		s = "Error in calling DeviceIoControl.",
		MessageBox( NULL, s, AfxGetAppName(), MB_ICONSTOP | MB_OK );
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Application for Laplink-like Server"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Change the current directory for the current process. 
	LoadProfileStrings();

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CJllServerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CJllServerView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	// CHANGE: Jll doesn't take command line parms to open documents
	//ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	_OutputDebugString( "CJllServerApp>Before MainWnd's Show.\n" );

	if( m_drvPortTalk.EnableIOPM( m_lptNibble.GetBaseAddr() ) != CPortTalk::Success )
	{
		s = "Error in IPOM setting for process.",
		MessageBox( NULL, s, AfxGetAppName(), MB_ICONSTOP | MB_OK );
		return FALSE;
	}

	((CMainFrame*)m_pMainWnd)->FormatOutput(
		"Found a printer port on 0x%x in the registry.", m_lptNibble.GetBaseAddr() );

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	m_pMainWnd->SetWindowPos( &CWnd::wndTopMost, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE );
	m_pMainWnd->SetWindowPos( &CWnd::wndNoTopMost, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE );
	m_pMainWnd->SetForegroundWindow();

	((CMainFrame*)m_pMainWnd)->m_pTheServer = new CDCServer( m_lptNibble );
	ASSERT_KINDOF( CDCServer, ((CMainFrame*)m_pMainWnd)->m_pTheServer );
	((CMainFrame*)m_pMainWnd)->m_pTheServer->ParseWorkDir( m_sStartingDir );
///v0.18***	((CMainFrame*)m_pMainWnd)->OnStartTimer( CMainFrame::nTimerIdDetectGuest );

	_OutputDebugString( "CJllServerApp>Leave App's InitInstance.\n" );
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CJllServerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CJllServerApp message handlers

void CJllServerApp::OnFileNew() 
{
	// TODO: Add your command handler code here
	_OutputDebugString( "theApp> ON FILE NEW.\n" );
	CWinApp::OnFileNew();
}

void CJllServerApp::LoadProfileStrings()
{
	CString strSection    = "Jll Section";
	CString strStringItem = "Starting Directory";
	CString strIntItem    = "Detecting Speaker";

	m_sStartingDir = GetProfileString( strSection, strStringItem, "C:\\" );
	if( ::SetCurrentDirectory( m_sStartingDir ) == FALSE )
	{
		char szPath[ 1024 ];			// Buffer to hold path information
		::GetCurrentDirectory( sizeof szPath, szPath );
		m_sStartingDir = szPath;
	}

	m_bDetectSpkOn = GetProfileInt( strSection, strIntItem, 0 );
}

void CJllServerApp::StoreProfileStrings()
{
	CString strSection    = "Jll Section";
	CString strStringItem = "Starting Directory";

	BOOL bRet = WriteProfileString( strSection, strStringItem, m_sStartingDir );
	ASSERT( bRet == TRUE );
}

void CJllServerApp::OnEditDisablewarmpoll() 
{
	DWORD myData = 0;
	HKEY phkResult;
	CString myKey;
	DWORD mySize;
	DWORD myType;
	myKey.LoadString( ID_EDIT_DISABLEWARMPOLL );
	myKey = myKey.Mid( myKey.Find( '\\' ) + 1 );
	LONG res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey, 0, KEY_READ, &phkResult );
	if( res == ERROR_SUCCESS )
	{
		mySize = sizeof myData;
		myType = REG_DWORD;

		res = RegQueryValueEx(
				phkResult,					// handle to key to query
				"DisableWarmPoll",			// address of name of value to query
				NULL,						// reserved
				&myType,					// address of buffer for value type
				(LPBYTE)&myData,			// address of data buffer
				&mySize );					// address of data buffer size

		if( res != ERROR_SUCCESS )
			myData = 0;
 	}
	RegCloseKey( phkResult );

	myData = myData ? 0 : 1;
	mySize = sizeof myData;
	myType = REG_DWORD;

	res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey, 0, KEY_WRITE, &phkResult );

	res = RegSetValueEx(
		phkResult,					// handle to key to query
		"DisableWarmPoll",			// address of name of value to query
		NULL,						// reserved
		myType,						// address of buffer for value type
		(LPBYTE)&myData,			// address of data buffer
		mySize );					// address of data buffer size

	RegCloseKey( phkResult );
}

// MSDN: If an application has set a very short timer, or if the system is
// sending the WM_SYSTIMER message, then OnIdle will be called repeatedly,
// and degrade performance.
BOOL CJllServerApp::OnIdle(LONG lCount) 
{
///	TRACE1( "theApp::OnIdle> lCount = %d\n", lCount );

	// as in most applications, we let the base class CWinApp::OnIdle complete
	// its processing before you attempt any additional idle loop processing.
	if( CWinApp::OnIdle( lCount ) )
		return TRUE;

	// Check to re-enable the timer for server if its is lost somehow.
	GetMyMainFrame()->CheckReTimerToDetectGuest();

	return FALSE;
}

//DEL void CJllServerApp::FormatOutput( LPCTSTR lpszFormat, ... )
//DEL {
//DEL 	// Find the document owned by this application.
//DEL /********
//DEL 	POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
//DEL 	ASSERT( pos );
//DEL 	CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate( pos );
//DEL 	ASSERT_KINDOF( CSingleDocTemplate, pTemplate );
//DEL 	pos = pTemplate->GetFirstDocPosition();
//DEL 	ASSERT( pos );
//DEL 	CJllServerDoc* pDoc = (CJllServerDoc*)pTemplate->GetNextDoc( pos );
//DEL 	ASSERT_KINDOF( CJllServerDoc, pDoc );
//DEL */
//DEL 	CJllServerDoc* pDoc = (CJllServerDoc*)((CFrameWnd*)m_pMainWnd)->GetActiveDocument();
//DEL 	ASSERT_KINDOF( CJllServerDoc, pDoc );
//DEL 
//DEL 	ASSERT( AfxIsValidString( lpszFormat ) );
//DEL 	va_list argList;
//DEL 	va_start( argList, lpszFormat );
//DEL 	pDoc->FormatOutputV( lpszFormat, argList );
//DEL 	va_end( argList );
//DEL }
