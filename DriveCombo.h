/****************************************************************************
	File:			DriveCombo.h
	Author:			Valerie L. Bradley (val@synthcom.com)
	Date Created:	08 January 1999
	Version:		1.1
	Purpose:		Header for the Drive Combo Box control implementation

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

#if !defined(AFX_DRIVECOMBO_H__08D4F1E2_A780_11D2_A177_0090270DD223__INCLUDED_)
#define AFX_DRIVECOMBO_H__08D4F1E2_A780_11D2_A177_0090270DD223__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CDriveCombo window

class CDriveCombo : public CComboBox
{
// Construction
public:
	CDriveCombo();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDriveCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDriveCombo();

	BOOL Initialize(void);		// Initializes the combo box control
	BOOL IsDriveReady(int);		// Whether or not the drive is ready
	void ResetDrive(char);		// Resets the combo box to the last good drive

	// Generated message map functions
protected:
	//{{AFX_MSG(CDriveCombo)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRIVECOMBO_H__08D4F1E2_A780_11D2_A177_0090270DD223__INCLUDED_)
