/****************************************************************************
	File:			DriveCombo.cpp
	Author:			Valerie L. Bradley (val@synthcom.com)
	Date Created:	08 January 1999
	Version:		1.1
	Purpose:		Implements the Drive Combo Box control

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
#include "DriveCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Definitions
#define MAX_BUFFER				1024		// Maximum buffer size

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDriveCombo, CComboBox)
	//{{AFX_MSG_MAP(CDriveCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CDriveCombo
CDriveCombo::CDriveCombo()
{
}

CDriveCombo::~CDriveCombo()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDriveCombo message handlers

/****************************************************************************
	Function:	Initialize()
	Purpose:	Reads the drives on the system and fills in itself with them,
				then it highlights the current drive letter.
	Inputs:		None
	Outputs:	BOOL -- Whether or not the operation was successful
****************************************************************************/
BOOL CDriveCombo::Initialize()
{
	BOOL    bFound = FALSE;	// Whether volume info was found or not
	DWORD   dwRes  = 0;		// Receives the drive bitmask
	int     i      = 0;		// Misc. counter variable
	CString str;			// Used for string manipulations
	
	char  szVolInfo[MAX_BUFFER];

	// Get the bitmask of available drives
	dwRes = GetLogicalDrives();
	if (0 == dwRes)
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
					  NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					  (LPTSTR) &lpMsgBuf, 0, NULL);
		MessageBox((LPTSTR)lpMsgBuf, "GetLastError - GetLogicalDrives failed", MB_OK|MB_ICONERROR);
		LocalFree(lpMsgBuf);
		return FALSE;
	}

	// Separate out each drive letter and add it to the Combo Box
	while (dwRes)
	{
		if (dwRes & 1)
		{
			// There's a drive for this bit position.
			str.Format("%c: ", 'A' + i);

			// Get the name of the drive, if it exists
			SetErrorMode(SEM_FAILCRITICALERRORS);
			bFound = GetVolumeInformation(str, szVolInfo, MAX_BUFFER, NULL, NULL, NULL, NULL, NULL);
			if (bFound)
				str += szVolInfo;

			// Add the drive information to the combo box
			AddString(str);
		}

		dwRes >>= 1;
		i++;
	}

	// Get the current directory, so we know what to highlight.
	dwRes = GetCurrentDirectory(MAX_BUFFER, szVolInfo);
	if (0 == dwRes)
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
					  NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					  (LPTSTR) &lpMsgBuf, 0, NULL);
		MessageBox((LPTSTR)lpMsgBuf, "GetLastError - GetCurrentDirectory failed", MB_OK|MB_ICONERROR);
		LocalFree(lpMsgBuf);
		return FALSE;
	}

	ResetDrive( szVolInfo[0] );
	return TRUE;
}


/****************************************************************************
	Function:	IsDriveReady()
	Purpose:	Tests whether or not the indicated drive is ready.
	Inputs:		int nIndex -- The drive to test
	Outputs:	BOOL -- Whether or not the operation was successful
****************************************************************************/
BOOL CDriveCombo::IsDriveReady(int nIndex)
{
	HANDLE          hFile;			// Handle to found file
	WIN32_FIND_DATA stFindData;		// Info about the found file
	CString         szDrive;		// Drive letter to test
	CString         szSearch;		// Search string
	CString         szError;		// Error message

	GetLBText(nIndex, szDrive);
	szSearch = szDrive.Left(1);
	szSearch += ":\\*.*";

	// First, see if there's anything in the directory
	hFile = FindFirstFile((LPCTSTR)szSearch, &stFindData);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		szError.Format("Drive %s: is not ready.", szDrive.Left(1));
		MessageBox(szError, "Error", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	FindClose(hFile);

	return TRUE;
}


/****************************************************************************
	Function:	ResetDrive()
	Purpose:	Resets the current selection to the drive letter specified.
	Inputs:		char cDrive -- The drive letter to select
	Outputs:	None
****************************************************************************/
void CDriveCombo::ResetDrive(char cDrive)
{
	CString szStr;
	int     nIndex = 0;

	szStr.Format("%c:", cDrive);
	nIndex = FindString(-1, szStr);
	if (CB_ERR != nIndex)
		SetCurSel(nIndex);
}