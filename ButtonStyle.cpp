// ButtonStyle.cpp : implementation file
//

#include "stdafx.h"
#include "ButtonStyle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char * THIS_FILE = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CButtonStyle

CButtonStyle::CButtonStyle()
{
	m_hIcon1 = m_hIcon2 = m_hIcon3 = m_hIcon4 = NULL;
	m_hSave = NULL;
}

CButtonStyle::~CButtonStyle()
{
}

BEGIN_MESSAGE_MAP(CButtonStyle, CButton)
	//{{AFX_MSG_MAP(CButtonStyle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButtonStyle message handlers

void CButtonStyle::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	if( lpDIS == NULL )
	{
		ASSERT(FALSE);
		return;
	}
	// must have at least the first icon loaded before calling DrawItem
	ASSERT( m_hIcon1 != NULL );     // required

	// use the main icon for up, the selected icon for down
	HICON hIcon = m_hIcon1;
	UINT state = lpDIS->itemState;
	if( (state & ODS_SELECTED) && m_hIcon2 != NULL )
		hIcon = m_hIcon2;
	else if( (state & ODS_FOCUS) && m_hIcon3 != NULL )
		hIcon = m_hIcon3;		// third image for focused
	else if( (state & ODS_DISABLED) && m_hIcon4 != NULL )
		hIcon = m_hIcon4;		// last image for disabled

	CDC *pDC = CDC::FromHandle( lpDIS->hDC );

	if( m_hSave != hIcon )		// to erase background?
	{
		m_hSave = hIcon;

		// Declare a CBrush and initialize to a system color.
		CBrush brush;
		brush.CreateSysColorBrush( COLOR_BTNFACE );

		// Draw.
		pDC->FillRect( &lpDIS->rcItem, &brush );
	}

	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawIcon( 0, 0, hIcon );
}

void CButtonStyle::SetIcons(UINT nIcon1, UINT nIcon2, UINT nIcon3, UINT nIcon4)
{
	// free memory old icons (if present) occupied.
	///::DestroyIcon( m_hIcon1 );
	///::DestroyIcon( m_hIcon2 );
	///::DestroyIcon( m_hIcon3 );
	///::DestroyIcon( m_hIcon4 );

	m_hIcon1 = ::LoadIcon(::AfxGetInstanceHandle(), MAKEINTRESOURCE(nIcon1));
	m_hIcon2 = ::LoadIcon(::AfxGetInstanceHandle(), MAKEINTRESOURCE(nIcon2));
	m_hIcon3 = ::LoadIcon(::AfxGetInstanceHandle(), MAKEINTRESOURCE(nIcon3));
	m_hIcon4 = ::LoadIcon(::AfxGetInstanceHandle(), MAKEINTRESOURCE(nIcon4));
}
