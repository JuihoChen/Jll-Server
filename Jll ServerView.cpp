// Jll ServerView.cpp : implementation of the CJllServerView class
//

#include "stdafx.h"
#include "Jll Server.h"

#include "Jll ServerDoc.h"
#include "Jll ServerView.h"
#include "ChooseDirDlg.h"

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
	_OutputDebugString( "View constructor..." );

	m_sStartingFolder = GetMyApp()->m_sStartingDir;
}

CJllServerView::~CJllServerView()
{
	_OutputDebugString( "View destructor..." );

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
	DDX_Text(pDX, IDC_EDIT_FOR_DIR, m_sStartingFolder);
	DDV_MaxChars(pDX, m_sStartingFolder, 256);
	//}}AFX_DATA_MAP

	_OutputDebugString( "View::DoDataExchange called." );
}


BEGIN_MESSAGE_MAP(CJllServerView, CFormView)
	//{{AFX_MSG_MAP(CJllServerView)
	ON_BN_CLICKED(IDC_BUTTON_FOR_DIR, OnButtonForDir)
	//}}AFX_MSG_MAP
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
	_OutputDebugString( "View::Button for Dir. called." );
	
	::SetCurrentDirectory( m_sStartingFolder );

	CChooseDirDlg dlg( m_szBackupDirs );
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
		if( m_sStartingFolder != dlg.m_sFolderName )
		{
			GetDocument()->FormatOutput( "Starting Folder is changed." );
		}
		m_sStartingFolder = dlg.m_sFolderName;
		UpdateData( FALSE );
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
}

void CJllServerView::OnDraw(CDC* pDC) 
{
	CJllServerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
	_OutputDebugString( "View::OnDraw called." );

	CRect rect;
	GetClientRect( &rect );
    if( pDC->RectVisible( &rect ) )
	{
		CStringList& rSlm = GetDocument()->m_slMessages;
		if( !rSlm.IsEmpty() )
		{
			TEXTMETRIC tm;

			pDC->SetTextAlign( TA_BASELINE );
			pDC->GetTextMetrics( &tm );

			// Start at front of list
			POSITION pos = rSlm.GetHeadPosition();
			int cY = 80;

			// Iterate over whole list
			while( pos != NULL )
			{
				pDC->TextOut( rect.left + 15, cY, rSlm.GetNext( pos ) );
				cY += tm.tmHeight + 2;
			}
		}
	}
}

void CJllServerView::InvalidateForNewLine()
{
	CPaintDC dc( this );
	OnPrepareDC( &dc );
	TEXTMETRIC tm;
	dc.GetTextMetrics( &tm );

	int nLines = GetDocument()->m_slMessages.GetCount();

	CRect rect;
	GetClientRect( &rect );

	Invalidate();

	int nTop = 80 + nLines * ( tm.tmHeight + 2 );

	if( nTop < rect.bottom )
	{
		rect.SetRect( rect.left, nTop, rect.right, rect.bottom );
		ValidateRect( &rect );
	}
/**************************************************
	rect.SetRect(
		rect.left,
		( nLines - 1 ) * ( tm.tmHeight + 2 ),
		rect.right,
		80 + nLines * ( tm.tmHeight + 2 )
	);

	InvalidateRect( &rect );
	///*!*
	UpdateWindow();
	///*!*
*****/
}

void CJllServerView::OnInitialUpdate() 
{
	_OutputDebugString( "View::OnInitialUpdate called." );

	// The OnInitialUpdate member function is called to perform one-time
	// initialization of the view.
	// Lines below are used to do custom initialization only.
	// ***Delete lines for initialization due to reset by NewDocument.***
	//m_sStartingFolder = GetMyApp()->m_sStartingDir;
	//UpdateData( FALSE );

	CFormView::OnInitialUpdate();		// this calls OnUpdate then
	
	// TODO: Add your specialized code here and/or call the base class

	// You'll typically place the call to SetScaleToFitSize in your override
	// of the view's OnInitialUpdate member function.
	//GetParentFrame()->RecalcLayout();
	//SetScaleToFitSize( CSize( 734, 450 ) );
	////SetScaleToFitSize( GetTotalSize() );
}

void CJllServerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	_OutputDebugString( "View::OnUpdate called." );

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

