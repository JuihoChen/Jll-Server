/****************************************************************************
	File:			DirectoryTree.cpp
	Author:			Valerie L. Bradley (val@synthcom.com)
	Date Created:	08 January 1999
	Version:		1.1
	Purpose:		Implements the Directory Tree control

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
#include "resource.h"
#include "DirectoryTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Definitions
#define MAX_DIR_LENGTH 1024

/////////////////////////////////////////////////////////////////////////////
// CDirectoryTree

CDirectoryTree::CDirectoryTree( CString* psBackup ) : m_psBackupDirs( psBackup )
{
	// Initialize Member Variables
	m_szCurrentDir.Empty();
	m_cCurrentDrive = 'a';	// Don't change this, it's a special marker!!

	// Initialize the icon IDs to their default values
	n_TreeIconClosed    = 0;
	n_TreeIconClosedSel = 1;
	n_TreeIconOpen      = 2;
	n_TreeIconOpenSel   = 3;
}

CDirectoryTree::~CDirectoryTree()
{
}


BEGIN_MESSAGE_MAP(CDirectoryTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CDirectoryTree)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirectoryTree message handlers

/****************************************************************************
	Function:	Initialize()
	Purpose:	Reads in the directory structure and highlights the current
				directory.
	Inputs:		None 
	Outputs:	BOOL -- Whether or not the operation was successful
****************************************************************************/
BOOL CDirectoryTree::Initialize()
{
	HTREEITEM       hTreeItem;				// Result of adding new item to tree
	HTREEITEM       hTreeParent;			// Parent of recently added item
	DWORD           dwRes = 0;				// Result code
	TV_INSERTSTRUCT tvstruct;				// Info for inserting Tree items
	int             nNumNodes = 0;			// Number of nodes initialized
	int             i = 0;					// Generic counter variable
	char            szPath[MAX_DIR_LENGTH];	// Buffer to hold path information
	char            szSeps[] = "\\";		// Separators to use with _tcstok()
	char *          token;					// Buffer to hold the token returned by _tcstok()

	// Determine if this is the first time we've initialized.
	// If m_cCurrentDrive is 'a', this is the first time.  Otherwise,
	// change the directory to the new one, empty the tree control, and
	// then continue as normal.
	if ('a' != m_cCurrentDrive)
	{
		if( m_psBackupDirs[ m_cCurrentDrive - 'A' ].IsEmpty() )
		{
			CString s;
			s.Format("%c:", m_cCurrentDrive);
			::SetCurrentDirectory(s);
		}
		else
		{
			::SetCurrentDirectory( m_psBackupDirs[ m_cCurrentDrive - 'A' ] );
		}
		DeleteAllItems();
	}

	// Get the current directory, so we know what to highlight.
	dwRes = GetCurrentDirectory(MAX_DIR_LENGTH, szPath);
	if (0 == dwRes)
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
					  NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					  (LPTSTR) &lpMsgBuf, 0, NULL);
		MessageBox((LPTSTR)lpMsgBuf, "GetLastError", MB_OK|MB_ICONERROR);
		LocalFree(lpMsgBuf);
		return FALSE;
	}

	// Backup the directory name if necessary.
	if( m_psBackupDirs[ szPath[0] - 'A' ].IsEmpty() )
	{
		m_psBackupDirs[ szPath[0] - 'A' ] = szPath;
	}

	// Save off the current path and drive letter.
	m_szCurrentDir = szPath;
	m_cCurrentDrive = szPath[0];

	// Fill in the tree starting from the drive down to the current dir.
	hTreeParent = TVI_ROOT;
	token =  _tcstok(szPath, szSeps);
	while (token)
	{
		// Add this item to the tree
		tvstruct.hParent = hTreeParent;
		tvstruct.hInsertAfter = TVI_LAST;
		tvstruct.item.iImage = n_TreeIconOpen;
		tvstruct.item.iSelectedImage = n_TreeIconOpenSel;
		tvstruct.item.pszText = token;
		tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
		hTreeItem = InsertItem(&tvstruct);
		hTreeParent = hTreeItem;
		nNumNodes++;	// Keep track of the number of nodes

		token = _tcstok(NULL, szSeps);
	}

	// Automatically expand tree, select the last one, and show its subdirectories.
	hTreeParent = hTreeItem = NULL;
	hTreeItem   = GetRootItem();
	for (i=0; i<nNumNodes; i++)
	{
		hTreeParent = hTreeItem;
		Expand(hTreeParent, TVE_EXPAND);
		hTreeItem = GetChildItem(hTreeParent);
	}
	SelectItem(hTreeParent);
	AddDirsToTree(m_szCurrentDir, hTreeParent);

	return TRUE;
}


/****************************************************************************
	Function:	AddDirsToTree()
	Purpose:	Enumerates the subdirectories underneath the provided
				starting directory and adds them to the tree.
	Inputs:		CString szStart -- The starting directory
	Outputs:	None
****************************************************************************/
void CDirectoryTree::AddDirsToTree(CString szStart, HTREEITEM htParent)
{
	CString         szStartDir;		// Local copy of szStart
	BOOL            bFound;			// Whether or not a new file has been found
	HANDLE          hFile;			// Handle to found file
	WIN32_FIND_DATA stFindData;		// Info about the found file
	HTREEITEM       hTreeItem;		// Result of adding new item to tree
	TV_INSERTSTRUCT tvstruct;		// Info for inserting Tree items

	// Append "*.*" to the end of the directory name passed in
	szStartDir.Format("%s\\*.*", szStart);

	// First, see if there's anything in the directory
	hFile = FindFirstFile((LPCTSTR)szStartDir, &stFindData);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBox("FindFirstFile() returned INVALID_HANDLE_VALUE", "DEBUG", MB_OK);
		return;
	}

	// Next, loop through everything in the directory.  If the found file
	// is itself a directory, add it to the tree.
	while (INVALID_HANDLE_VALUE != hFile)
	{
		if (stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((0 == strcmp(stFindData.cFileName, ".")) || (0 == strcmp(stFindData.cFileName, "..")))
				;	// Skip over directories named "." and ".."
			else
			{
				// Add this item to the tree
				tvstruct.hParent = htParent;
				tvstruct.hInsertAfter = TVI_SORT;
				tvstruct.item.iImage = n_TreeIconClosed;
				tvstruct.item.iSelectedImage = n_TreeIconClosedSel;
				tvstruct.item.pszText = stFindData.cFileName;
				tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
				hTreeItem = InsertItem(&tvstruct);
			}
		}

		bFound = FindNextFile(hFile, &stFindData);
		if (!bFound)
			break;
	}
	FindClose(hFile);

	// Expand the new list of directories
	Expand(htParent, TVE_EXPAND);

	return;
}


/****************************************************************************
	Function:	BuildPathFromTree()
	Purpose:	Builds the complete path from the selected item in the tree,
				and sets m_szCurrentDir equal to that new string.
	Inputs:		HTREEITEM htItem -- the end of the path
	Outputs:	None
****************************************************************************/
void CDirectoryTree::BuildPathFromTree(HTREEITEM htItem)
{
	CString szFoo;
	CString szBar;
	HTREEITEM htItem1;
	HTREEITEM htItem2;

	szFoo = GetItemText(htItem);
	htItem1 = GetParentItem(htItem);
	while (NULL != htItem1)
	{
		szBar = GetItemText(htItem1);
		szFoo = szBar + "\\" + szFoo;
		htItem2 = GetParentItem(htItem1);
		htItem1 = htItem2;
	}

	// Copy the new path
	m_szCurrentDir = szFoo;

	// Send notification to the parent window that the path changed.
	CWnd * pWnd = GetParent();
	if (pWnd)
		pWnd->PostMessage(WM_USER_PATHCHANGED);

	// backup current directory for record on the very dirve
	m_psBackupDirs[ m_szCurrentDir[0] - 'A' ] = GetCurrentDir();
}


/****************************************************************************
	Function:	DeleteChildren()
	Purpose:	Deletes the child items of the item that was just expanded.
				Because of the way we're doing this, there will always be two
				tiers to remove.
	Inputs:		HTREEITEM htItem -- the end of the path
	Outputs:	None
****************************************************************************/
void CDirectoryTree::DeleteChildren(HTREEITEM htItem)
{
	HTREEITEM hTreeChild1;	// A child of the item
	HTREEITEM hTreeChild2;	// A child of the item

	if (ItemHasChildren(htItem))
	{
		hTreeChild1 = GetChildItem(htItem);
		if (hTreeChild1)
		{
			if (ItemHasChildren(hTreeChild1))
			{
				hTreeChild2 = GetChildItem(hTreeChild1);
				while (hTreeChild2)
				{
					DeleteItem(hTreeChild2);
					hTreeChild2 = GetChildItem(hTreeChild1);
				}
			}

			DeleteItem(hTreeChild1);
		}
	}

	return;
}


/****************************************************************************
	Function:	DeletePeers()
	Purpose:	Deletes the peer items of the item that was just expanded.
				This cleans up the tree so that only this item and its
				children are being displayed.
	Inputs:		HTREEITEM htItem -- the end of the path
	Outputs:	None
****************************************************************************/
void CDirectoryTree::DeletePeers(HTREEITEM htItem)
{
	HTREEITEM hTreePeer;

	// Remove all preceeding siblings
	hTreePeer = GetPrevSiblingItem(htItem);
	while (hTreePeer)
	{
		DeleteItem(hTreePeer);
		hTreePeer = GetPrevSiblingItem(htItem);
	}

	// Remove all trailing siblings
	hTreePeer = GetNextSiblingItem(htItem);
	while (hTreePeer)
	{
		DeleteItem(hTreePeer);
		hTreePeer = GetNextSiblingItem(htItem);
	}

	return;
}


/****************************************************************************
	Function:	OnDblclk()
	Purpose:	Handler for the NM_DBLCLK message.  This message is sent when
				an item in the tree has been double-clicked.  This function
				fills in the rest of the subdirectories when the user changes
				to a new directory by double-clicking in the tree.
	Inputs:		NMHDR* pNMHDR    -- Information about the notification
									message
				LRESULT* pResult -- The result of the operation (??)
	Outputs:	None
****************************************************************************/
void CDirectoryTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM hTreeItem;		// The item's Handle
	HTREEITEM hTreeChild;		// A child of the item
	UINT      uItemState;		// The item's current state
	int       nImage;			// The ID of the image in the list
	int       nSelImage;		// The ID of the selected image in the list

	// First, find out which item is selected and get its information
	hTreeItem  = GetSelectedItem();
	uItemState = GetItemState(hTreeItem, TVIF_STATE);

	if (uItemState & TVIS_EXPANDED)
	{
		// If its children have the "closed" icon, then don't do anything.
		if (ItemHasChildren(hTreeItem))
		{
			hTreeChild = GetChildItem(hTreeItem);
			if (hTreeChild)
			{
				GetItemImage(hTreeChild, nImage, nSelImage);
				if ((nImage == n_TreeIconClosed) || 
					(nSelImage == n_TreeIconClosedSel))
				{
					*pResult = 1;
					return;
				}
			}
		}

		// Otherwise, delete the children of this item, determine the
		// new root directory, and rebuild from there.
		DeleteChildren(hTreeItem);
		BuildPathFromTree(hTreeItem);
		AddDirsToTree(m_szCurrentDir, hTreeItem);
	}
	else
	{
		// First, change this item's icon to an open folder.
		SetItemImage(hTreeItem, 2, 3);

		// Next, delete all the peers of this item, determine the new root
		// directory, and display its children
		DeletePeers(hTreeItem);
		BuildPathFromTree(hTreeItem);
		AddDirsToTree(m_szCurrentDir, hTreeItem);
	}

	// We never want the tree to automatically expand or collapse... we're
	// redefining that behavior!
	*pResult = 1;
	return;
}


/****************************************************************************
	Function:	SetBitmapList()
	Purpose:	Initializes the CImageList member variable with the
				CImageList object passed in.  It will be used in function
				Initialize() to initialize the tree's image list.
				
				NOTE: The images must be loaded in this order to be used
				properly:
				1) Closed file folder image
				2) Selected closed file folder image
				3) Open file folder image
				4) Selected open file folder image

	Inputs:		CBitmap * pBmpList -- the list of images
	Outputs:	None

	NOTE: This function can not be used in conjunction with SetBitmapItem().
****************************************************************************/
void CDirectoryTree::SetBitmapList(CImageList * pBmpList)
{
	SetImageList(pBmpList, TVSIL_NORMAL);
}


/****************************************************************************
	Function:	SetBitmapOrder()
	Purpose:	Sets the internal variables on which bitmap is which, so that
				the user doesn't have to use the prescribed order.
	Inputs:		int nClosed    -- Which item in list is closed folder
				int nClosedSel -- Which item in list is selected closed folder
				int nOpen      -- Which item in list is open folder
				int nOpenSel   -- Which item in list is selected open folder
	Outputs:	None
****************************************************************************/
//DEL void CDirectoryTree::SetBitmapOrder(int nClosed = 0, int nClosedSel = 1, int nOpen = 2, int nOpenSel = 3)
//DEL {
//DEL 	n_TreeIconClosed    = nClosed;
//DEL 	n_TreeIconClosedSel = nClosedSel;
//DEL 	n_TreeIconOpen      = nOpen;
//DEL 	n_TreeIconOpenSel   = nOpenSel;
//DEL }


IMPLEMENT_DYNAMIC(CDirectoryTree, CTreeCtrl)
