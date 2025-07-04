/****************************************************************************
	File:			DirectoryTree.h
	Author:			Valerie L. Bradley (val@synthcom.com)
	Date Created:	08 January 1999
	Version:		1.1
	Purpose:		Header for the Directory Tree control implementation

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

#if !defined(AFX_DIRECTORYTREE_H__08D4F1E1_A780_11D2_A177_0090270DD223__INCLUDED_)
#define AFX_DIRECTORYTREE_H__08D4F1E1_A780_11D2_A177_0090270DD223__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Definitions
#define WM_USER_PATHCHANGED		WM_USER+10	// Message to incicate the path changed

/////////////////////////////////////////////////////////////////////////////
// CDirectoryTree window

class CDirectoryTree : public CTreeCtrl
{
	DECLARE_DYNAMIC(CDirectoryTree)

// Construction
public:
	CDirectoryTree( CString* psBackup );

// Attributes
protected:
	CString   m_szCurrentDir;		// Holds the currently highlighted directory
	char      m_cCurrentDrive;		// Holds the letter of the current drive
	
	int       n_TreeIconClosed;
	int       n_TreeIconClosedSel;
	int       n_TreeIconOpen;
	int       n_TreeIconOpenSel;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDirectoryTree)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDirectoryTree();
    
	BOOL    Initialize(void);
	void    AddDirsToTree(CString, HTREEITEM);
	void    BuildPathFromTree(HTREEITEM);
	void    DeleteChildren(HTREEITEM);
	void    DeletePeers(HTREEITEM);
	void    SetBitmapList(CImageList *);

	// Inline functions
	CString GetCurrentDir()         { return m_szCurrentDir; }
	char    GetCurrentDrive()       { return m_cCurrentDrive; }
	void    SetCurrentDrive(char c) { m_cCurrentDrive = c; }

	// Generated message map functions
protected:
	CString* const m_psBackupDirs;
	//{{AFX_MSG(CDirectoryTree)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTORYTREE_H__08D4F1E1_A780_11D2_A177_0090270DD223__INCLUDED_)
