// dirpkr.h : main header file for the DIRPKR application
//

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

class CMyFileDlg : public CFileDialog
{
public:
    
// Public data members

   BOOL m_bDlgJustCameUp;
    
// Constructors

    CMyFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
               LPCTSTR lpszDefExt = NULL,
               LPCTSTR lpszFileName = NULL,
               DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
               LPCTSTR lpszFilter = NULL,
               CWnd* pParentWnd = NULL);
                                          
// Implementation
protected:
    //{{AFX_MSG(CMyFileDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
