/****************************************************************************
	File:			ChooseDirDlg.cpp
	Author:			Valerie L. Bradley (val@synthcom.com)
	Date Created:	08 January 1999
	Version:		1.1
	Purpose:		Demonstrates how the Directory Tree control and the
					Drive Combo Box control can be used in an application.

	© Copyright 1999 Valerie Bradley.
    All rights reserved.

	This source code is public domain.  I give full permission to all 
	developers to freely use this code in their own applications, commercial
	or otherwise.

	This source code is provided "AS IS" without warranty of any kind,
	including, but not limited to, any warranty of merchantability, fitness
	for any particular purpose, or any warranty otherwise arising out of any
	proposal, specification or sample.  In no event shall I be liable for
	any damages whatsoever arising out of the use of or inability to use
	this source code.

****************************************************************************/

#include "stdafx.h"
#include "Jll Server.h"
#include "ChooseDirDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseDirDlg dialog

CChooseDirDlg::CChooseDirDlg( CString* psBackup, CWnd* pParent /*=NULL*/)
	: m_DirTree( psBackup ), CDialog(CChooseDirDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseDirDlg)
	m_sFolderName = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	///m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CChooseDirDlg::~CChooseDirDlg()
{
	delete m_pImageList;
}

void CChooseDirDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseDirDlg)
	DDX_Control(pDX, IDC_DIRECTORY_NAME, m_DirName);
	DDX_Control(pDX, IDC_DIRECTORY_TREE, m_DirTree);
	DDX_Control(pDX, IDC_DRIVE_COMBO, m_DriveList);
	DDX_Text(pDX, IDC_DIRECTORY_NAME, m_sFolderName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CChooseDirDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseDirDlg)
	ON_CBN_SELCHANGE(IDC_DRIVE_COMBO, OnSelchangeDriveCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseDirDlg message handlers

BOOL CChooseDirDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

/*********************** JHC ***
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
********************************/

	// Set up the image list for the tree control
	m_pImageList = new CImageList();
	m_pImageList->Create(16, 16, ILC_COLOR, 4, 4);

	// Load the bitmaps.  This must be done before calling Initialize()
	// on the CDirectoryTree object.
	CBitmap	bBitmap;
	for (int nID = IDB_FIRST_TREE_ICON; nID <= IDB_LAST_TREE_ICON; nID++)
	{
		bBitmap.LoadMappedBitmap(nID);
		m_pImageList->Add(&bBitmap, (COLORREF)0x000000);
		bBitmap.DeleteObject();
	}
	m_DirTree.SetBitmapList(m_pImageList);

	// Initialize the custom controls
	m_DriveList.Initialize();
	m_DirTree.Initialize();
	m_DirName.SetWindowText(m_DirTree.GetCurrentDir());

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

//DEL void CChooseDirDlg::OnPaint() 
//DEL {
//DEL 	if (IsIconic())
//DEL 	{
//DEL 		CPaintDC dc(this); // device context for painting
//DEL 
//DEL 		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
//DEL 
//DEL 		// Center icon in client rectangle
//DEL 		int cxIcon = GetSystemMetrics(SM_CXICON);
//DEL 		int cyIcon = GetSystemMetrics(SM_CYICON);
//DEL 		CRect rect;
//DEL 		GetClientRect(&rect);
//DEL 		int x = (rect.Width() - cxIcon + 1) / 2;
//DEL 		int y = (rect.Height() - cyIcon + 1) / 2;
//DEL 
//DEL 		// Draw the icon
//DEL 		dc.DrawIcon(x, y, m_hIcon);
//DEL 	}
//DEL 	else
//DEL 	{
//DEL 		CDialog::OnPaint();
//DEL 	}
//DEL }

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
//DEL HCURSOR CChooseDirDlg::OnQueryDragIcon()
//DEL {
//DEL 	return (HCURSOR) m_hIcon;
//DEL }


/****************************************************************************
	Function:	PreTranslateMessage()
	Purpose:	Look for the message that indicates the path has changed so
				the edit box can be updated properly.
				NOTE: DON'T ADD YOUR OWN MESSAGE HANDLER AND MESSAGE MAP ENTRY.
				For whatever reason, this causes an access violation crash.
				If you do it this way, everything works just fine.
	Inputs:		MSG* pMsg -- The message information
	Outputs:	BOOL -- Whether or not the operation succeeded
****************************************************************************/
BOOL CChooseDirDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (WM_USER_PATHCHANGED == pMsg->message)
	{
		OnPathChanged();
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


/****************************************************************************
	Function:	OnSelchangeDriveCombo()
	Purpose:	Called when the selection in the drives combo box changes.
				Re-initializes the tree control to display the new drive's
				directories.
	Inputs:		None
	Outputs:	None
****************************************************************************/
void CChooseDirDlg::OnSelchangeDriveCombo() 
{
	int     nSelected = 0;
	CString szText;

	// Determine the new drive letter and test to see if it's valid.
	nSelected = m_DriveList.GetCurSel();
	if (!m_DriveList.IsDriveReady(nSelected))
	{
		// The drive wasn't valid!!  Revert to the previous one
		m_DriveList.ResetDrive(m_DirTree.GetCurrentDrive());
		return;
	}

	// Get the drive letter and communicate it to the tree.
	m_DriveList.GetLBText(nSelected, szText);
	m_DirTree.SetCurrentDrive(szText[0]);

	// Re-initialize the tree to show the new drive's directories.
	m_DirTree.Initialize();
	m_DirName.SetWindowText(m_DirTree.GetCurrentDir());
}


/****************************************************************************
	Function:	OnPathChanged()
	Purpose:	Called when the selection in the directory tree changes.
				Re-initializes the edit control to display the new path.
	Inputs:		None
	Outputs:	None
****************************************************************************/
void CChooseDirDlg::OnPathChanged()
{
	m_DirName.SetWindowText(m_DirTree.GetCurrentDir());
}

