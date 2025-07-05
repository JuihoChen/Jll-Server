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
	//}}AFX_DATA_INIT

	// TODO: add construction code here
	_OutputDebugString( "View constructor...\n" );

	m_cButtonResetDir.SetIcons( IDI_ICON_RESET_DIR_U, IDI_ICON_RESET_DIR_D, IDI_ICON_RESET_DIR_F, IDI_ICON_RESET_DIR_X );
	m_cButtonSetDir.LoadBitmaps( IDB_BITMAP_SET_DIR_U, IDB_BITMAP_SET_DIR_D, IDB_BITMAP_SET_DIR_F, IDB_BITMAP_SET_DIR_X );

	m_f1stInitialUpdate = TRUE;
}

void CJllServerView::OnDestroy() 
{
	_OutputDebugString( "CJllServerView::OnDestroy\n" );
	CFormView::OnDestroy();
	
	// TODO: Add your message handler code here
	m_cStartingFolder.CleanUp();
}

CJllServerView::~CJllServerView()
{
	_OutputDebugString( "View destructor...\n" );
}

void CJllServerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJllServerView)
	DDX_Control(pDX, IDC_COMBO_FOR_DIR, m_cStartingFolder);
	DDX_Control(pDX, IDC_BUTTON_SET_DIR, m_cButtonSetDir);
	DDX_Control(pDX, IDC_BUTTON_RESET_DIR, m_cButtonResetDir);
	DDX_Control(pDX, IDC_GRP_FRAME, m_cGroupFrame);
	DDX_Control(pDX, IDC_BUTTON_FOR_DIR, m_cButtonForDir);
	//}}AFX_DATA_MAP

	_OutputDebugString( "View::DoDataExchange called.\n" );
}


BEGIN_MESSAGE_MAP(CJllServerView, CFormView)
	//{{AFX_MSG_MAP(CJllServerView)
	ON_BN_CLICKED(IDC_BUTTON_FOR_DIR, OnButtonForDir)
	ON_BN_CLICKED(IDC_BUTTON_RESET_DIR, OnButtonResetDir)
	ON_BN_CLICKED(IDC_BUTTON_SET_DIR, OnButtonSetDir)
	ON_CBN_SELCHANGE(IDC_COMBO_FOR_DIR, OnSelchangeComboForDir)
	ON_WM_DESTROY()
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

INT CALLBACK CJllServerView::BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp,LPARAM pData)
{
	TCHAR szDir[MAX_PATH];

	switch( uMsg )
	{
	case BFFM_INITIALIZED:
#if 0
		if( GetCurrentDirectory( sizeof(szDir)/sizeof(TCHAR), szDir ) )
		{
			// WParam is TRUE since you are passing a path.
			// It would be FALSE if you were passing a pidl.
			::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szDir );
		}
#else
		::SendMessage( hwnd, BFFM_SETSELECTION, TRUE,
			(LPARAM)(LPCSTR) GetMyMainFrame()->GetActiveView()->m_cStartingFolder.GetCurText() );
#endif
		break;

	case BFFM_SELCHANGED:
		// Set the status window to the currently selected path.
		if( SHGetPathFromIDList( (LPITEMIDLIST)lp , szDir ) )
		{
			::SendMessage( hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir );
		}
		break;
	}

	return 0;
}

void CJllServerView::OnButtonForDir()
{
	_OutputDebugString( "View::Button for Dir. called.\n" );

	// Disable timer for detecting the guest temporarily.
	CMainFrame* pFrame = (CMainFrame*)GetParent();
	ASSERT_KINDOF( CMainFrame, pFrame );
	pFrame->StopTimer( CMainFrame::nTimerIdDetectGuest );

	BROWSEINFO bi;
	TCHAR szDir[MAX_PATH];
	LPITEMIDLIST pidl;
	LPMALLOC pMalloc;

	if( SUCCEEDED( SHGetMalloc( &pMalloc ) ) )
	{
		ZeroMemory( &bi, sizeof bi );
		bi.hwndOwner = GetSafeHwnd();
		bi.pszDisplayName = 0;
		bi.lpszTitle = "Select path:";
		bi.pidlRoot = 0;
		bi.ulFlags = BIF_RETURNONLYFSDIRS;// | BIF_STATUSTEXT;
		bi.lpfn = CJllServerView::BrowseCallbackProc;
		pidl = SHBrowseForFolder( &bi );
		if( pidl )
		{
			if( SHGetPathFromIDList( pidl, szDir ) )
			{
				if( m_cStartingFolder.SetCurText( szDir ) )
				{
					GetDocument()->FormatOutput( "Starting Folder is changed." );
				}
				UpdateData( FALSE );
				// Update directory name in CDCServer class anyway.
				ASSERT_KINDOF( CDCServer, pFrame->m_pTheServer );
				pFrame->m_pTheServer->ParseWorkDir( m_cStartingFolder.GetCurText() );

				pMalloc->Free( pidl );
				pMalloc->Release();
			}
		}
	}

	// Reenable timer to detect the guest after this Dialog.
///v0.18***	pFrame->StartTimer( CMainFrame::nTimerIdDetectGuest );	
}

void CJllServerView::OnSelchangeComboForDir() 
{
	_OutputDebugString( "View::OnSelchangeComboForDir called.\n" );

	// Disable timer for detecting the guest temporarily.
	CMainFrame* pFrame = GetMyMainFrame();
	pFrame->StopTimer( CMainFrame::nTimerIdDetectGuest );

	if( m_cStartingFolder.SetCurText( m_cStartingFolder.GetCurText() ) )
	{
		GetDocument()->FormatOutput( "Starting Folder is changed." );
	}

	ASSERT_KINDOF( CDCServer, pFrame->m_pTheServer );
	pFrame->m_pTheServer->ParseWorkDir( m_cStartingFolder.GetCurText() );
}

void CJllServerView::OnButtonResetDir()
{
	// Disable timer for detecting the guest temporarily.
	CMainFrame* pFrame = GetMyMainFrame();
	pFrame->StopTimer( CMainFrame::nTimerIdDetectGuest );

	GetDocument()->FormatOutput( "Transferring directory is restored to the Starting Folder." );

	ASSERT_KINDOF( CDCServer, pFrame->m_pTheServer );
	pFrame->m_pTheServer->ParseWorkDir( m_cStartingFolder.GetCurText() );
}

void CJllServerView::OnButtonSetDir()
{
	// Disable timer for detecting the guest temporarily.
	CMainFrame* pFrame = GetMyMainFrame();
	pFrame->StopTimer( CMainFrame::nTimerIdDetectGuest );

	GetDocument()->FormatOutput( "Starting Folder is set to the Transferring directory." );

	// restore the working folder to a LongPathName.
	char szLong[_MAX_PATH];
	::GetLongPathName( pFrame->m_pTheServer->GetWorkDir(), szLong, sizeof szLong );

	m_cStartingFolder.SetCurText( szLong );
	UpdateData( FALSE );
}

void CJllServerView::EnableFolderChange(BOOL bEnable /* = TRUE */)
{
	if( bEnable )
	{
		m_cButtonForDir.EnableWindow( TRUE );
		m_cStartingFolder.EnableWindow( TRUE );

		// restore the working folder to a LongPathName.
		char szLong[_MAX_PATH];
		::GetLongPathName( GetMyMainFrame()->m_pTheServer->GetWorkDir(), szLong, sizeof szLong );

		if( m_cStartingFolder.GetCurText().CompareNoCase( szLong ) )
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
		m_cStartingFolder.EnableWindow( FALSE );
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

	if( m_f1stInitialUpdate )			// Till now, there is a real window!
	{
		m_f1stInitialUpdate = FALSE;
		m_cStartingFolder.Initialize();

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
		SetScrollSizes( MM_TEXT, CSize( 700, m_nTopForText + m_nNumLines * m_nLineHeight ) );

		// set up the tooltip control....
		m_cToolTip.Create( this, TTS_ALWAYSTIP );
		m_cToolTip.AddTool( &m_cButtonResetDir, IDC_BUTTON_RESET_DIR );
		m_cToolTip.AddTool( &m_cButtonSetDir, IDC_BUTTON_SET_DIR );
		m_cToolTip.AddTool( &m_cStartingFolder, IDC_COMBO_FOR_DIR );
		m_cToolTip.Activate( TRUE );
	}
}

void CJllServerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	_OutputDebugString( "View::OnUpdate called.\n" );

	// Call SetScrollSizes when the view is about to be updated. Call it in your
	// override of the OnUpdate member function to adjust scrolling characteristics.
	///SetScrollSizes( MM_TEXT, CSize( 700, 400 ) );

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
		if( nID == IDC_BUTTON_FOR_DIR || nID == IDC_COMBO_FOR_DIR )
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

/////////////////////////////////////////////////////////////////////////////
// CFolderCombo

CFolderCombo::CFolderCombo()
{
	m_fChanged = FALSE;
}

CFolderCombo::~CFolderCombo()
{
	ASSERT( m_fChanged == FALSE );
}

BEGIN_MESSAGE_MAP(CFolderCombo, CComboBox)
	//{{AFX_MSG_MAP(CFolderCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFolderCombo message handlers

void CFolderCombo::Initialize()
{
	TRACE1( "CFolderCombo::Initialize -- GetWindowLong = %x\n", ::GetWindowLong( m_hWnd, GWL_STYLE ) );

	// Use the extended UI if it is not already set.
	if( !GetExtendedUI() )
		SetExtendedUI( TRUE );

	// Must have CBS_DROPDOWNLIST style! but ModifyStyle doesn't work?

	AddString( theApp.m_sStartingDir );

	for( int i = 1; i < 10; i ++ )
	{
		CString sTemp = theApp.LoadProfileStrings( i );
		if( !sTemp.IsEmpty() ) InsertString( GetCount(), sTemp );
	}

	// Selects the first string in the list box of a combo box
	SetCurSel( 0 );
}

void CFolderCombo::CleanUp()
{
	if( m_fChanged )
	{
		m_fChanged = FALSE;
		for( int i = 0; i < 10; i ++ )
		{
			if( i < GetCount() )
			{
				CString sText;
				GetLBText( i, sText );
				theApp.StoreProfileStrings( i, sText );
			}
			else
				theApp.StoreProfileStrings( i );
		}
	}
}

CString CFolderCombo::GetCurText() const
{
	CString sText;
	GetLBText( GetCurSel(), sText );
	return sText;
}

// CFolderCombo::SetCurText returns FALSE if none set in list!

BOOL CFolderCombo::SetCurText(const CString sText)
{
	// only take care in case of differences...
	CString sTemp;
	GetLBText( 0, sTemp );
	if( sTemp.CompareNoCase( sText ) )
	{
		int nCount = GetCount();
		for( int i = nCount - 1; i > 0; i -- )
		{
			// take out the same entry inside the list
			GetLBText( i, sTemp );
			if( sTemp.CompareNoCase( sText ) == 0 )
				DeleteString( i );
		}
		// check maximum count in list
		if( GetCount() > 10 ) DeleteString( 9 );
		InsertString( 0, sText );
		SetCurSel( 0 );
		return m_fChanged = TRUE;
	}
	else
		return FALSE;
}
