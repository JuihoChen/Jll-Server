#include "stdafx.h"
#include "Jll Server.h"
#include "Except.h"
#include "TrayIcon.h"

IMPLEMENT_DYNAMIC( CTrayIcon, CObject )

CTrayIcon::CTrayIcon(HWND hWnd, UINT uIconID, HICON hIcon, LPCSTR lpToolTip, MouseMsgHandlerPtr *pMouseMsgHandler, int nHandlers)
{
	m_bMinimizedToTray = FALSE;

	SetTrayIcon(hWnd, uIconID, hIcon, lpToolTip);
	SetMouseMsgHandler(pMouseMsgHandler, nHandlers);
}

CTrayIcon::~CTrayIcon()
{
	DeleteIcon();

	for( int i = 0; i < m_nHandlers; i ++ )
		delete m_pMouseMsgHandler[i];
	delete m_pMouseMsgHandler;
}

void CTrayIcon::SetTrayIcon(HWND hWnd, UINT uIconID, HICON hIcon, LPCSTR lpToolTip)
{
	m_hWnd = hWnd;
	m_uIconID = uIconID;
	m_hIcon = hIcon;
	m_lpToolTip = lpToolTip;	
}

void CTrayIcon::SetMouseMsgHandler(MouseMsgHandlerPtr *pMouseMsgHandler, int nHandlers)
{
	m_pMouseMsgHandler = pMouseMsgHandler;
	m_nHandlers = nHandlers;
	for( int i = 0; i < m_nHandlers; i ++ )
		m_pMouseMsgHandler[i]->SetTrayIcon( this );
}

LRESULT CTrayIcon::OnTaskBarCreated(WPARAM wParam, LPARAM lParam)
{
	if( m_bMinimizedToTray )
	{
		HideWindow();
	}
	return 0;
}

void CTrayIcon::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	UINT uID;
	UINT uMouseMsg;

	uID = (UINT) wParam;
	uMouseMsg = (UINT) lParam;

	if( uID == m_uIconID )
		for( int i = 0; i < m_nHandlers; i ++ )
			if( uMouseMsg == m_pMouseMsgHandler[i]->GetMouseMsgID() )
				m_pMouseMsgHandler[i]->MouseMsgHandler();
}

void CTrayIcon::RestoreWindow()
{
	m_bMinimizedToTray = FALSE;
	DeleteIcon();
// v0.24 *** produce a custom caption animation to maximize from the system tray.
	RestoreWndFromTray( m_hWnd );
//	FromHandle()->ShowWindow(SW_SHOW);	///FromHandle()->ShowWindow( SW_NORMAL );
//	FromHandle()->ShowWindow(SW_RESTORE);
//	FromHandle()->SetForegroundWindow();
}

void CTrayIcon::HideWindow(HICON hIcon /* = NULL */)
{
	m_bMinimizedToTray = TRUE;
	if( AddIcon( hIcon ) )
	{
// v0.24 *** produce a custom caption animation to minimize to the system tray.
		MinimizeWndToTray( m_hWnd );
//		FromHandle()->ShowWindow( SW_MINIMIZE );	// animate Minimize.
//		FromHandle()->ShowWindow( SW_HIDE );
	}
}

BOOL CTrayIcon::AddIcon(HICON hIcon)
{
	NOTIFYICONDATA tnid; 

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = m_hWnd;
	tnid.uID = m_uIconID;
	tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnid.uCallbackMessage = UWM_NOTIFY_ICON;
	tnid.hIcon = hIcon ? hIcon : m_hIcon;
	if( m_lpToolTip )
		lstrcpyn(tnid.szTip, m_lpToolTip, sizeof(tnid.szTip));
	else
		tnid.szTip[0] = '\0';

	return Shell_NotifyIcon(NIM_ADD, &tnid);
}

BOOL CTrayIcon::DeleteIcon()
{
	NOTIFYICONDATA tnid; 

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = m_hWnd;
	tnid.uID = m_uIconID;

	return Shell_NotifyIcon(NIM_DELETE, &tnid); 
}

BOOL CTrayIcon::ModifyIcon(HICON hIcon, LPCSTR lpToolTip /* = NULL */)
{
#define SIZEOF_TNID_SZTIP	68			// Use the Windows 95 behavior. 

	if( m_bMinimizedToTray )
	{
		NOTIFYICONDATA tnid;
		tnid.hWnd = m_hWnd;
		tnid.uID = m_uIconID;
		tnid.hIcon = m_hIcon = hIcon ? hIcon : m_hIcon;
		if( !lpToolTip ) {
			tnid.uFlags = NIF_ICON;
		}
		else {
			tnid.uFlags = NIF_ICON | NIF_TIP;
			CString sTemp1 = lpToolTip;
			if( sTemp1.GetLength() >= SIZEOF_TNID_SZTIP )
			{
				CString sTemp2 = sTemp1.Left( 3 ) + "...";
				sTemp1 = sTemp2 + sTemp1.Right( SIZEOF_TNID_SZTIP - 7 );
			}
			lstrcpyn( tnid.szTip, sTemp1, SIZEOF_TNID_SZTIP );
		}
		return Shell_NotifyIcon( NIM_MODIFY, &tnid );
	}
	else
		return FALSE;
}

CLeftMouseClickMsgHandler::CLeftMouseClickMsgHandler() : CTrayIconMouseMsgHandler(WM_LBUTTONUP)
{
	m_i64LastTime = gblQPCTimer.GetQPCTime();
}

void CLeftMouseClickMsgHandler::MouseMsgHandler()
{
	ULONGLONG i64Diff = gblQPCTimer.GetQPCTime() - m_i64LastTime;
	if( (i64Diff * 10) / gblQPCTimer.GetFrequency() <= 3 )	// range is 0 ~ 399 ms
	{
		ASSERT_KINDOF( CTrayIcon, m_pTrayIcon );
		m_pTrayIcon->RestoreWindow();
	}
	m_i64LastTime = gblQPCTimer.GetQPCTime();
}

void CLeftMouseDblClickMsgHandler::MouseMsgHandler()
{
	ASSERT_KINDOF( CTrayIcon, m_pTrayIcon );
	m_pTrayIcon->RestoreWindow();
}

void CRightMouseClickMsgHandler::MouseMsgHandler()
{
	// Display the menu at the current mouse location. There's a "bug" (Microsoft
	// calls it a feature) in Windows 95 that requires calling SetForegroundWindow.
	// To find out more, search for Q135788 in MSDN.
	::SetForegroundWindow( m_pTrayIcon->FromHandle()->GetSafeHwnd() );
	// Without this modification, the pop-up menu is not dismissed even if the user
	// clicks outside the pop-up menu. 

	POINT p;
	GetCursorPos( &p );
	CMenu aMenu;
	aMenu.LoadMenu( m_idMenu );
	aMenu.GetSubMenu( 0 )->TrackPopupMenu(
		TPM_RIGHTALIGN | TPM_RIGHTBUTTON,
		p.x, p.y,
		m_pTrayIcon->FromHandle(),
		NULL );

	// The second problem is caused by a problem with TrackPopupMenu. It is
	// necessary to force a task switch to the application that called
	// TrackPopupMenu at some time in the near future. This can be accomplished
	// by posting a benign message to the window or thread. 
	::PostMessage( m_pTrayIcon->FromHandle()->GetSafeHwnd(), WM_NULL, 0, 0 );
}
