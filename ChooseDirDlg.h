/****************************************************************************
	File:			ChooseDirDlg.h
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

#if !defined(AFX_CHOOSEDIRDLG_H__7729A4AA_A77E_11D2_A177_0090270DD223__INCLUDED_)
#define AFX_CHOOSEDIRDLG_H__7729A4AA_A77E_11D2_A177_0090270DD223__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DriveCombo.h"
#include "DirectoryTree.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseDirDlg dialog

class CChooseDirDlg : public CDialog
{
// Construction
public:
	CChooseDirDlg( CString* psBackup, CWnd* pParent = NULL );	// standard constructor
	virtual ~CChooseDirDlg();

	void OnPathChanged(void);

// Dialog Data
	//{{AFX_DATA(CChooseDirDlg)
	enum { IDD = IDD_CHOOSEDIR_DIALOG };
	CEdit m_DirName;
	CDirectoryTree m_DirTree;
	CDriveCombo m_DriveList;
	CString	m_sFolderName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseDirDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CImageList* m_pImageList;
///	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CChooseDirDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeDriveCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEDIRDLG_H__7729A4AA_A77E_11D2_A177_0090270DD223__INCLUDED_)
