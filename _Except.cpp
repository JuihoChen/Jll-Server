// _Except.cpp: implementation of the CExcptClass class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jll Server.h"
#include "Jll ServerDoc.h"
#include "Jll ServerView.h"
#include "MainFrm.h"
#include "Except.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// determine number of elements in an array (not bytes)
#define _countof(array) (sizeof(array)/sizeof(array[0]))

IMPLEMENT_DYNAMIC( CExcptClass, CException )
IMPLEMENT_DYNAMIC( CCheckStatusException, CExcptClass )
IMPLEMENT_DYNAMIC( CInfoException, CExcptClass )
IMPLEMENT_DYNAMIC( CTimerException, CExcptClass )

LPCSTR CInfoException::TranslateCause( int nCause )
{
	static const LPCSTR rgszCFileExceptionCause[] =
	{
		"none",
		"generic",
		"fileNotFound",
		"badPath",
		"tooManyOpenFiles",
		"accessDenied",
		"invalidFile",
		"removeCurrentDir",
		"directoryFull",
		"badSeek",
		"hardIO",
		"sharingViolation",
		"lockViolation",
		"diskFull",
		"endOfFile",
	};
	static const char szUnknown[] = "unknown";

	if (nCause >= 0 && nCause < _countof(rgszCFileExceptionCause))
		return rgszCFileExceptionCause[nCause];
	else
		return szUnknown;
}

void CInfoException::Handler() const
{
	ASSERT_VALID( this );
	CString sTemp;
	switch( m_nError )
	{
	case FileIsDirAttributed:
		sTemp = "InfoError: directory attributed file info. returned.\n";
		break;
	case BadFileCountInTOC:
		sTemp = "InfoError: erroneous file count returned for TOC.\n";
		break;
	case BArchiveOverflow:
		sTemp = "InfoError: buffer archive of communication overflow.\n";
		break;
	case InvalidFindFile:
		sTemp = "InfoError: The network path was not found.\n";
		break;
	default:
		sTemp.Format( "InfoError: erroneous status <%d> thrown!\n", m_nError );
        break;
    }

	// The temporary created CWnd object couldn't used to reach the member
	// variables that we intend to access.
	///	CMainFrame* pFrame = (CMainFrame*)CWnd::FromHandle( hWnd );
	GetMyMainFrame()->m_ExceptDlg.AddStringToEdit( sTemp );
}

#ifdef _DEBUG
void CInfoException::AssertValid() const
{
    CObject::AssertValid();
}
#endif

CTimerException::CTimerException( CString& rMsghdr ) : m_strError( rMsghdr )
{
	m_strError += "remote system not responding.\r\n";
}

void CTimerException::Handler() const
{
	ASSERT_VALID( this );
	ASSERT( m_strError.IsEmpty() == FALSE );

	// The temporary created CWnd object couldn't used to reach the member
	// variables that we intend to access.
	///	CMainFrame* pFrame = (CMainFrame*)CWnd::FromHandle( hWnd );
	GetMyMainFrame()->m_ExceptDlg.AddStringToEdit( m_strError );
}

#ifdef _DEBUG
void CTimerException::AssertValid() const
{
	CObject::AssertValid();
}
#endif

void CTimer::SetTimer( CLOCK_T interval, LPCSTR msghdr /* = NULL */)
{
	ASSERT_VALID( this );
	m_strException = msghdr;
	m_fEnabled = TRUE;					// turn on the timer
	m_fTimeout = FALSE;
	m_dwInterval = interval;
	m_dwStart = GetTickCount();			// start to tick the timer
}

BOOL CTimer::CheckTimeout( BOOL fThrow /* = TRUE */)
{
	ASSERT_VALID( this );

	if( m_fEnabled )					// timer is being turned on
	{
		if( m_fTimeout == FALSE )		// not timeout yet
		{
			CLOCK_T dwElapsed = (CLOCK_T)GetTickCount() - m_dwStart;
			if( dwElapsed >= m_dwInterval )	// timeout to alarm!
			{
				m_fTimeout = TRUE;
			}
		}
		if( m_fTimeout )
		{
			if( fThrow )				// use try/catch mechanism
			{
				TRACE( "Error: Timeout waiting for busy (S6).\n" );
				THROW( new CTimerException( m_strException ) );
			}
			return TRUE;
		}
	}
	return FALSE;
}

#ifdef _DEBUG
void CTimer::AssertValid() const
{
	CObject::AssertValid();
}
#endif

//////////////////////////////////////////////////////////////////////
// QPCTimer Class
//////////////////////////////////////////////////////////////////////

QPCTimer gblQPCTimer;						// the global QPC Timer object

////int QPCTimer::m_nCountBeforeCheck;
ULONGLONG QPCTimer::m_i64Multiplier = 10;	// a number magnified by 10X
// this count for one sec. is a value on my machine "Toshiba Satellite" for reference.
const ULONGLONG QPCTimer::m_i64CountForOneSec = 0x369e99;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QPCTimer::QPCTimer()
{
	// check if high-resolution performance counter not supported.
	if( QueryPerformanceFrequency( (LARGE_INTEGER*) &m_i64Freq ) )
	{
		m_i64Multiplier = (m_i64Freq * 10) / m_i64CountForOneSec;
		if( m_i64Multiplier <= 0 ) m_i64Multiplier = 1;
	}
	else
	{
		m_i64Freq = 0;
		m_i64Multiplier = 10;
	}
}

ULONGLONG QPCTimer::GetQPCTime()
{
	ULONGLONG li;
	QueryPerformanceCounter( (LARGE_INTEGER*) &li );
	return li;
}

void QPCTimer::Delay( ULONGLONG count )
{
	ASSERT( gblQPCTimer.m_i64Freq );

	count = (count * m_i64Multiplier) / 10 * 10;	// enlarge the delay by 10 times
	ULONGLONG fromQPC = GetQPCTime(), toQPC;
    do
    {
        toQPC = GetQPCTime();
    }
    while( count > (toQPC - fromQPC) );
}

void QPCTimer::CheckTimeout() const
{
	if( (GetQPCTime() - m_i64Start) > m_i64Interval )
	{
		TRACE( "Error: Timeout in block transfer.\n" );
		THROW( new CTimerException( ":( Timeout occurred in block transfer.\r\n" ) );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CExceptDlg dialog

CExceptDlg::CExceptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExceptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExceptDlg)
	m_sExceptEdit = _T("");
	//}}AFX_DATA_INIT
}

void CExceptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExceptDlg)
	DDX_Text(pDX, IDC_EXCEPTEDIT, m_sExceptEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExceptDlg, CDialog)
	//{{AFX_MSG_MAP(CExceptDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExceptDlg message handlers

void CExceptDlg::AddStringToEdit( LPCSTR s )
{
	if( m_sExceptEdit.GetLength() > 2000 )
	{
		m_sExceptEdit.Empty();
	}
	m_sExceptEdit += s;
///	UpdateData( FALSE );	// System assertion! Cannot be executed in worker thread!
}

