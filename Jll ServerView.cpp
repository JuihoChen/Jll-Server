// Jll ServerView.cpp : implementation of the CJllServerView class
//

#include "stdafx.h"
#include "Jll Server.h"
#include "Jll ServerDoc.h"
#include "Jll ServerView.h"
#include "MainFrm.h"
#include "dirpkr.h"
#include "dlgs.h"
#include "Direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJllServerView

IMPLEMENT_DYNCREATE(CJllServerView, CFormView)

CJllServerView::CJllServerView()
	: CFormView(CJllServerView::IDD)
{
	//{{AFX_DATA_INIT(CJllServerView)
	m_sStartingFolder = _T("");
	//}}AFX_DATA_INIT

	// TODO: add construction code here
	_OutputDebugString( "View constructor...\n" );

	m_cButtonResetDir.SetIcons( IDI_ICON_RESET_DIR_U, IDI_ICON_RESET_DIR_D, IDI_ICON_RESET_DIR_F, IDI_ICON_RESET_DIR_X );
	m_cButtonSetDir.LoadBitmaps( IDB_BITMAP_SET_DIR_U, IDB_BITMAP_SET_DIR_D, IDB_BITMAP_SET_DIR_F, IDB_BITMAP_SET_DIR_X );

	m_sStartingFolder = GetMyApp()->m_sStartingDir;
	m_f1stInitialUpdate = TRUE;
}

CJllServerView::~CJllServerView()
{
	_OutputDebugString( "View destructor...\n" );

	CJllServerApp* pApp = GetMyApp();

	if( m_sStartingFolder != pApp->m_sStartingDir )
	{
		pApp->m_sStartingDir = m_sStartingFolder;
		pApp->StoreProfileStrings();
	}
}

void CJllServerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJllServerView)
	DDX_Control(pDX, IDC_BUTTON_SET_DIR, m_cButtonSetDir);
	DDX_Control(pDX, IDC_BUTTON_RESET_DIR, m_cButtonResetDir);
	DDX_Control(pDX, IDC_GRP_FRAME, m_cGroupFrame);
	DDX_Control(pDX, IDC_BUTTON_FOR_DIR, m_cButtonForDir);
	DDX_Text(pDX, IDC_EDIT_FOR_DIR, m_sStartingFolder);
	DDV_MaxChars(pDX, m_sStartingFolder, 256);
	//}}AFX_DATA_MAP

	_OutputDebugString( "View::DoDataExchange called.\n" );
}


BEGIN_MESSAGE_MAP(CJllServerView, CFormView)
	//{{AFX_MSG_MAP(CJllServerView)
	ON_BN_CLICKED(IDC_BUTTON_FOR_DIR, OnButtonForDir)
	ON_BN_CLICKED(IDC_BUTTON_RESET_DIR, OnButtonResetDir)
	ON_BN_CLICKED(IDC_BUTTON_SET_DIR, OnButtonSetDir)
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJllServerView diagnostics

#ifdef _DEBUG
void CJllServerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CJllServerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CJllServerDoc* CJllServerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CJllServerDoc)));
	return (CJllServerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CJllServerView message handlers

void CJllServerView::OnButtonForDir()
{
	// TODO: Add your control notification handler code here
	_OutputDebugString( "View::Button for Dir. called.\n" );

	// Disable timer for detecting the guest temporarily.
	CMainFrame* pFrame = (CMainFrame*)GetParent();
	ASSERT_KINDOF( CMainFrame, pFrame );
	pFrame->OnStopTimer( CMainFrame::nTimerIdDetectGuest );
	
	CMyFileDlg cfdlg(
		FALSE,
		NULL,
		NULL,
		/*OFN_SHOWHELP |*/ OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLETEMPLATE,
		NULL,
		GetMyApp()->m_pMainWnd
	);
 	// Indicates that the lpTemplateName member is a pointer to the name of a
	// dialog template resource in the module identified by the hInstance member.
    cfdlg.m_ofn.hInstance = AfxGetInstanceHandle();
	cfdlg.m_ofn.lpTemplateName = MAKEINTRESOURCE( IDD_FILEOPENORD );
	// If the OFN_EXPLORER flag is set, the system uses the specified template to
	// create a dialog box that is a child of the default Explorer-style dialog box.
	// If the OFN_EXPLORER flag is not set, the system uses the template to create
	// an old-style dialog box that replaces the default dialog box.
	cfdlg.m_ofn.Flags &= ~OFN_EXPLORER;
	// Lines of some code snippet is used to circumvent the function call to
	// ::SetCurrentDirectory( m_sStartingFolder )
	if( !m_sStartingFolder.IsEmpty() )
		cfdlg.m_ofn.lpstrInitialDir = m_sStartingFolder;

    if( IDOK == cfdlg.DoModal() )
    {
		WORD wFileOffset = cfdlg.m_ofn.nFileOffset;		//for convenience

		cfdlg.m_ofn.lpstrFile[ wFileOffset-1 ] = 0;		//Nuke the "Junk"

		if( m_sStartingFolder.CompareNoCase( cfdlg.m_ofn.lpstrFile ) )
		{
			m_sStartingFolder = cfdlg.m_ofn.lpstrFile;
			GetDocument()->FormatOutput( "Starting Folder is changed." );
		}
		UpdateData( FALSE );
		// Update directory name in CDCServer class anyway.
		ASSERT_KINDOF( CDCServer, pFrame->m_pTheServer );
		pFrame->m_pTheServer->ParseWorkDir( m_sStartingFolder );
    }

	// Reenable timer to detect the guest after this Dialog.
///v0.18***	pFrame->OnStartTimer( CMainFrame::nTimerIdDetectGuest );	
}

void CJllServerView::OnButtonResetDir()
{
	// Disable timer for detecting the guest temporarily.
	CMainFrame* pFrame = GetMyMainFrame();
	pFrame->OnStopTimer( CMainFrame::nTimerIdDetectGuest );

	GetDocument()->FormatOutput( "Transferring directory is restored to the Starting Folder." );

	ASSERT_KINDOF( CDCServer, pFrame->m_pTheServer );
	pFrame->m_pTheServer->ParseWorkDir( m_sStartingFolder );
}

void CJllServerView::OnButtonSetDir()
{
	// Disable timer for detecting the guest temporarily.
	CMainFrame* pFrame = GetMyMainFrame();
	pFrame->OnStopTimer( CMainFrame::nTimerIdDetectGuest );

	GetDocument()->FormatOutput( "Starting Folder is set to the Transferring directory." );

	// restore the working folder to a LongPathName.
	char szLong[_MAX_PATH];
	::GetLongPathName( pFrame->m_pTheServer->GetWorkDir(), szLong, sizeof szLong );

	m_sStartingFolder = szLong;
	UpdateData( FALSE );
}

void CJllServerView::EnableFolderChange(BOOL bEnable /* = TRUE */)
{
	if( bEnable )
	{
		m_cButtonForDir.EnableWindow( TRUE );

		// restore the working folder to a LongPathName.
		char szLong[_MAX_PATH];
		::GetLongPathName( GetMyMainFrame()->m_pTheServer->GetWorkDir(), szLong, sizeof szLong );

		if( m_sStartingFolder.CompareNoCase( szLong ) )
		{
			m_cButtonResetDir.EnableWindow( TRUE );
			m_cButtonSetDir.EnableWindow( TRUE );
		}
		else
		{
			m_cButtonResetDir.EnableWindow( FALSE );
			m_cButtonSetDir.EnableWindow( FALSE );
		}
	}
	else
	{
		m_cButtonForDir.EnableWindow( FALSE );
		m_cButtonResetDir.EnableWindow( FALSE );
		m_cButtonSetDir.EnableWindow( FALSE );
	}
}

void CJllServerView::OnDraw(CDC* pDC) 
{
	CJllServerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
///	_OutputDebugString( "View::OnDraw called.\n" );

	// this line below makes abnormal scrolling not to display some text.
	///v0.16//if( pDC->RectVisible( &rect ) )
	{
		CStringList& rSlm = pDoc->m_slMessages;
		if( !rSlm.IsEmpty() )
		{
			pDC->SetTextAlign( TA_BASELINE );

			// Start at front of list
			POSITION pos = rSlm.GetHeadPosition();
			int cY = m_nTopForText;

			// Iterate over whole list
			while( pos != NULL )
			{
				pDC->TextOut( 15, cY, rSlm.GetNext( pos ) );
				cY += m_nLineHeight;
			}
		}
	}
}

void CJllServerView::InvalidateForNewLine()
{
	int nLines = GetDocument()->m_slMessages.GetCount();

	CRect rect;
	GetClientRect( &rect );

	rect.SetRect(
		rect.left,
		0, //m_nTopForText,
		rect.right,
		m_nTopForText + nLines * m_nLineHeight
	);

	InvalidateRect( &rect );

	// UpdateWindow is needed for the text to be put onto screen.
	UpdateWindow();
}

void CJllServerView::OnInitialUpdate() 
{
	_OutputDebugString( "View::OnInitialUpdate called.\n" );

	// The OnInitialUpdate member function is called to perform one-time
	// initialization of the view.
	// Lines below are used to do custom initialization only.
	// ***Delete lines for initialization due to reset by NewDocument.***
	//m_sStartingFolder = GetMyApp()->m_sStartingDir;
	//UpdateData( FALSE );

	CFormView::OnInitialUpdate();		// this calls OnUpdate then
	
	// TODO: Add your specialized code here and/or call the base class

	EnableToolTips( TRUE );				// necessary for a windows NOT derived from CFrameWnd

//	m_cButtonResetDir.SizeToContent();
	m_cButtonSetDir.SizeToContent();

	// You'll typically place the call to SetScaleToFitSize in your override
	// of the view's OnInitialUpdate member function.
	//GetParentFrame()->RecalcLayout();
	//SetScaleToFitSize( CSize( 734, 450 ) );
	////SetScaleToFitSize( GetTotalSize() );

	TEXTMETRIC tm;
	CPaintDC dc( this );
	OnPrepareDC( &dc );
	dc.GetTextMetrics( &tm );

	CRect rect;
	m_cGroupFrame.GetClientRect( &rect );

	m_nTopForText = (int)(1.45 * rect.bottom);
	m_nLineHeight = tm.tmHeight + 2;

	GetClientRect( &rect );				// the initial rect. It's variable!

	m_nNumLines = (rect.bottom - m_nTopForText) / m_nLineHeight - 3;
	if( m_nNumLines < 20 ) m_nNumLines = 20;

	if( m_f1stInitialUpdate )
	{
		m_f1stInitialUpdate = FALSE;

		m_cGroupFrame.GetClientRect( &rect );

		int ncx = rect.right + 0x30;
		CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
		ASSERT_KINDOF( CMainFrame, pFrame );

		pFrame->GetWindowRect( &rect );

		if( rect.Width() < ncx )
		{
			int scx = ::GetSystemMetrics( SM_CXSCREEN );
			scx = ( scx >= ncx ) ? (scx - ncx) / 2 : rect.left;
			pFrame->SetWindowPos(
				NULL,
				scx, rect.top,
				ncx, rect.Height(),
				SWP_NOZORDER | SWP_NOACTIVATE );	// SWP_NOMOVE
		}

		// set up the tooltip control....
		m_cToolTip.Create( this );
		m_cToolTip.AddTool( &m_cButtonResetDir, IDC_BUTTON_RESET_DIR );
		m_cToolTip.AddTool( &m_cButtonSetDir, IDC_BUTTON_SET_DIR );
		m_cToolTip.Activate( TRUE );
	}
}

void CJllServerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	_OutputDebugString( "View::OnUpdate called.\n" );

	// Call SetScrollSizes when the view is about to be updated. Call it in your
	// override of the OnUpdate member function to adjust scrolling characteristics.
	SetScrollSizes( MM_TEXT, CSize( 700, 450 ) );

	// The OnUpdate member function is defined by CView and is called to update
	// the form view's appearance. Override this function to update the member
	// variables in your view class with the appropriate values from the current
	// document.
	// Then, if you are using DDX, use the UpdateData member function with an
	// argument of FALSE to update the controls in your form view.
	UpdateData( FALSE );

}

BOOL CJllServerView::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult )
{
	_OutputDebugString( "View::OnToolTipNotify called.\n" );

	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	UINT nID = pNMHDR->idFrom;
	if( pTTT->uFlags & TTF_IDISHWND )
	{
		// idFrom is actually the HWND of the tool
		nID = ::GetDlgCtrlID( (HWND)nID );
		if( nID == IDC_BUTTON_FOR_DIR || nID == IDC_EDIT_FOR_DIR )
		{
			pTTT->lpszText = MAKEINTRESOURCE( nID );
			pTTT->hinst = AfxGetResourceHandle();
			return TRUE;			// message was handled
		}
	}
	return FALSE;
}

BOOL CJllServerView::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_cToolTip.RelayEvent( pMsg );
	return CFormView::PreTranslateMessage(pMsg);
}
