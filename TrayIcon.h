#if !defined(_TRAYICON_H_)
#define _TRAYICON_H_

#define UWM_NOTIFY_ICON_MSG	L"UWM_NOTIFY_ICON-{5DC84998-8E67-4369-B279-F77AB0B099B2}"

const UINT UWM_NOTIFY_ICON = ::RegisterWindowMessage( UWM_NOTIFY_ICON_MSG );

const UINT UWM_TASKBAR_CREATED = ::RegisterWindowMessage( L"TaskbarCreated" );

class CTrayIconMouseMsgHandler;

typedef CTrayIconMouseMsgHandler* MouseMsgHandlerPtr;

class CTrayIcon : public CObject
{
	DECLARE_DYNAMIC( CTrayIcon )
public:
	CTrayIcon(HWND hWnd, UINT uIconID, HICON hIcon, LPCTSTR lpToolTip, MouseMsgHandlerPtr *pMouseMsgHandler, int nHandlers);
	CTrayIcon() { m_bMinimizedToTray = FALSE; };
	~CTrayIcon();
	void SetTrayIcon(HWND hWnd, UINT uIconID, HICON hIcon, LPCTSTR lpToolTip);
	void SetMouseMsgHandler(MouseMsgHandlerPtr *pMouseMsgHandler, int nHandlers);
	void HideWindow(HICON hIcon = NULL);
	void RestoreWindow();
	void MinimizeWndToTray(HWND hWnd);
	void RestoreWndFromTray(HWND hWnd);
	CWnd* FromHandle() const { return CWnd::FromHandle(m_hWnd); }
	BOOL AddIcon(HICON hIcon) const;
	BOOL DeleteIcon() const;
	BOOL ModifyIcon(HICON hIcon, LPCTSTR lpToolTip = NULL);
	afx_msg LRESULT OnTaskBarCreated(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNotifyIcon(WPARAM wParam, LPARAM lParam);	
private:
	HWND m_hWnd;
	UINT m_uIconID;
	HICON m_hIcon;
	LPCTSTR m_lpToolTip;
	MouseMsgHandlerPtr *m_pMouseMsgHandler;
	int m_nHandlers;
	BOOL m_bMinimizedToTray;
};

class CTrayIconMouseMsgHandler : public CObject
{
public:
	CTrayIconMouseMsgHandler(UINT uMouseMsgID = WM_LBUTTONDBLCLK) : m_pTrayIcon(nullptr) { m_uMouseMsgID = uMouseMsgID; }
	UINT GetMouseMsgID() const { return m_uMouseMsgID; }
	void SetMouseMsgID(UINT uMouseMsgID) { m_uMouseMsgID = uMouseMsgID; }
	void SetTrayIcon(CTrayIcon* pTrayIcon) { m_pTrayIcon = pTrayIcon; }
	virtual void MouseMsgHandler() = 0;
protected:
	CTrayIcon* m_pTrayIcon;
private:
	UINT m_uMouseMsgID;
};

// CLeftMouseClickMsgHandler is added for the reason that CLeftMouseDblClickMsgHandler
// sometimes is not sensible enough.

class CLeftMouseClickMsgHandler : public CTrayIconMouseMsgHandler
{
public:
	CLeftMouseClickMsgHandler();
	virtual void MouseMsgHandler();
private:
	ULONGLONG m_i64LastTime;
};

class CLeftMouseDblClickMsgHandler : public CTrayIconMouseMsgHandler
{
public:
	CLeftMouseDblClickMsgHandler() {}
	virtual void MouseMsgHandler();
};

class CRightMouseClickMsgHandler : public CTrayIconMouseMsgHandler
{
public:
	CRightMouseClickMsgHandler(UINT nMenu) : CTrayIconMouseMsgHandler(WM_RBUTTONUP)
		{ m_idMenu = nMenu; }
	virtual void MouseMsgHandler();
private:
	UINT m_idMenu;
};

#endif // _TRAYICON_H_
