// Jll ServerDoc.cpp : implementation of the CJllServerDoc class
//

#include "stdafx.h"
#include "Jll Server.h"
#include "Jll ServerDoc.h"
#include "Jll ServerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJllServerDoc

IMPLEMENT_DYNCREATE(CJllServerDoc, CDocument)

BEGIN_MESSAGE_MAP(CJllServerDoc, CDocument)
	//{{AFX_MSG_MAP(CJllServerDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJllServerDoc construction/destruction

CJllServerDoc::CJllServerDoc()
{
	// TODO: add one-time construction code here
	_OutputDebugString( "Doc constructor...\n" );

}

CJllServerDoc::~CJllServerDoc()
{
}

BOOL CJllServerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	_OutputDebugString( "Doc::OnNewDocument.\n" );

	POSITION pos = GetFirstViewPosition();
	CView* pFirstView = GetNextView( pos );
	pFirstView->Invalidate();
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CJllServerDoc serialization

void CJllServerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CJllServerDoc diagnostics

#ifdef _DEBUG
void CJllServerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CJllServerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CJllServerDoc commands

//DEL void CJllServerDoc::OnEditCut() 
//DEL {
//DEL 	// TODO: Add your command handler code here
//DEL 	
//DEL }

void CJllServerDoc::DeleteContents() 
{
	// TODO: Add your specialized code here and/or call the base class
	_OutputDebugString( "Doc::DeleteContents.\n" );

	m_slMessages.RemoveAll();

	CDocument::DeleteContents();	// default implementation does nothing.
}

void CJllServerDoc::FormatOutput( LPCTSTR lpszFormat, ... )
{
	ASSERT( AfxIsValidString( lpszFormat ) );
	va_list argList;
	va_start( argList, lpszFormat );
	FormatOutputV( lpszFormat, argList );
	va_end( argList );
}

void CJllServerDoc::FormatOutputV( LPCTSTR lpszFormat, va_list argList )
{
	CString sTemp;
	sTemp.FormatV( lpszFormat, argList );
	m_slMessages.AddTail( sTemp );

	POSITION pos = GetFirstViewPosition();
	CJllServerView* pFirstView = (CJllServerView*)( GetNextView( pos ) );

	if( m_slMessages.GetCount() <= 20 )
		pFirstView->InvalidateForNewLine();

	else
	{
		m_slMessages.RemoveHead();
		pFirstView->Invalidate();
	}
}
