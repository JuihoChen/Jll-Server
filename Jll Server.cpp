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

#include "stdafx.h"
#include "Jll Server.h"

#include "MainFrm.h"
#include "Jll ServerDoc.h"
#include "Jll ServerView.h"
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
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJllServerApp construction

CJllServerApp::CJllServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	_OutputDebugString( "We start here...\n" );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CJllServerApp object

CJllServerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CJllServerApp initialization

BOOL CJllServerApp::InitInstance()
{
	// Check only running on WinXP/NT systems.
	if (GetVersion() & 0x80000000)
	{
		MessageBoxA(
			NULL,
			"This application is only allowed to be running on Windows XP or NT.",
			AfxGetAppName(),
			MB_ICONSTOP | MB_OK
		);
		return FALSE; // and fail
	}

	// Check if the parallel port setup okay?
	m_lptNibble.Setup();

	if( m_lptNibble.PortIsPresent() == FALSE )
	{
		MessageBoxA(
			NULL,
			"This application needs a printer port on the system.",
			AfxGetAppName(),
			MB_ICONSTOP | MB_OK
		);
		return FALSE; // and fail
	}

	// Only one instance is allowed to be running.
	HANDLE hMapping = CreateFileMapping(
		(HANDLE) 0xffffffff,
		NULL,
		PAGE_READONLY,
		0,
		32,
		"MyTestMap"
	);

	if( hMapping )
	{
		if( GetLastError() == ERROR_ALREADY_EXISTS )
		{
			// Display something that tells the user
			// the app is already running.
			AfxMessageBox( "Application is already running." );
			///MessageBox( NULL, "Application is already running.", m_pszAppName, MB_OK );
			///SetActiveWindow( FindWindow( _T("Jll Server"), NULL ) );
			return FALSE;
		}
	}
	else
	{
		// Some other error; handle error.
		AfxMessageBox( "Error creating mapping" );
		return FALSE;
	}

	// Try to open PortTalk driver to touch LPT ports...
	switch( m_drvPortTalk.OpenPortTalk() )
	{
	case CPortTalk::Success:
		break;
	case CPortTalk::InvalidHandleValue:
		MessageBoxA(
			NULL,
			"Couldn't access PortTalk Driver.",
			AfxGetAppName(),
			MB_ICONSTOP | MB_OK
		);
		return FALSE;
	case CPortTalk::IoControlError:
		MessageBoxA(
			NULL,
			"Error in calling DeviceIoControl.",
			AfxGetAppName(),
			MB_ICONSTOP | MB_OK
		);
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
		MessageBoxA(
			NULL,
			"Error in IPOM setting for process.",
			AfxGetAppName(),
			MB_ICONSTOP | MB_OK
		);
		return FALSE;
	}

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	((CMainFrame*)m_pMainWnd)->FormatOutput(
		"Found a printer port on 0x%x in the registry.", m_lptNibble.GetBaseAddr() );

	((CMainFrame*)m_pMainWnd)->m_pTheServer = new CDCServer( m_lptNibble );
	ASSERT_KINDOF( CDCServer, ((CMainFrame*)m_pMainWnd)->m_pTheServer );
	((CMainFrame*)m_pMainWnd)->m_pTheServer->ParseWorkDir( m_sStartingDir );
	((CMainFrame*)m_pMainWnd)->OnStartTimer( CMainFrame::nTimerIdDetectGuest );

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
	char szPath[ 1024 ];			// Buffer to hold path information

	m_sStartingDir = GetProfileString( strSection, strStringItem, "C:\\" );
	if( ::SetCurrentDirectory( m_sStartingDir ) == FALSE )
	{
		::GetCurrentDirectory( sizeof szPath, szPath );
		m_sStartingDir = szPath;
	}
}

void CJllServerApp::StoreProfileStrings()
{
	CString strSection    = "Jll Section";
	CString strStringItem = "Starting Directory";

	BOOL bRet = WriteProfileString( strSection, strStringItem, m_sStartingDir );
	ASSERT( bRet == TRUE );
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
