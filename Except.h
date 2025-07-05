// Except.h: interface for the CExcptClass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCEPT_H__776D2795_0654_436D_85AF_26C4EAFDD982__INCLUDED_)
#define AFX_EXCEPT_H__776D2795_0654_436D_85AF_26C4EAFDD982__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef DWORD CLOCK_T;

class CExcptClass : public CException  
{
    DECLARE_DYNAMIC( CExcptClass )
protected:
    // CObject cannot use copy constructor & assignment by default.
    // If CExcptClass would like to be caught by a 'reference', ...
    CExcptClass() {}
    CExcptClass( const CExcptClass& ) {}
public:
    // virtual destructors are necessary
    virtual ~CExcptClass() {}
    virtual void Handler() const = 0;
};

class CCheckStatusException : public CExcptClass
{
	DECLARE_DYNAMIC( CCheckStatusException )
public:
	CCheckStatusException( int nStatus ) : m_nError( nStatus ) {}
	virtual void Handler() const;
private:
	int m_nError;
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};

class CInfoException : public CExcptClass
{
	DECLARE_DYNAMIC( CInfoException )
public:
	enum EInfo {
		OkInfo = 0,
		FileIsDirAttributed,
		BadFileCountInTOC,
		BArchiveOverflow
	};
	CInfoException( int nStatus ) : m_nError( nStatus ) {}
	virtual void Handler() const;
private:
	int m_nError;
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};

class CTimerException : public CExcptClass
{
	DECLARE_DYNAMIC( CTimerException )
public:
	CTimerException( CString& rMsg );
	CTimerException( LPCTSTR pMessage ) { m_strError = pMessage; }
	virtual void Handler() const;
private:
	CString m_strError;
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};

class CTimer : CObject
{
public:
	CTimer( CLOCK_T interval, LPCSTR msghdr = NULL ) { SetTimer( interval, msghdr ); }
	CTimer() { m_fEnabled = FALSE; }
	void SetTimer( CLOCK_T interval, LPCSTR msghdr = NULL );
	void SetMsghdr( LPCSTR msghdr ) { m_strException = msghdr; }
	BOOL CheckTimeout( BOOL fThrow = TRUE );
	BOOL IsTimeout() const { ASSERT_VALID( this ); return m_fTimeout; }
	BOOL m_fEnabled;
private:
	BOOL m_fTimeout;
	CLOCK_T m_dwInterval;
	CLOCK_T m_dwStart;
	CString m_strException;
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};

// a high resolution timer utilizing QPC & QPF functions.
class QPCTimer : public CObject  
{
public:
	QPCTimer();
	void SetTimer( int nSec )
		{ m_i64Start = GetQPCTime(); m_i64Interval = m_i64Freq * nSec; }
	void ResetCounter()
		{ m_nCountBeforeCheck = 0; }
    void CounterExceedToCheck( int nNum = 3000 )
		{ if( m_nCountBeforeCheck <= nNum ) m_nCountBeforeCheck ++; else CheckTimeout(); }
	void CheckTimeout() const;
	static ULONGLONG GetQPCTime();
	static void Delay( ULONGLONG count );
protected:
	int m_nCountBeforeCheck;
	ULONGLONG m_i64Start;
	ULONGLONG m_i64Interval;
	ULONGLONG m_i64Freq;
	static ULONGLONG m_i64Multiplier;	// a number magnified by 10X
	static const ULONGLONG m_i64CountForOneSec;
};

extern QPCTimer gblQPCTimer;			// the global QPC Timer object

/////////////////////////////////////////////////////////////////////////////
// CExceptDlg dialog

class CExceptDlg : public CDialog
{
// Construction
public:
	CExceptDlg(CWnd* pParent = NULL);   // standard constructor
	void AddStringToEdit( LPCSTR s );

// Dialog Data
	//{{AFX_DATA(CExceptDlg)
	enum { IDD = IDD_EXCEPTBOX };
	CString	m_sExceptEdit;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExceptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CExceptDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_EXCEPT_H__776D2795_0654_436D_85AF_26C4EAFDD982__INCLUDED_)
