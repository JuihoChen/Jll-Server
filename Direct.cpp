// Direct.cpp: implementation of the CDirectCable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jll Server.h"
#include "MainFrm.h"
#include "except.h"
#include "Direct.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define  _FI_LEN    20

IMPLEMENT_DYNAMIC( CDirectCable, CObject )
IMPLEMENT_DYNAMIC( CDCServer, CDirectCable )

BYTE CDirectCable::abTestData[16] =
	{ 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

const int CDirectCable::m_nMaxSizeBase = 1024;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectCable::CDirectCable( CNibbleModeProto& lpt ) : m_rNibbleModeDev( lpt )
{
	ASSERT_VALID( &lpt );
	m_pfnSendFromBuffer = SendFromBuffer;
	m_pfnReceiveIntoBuffer = ReceiveIntoBuffer;
	m_fpBuffer = (BYTE FAR*)malloc( 65536 );
	ASSERT( m_fpBuffer != NULL);
}

CDirectCable::~CDirectCable()
{
	ASSERT_VALID( this );
	if( m_fpBuffer != NULL )
		free( m_fpBuffer );
	DeleteBase();
}

#pragma check_stack( off )

void CDirectCable::SendFromBuffer( UINT nLen ) const
{
	ASSERT( nLen > 0 && nLen <= BF_MAXLEN );

	m_rNibbleModeDev.SetPollCounter( 0 );
	for( register UINT n = 0; -- nLen; n ++ )
	{
		m_rNibbleModeDev.WriteByteToPort( m_fpBuffer[ n ] );
	}

	m_rNibbleModeDev.SetPollCounter( 500 );
	m_rNibbleModeDev.WriteByteToPort( m_fpBuffer[ n ] );
}

void CDirectCable::ReceiveIntoBuffer( register UINT nIndex, UINT nLen )
// the reason to use param nIndex is that data from the opposite
// could be sent in different packets. (ex. header + data)

// NOTE: It's WIERD!
// register declarator for nIndex is necessary to fix MSC (v8.00c)
// compiler's BUG, if programmed as deleted code (2 lines below).
{
	ASSERT( nIndex >= 0 && nLen > 0 );
	ASSERT( nIndex <= BF_MAXLEN && (nIndex + nLen - 1) <= BF_MAXLEN );
	while( nLen -- )
	{
/** these 2 lines below is replaced with for speeding up. ***
///        m_rNibbleModeDev.ReadByteFromPort();
///        m_fpBuffer[ nIndex ++ ] = m_rNibbleModeDev.GetByteRead();
***/
		m_fpBuffer[ nIndex ++ ] = m_rNibbleModeDev.ReadByteFromPort();
	}
}

#ifdef POLL_S7_FOR_BUSY
void CDirectCable::SendFromBufferInByte( UINT nLen ) const
{
	ASSERT( nLen > 0 && nLen <= BF_MAXLEN );

	m_rNibbleModeDev.SetPollCounter( 0 );
	for( register UINT n = 0; nLen > 2; n += 2, nLen -= 2 )
	{
		m_rNibbleModeDev.WriteByteToPortInByte( *(WORD FAR*)(m_fpBuffer +n) );
	}

	m_rNibbleModeDev.SetPollCounter( 500 );
	m_rNibbleModeDev.WriteByteToPortInByte( *(WORD FAR*)(m_fpBuffer + n) );
}

void CDirectCable::ReceiveIntoBufferInByte( register UINT nIndex, UINT nLen )
{
	ASSERT( nIndex >= 0 && nLen > 0 );
	ASSERT( nIndex <= BF_MAXLEN && (nIndex + nLen - 1) <= BF_MAXLEN );

	m_rNibbleModeDev.MakeControl4Input();
	while( 1 )
	{
		*(WORD FAR*)(m_fpBuffer + nIndex) = m_rNibbleModeDev.ReadByteFromPortInByte();
		if( nLen <= 2 ) break;		// received the last word? (1 or 2)
		nIndex += 2;
		nLen -= 2;
	}
}
#endif

#pragma check_stack()

void CDirectCable::FillBuffer( UINT nIndex, BYTE* src, UINT nLen )
{
	ASSERT_VALID( this );
	ASSERT( src );
	ASSERT( nIndex >= 0 && nLen > 0 );
	ASSERT( nIndex <= BF_MAXLEN && (nIndex + nLen - 1) <= BF_MAXLEN );
	memcpy( m_fpBuffer + nIndex, src, nLen );
}

void CDirectCable::SetString( UINT nIndex, CString& rString )
{
	int nLen = rString.GetLength();		// not include the null terminator

	ASSERT_VALID( this );
	ASSERT( nIndex >= 0 );
	ASSERT( nIndex <= BF_MAXLEN && (nIndex + nLen - 1) <= BF_MAXLEN );

	memcpy( m_fpBuffer + nIndex, (const char*)rString, nLen + 1 );
}

void CDirectCable::DeleteBase()
{
	ASSERT_VALID( this );
	for( int i = 0; i < m_aFiInfoBase.GetSize(); i ++ )
	{
		delete m_aFiInfoBase[ i ];
	}
	m_aFiInfoBase.RemoveAll();
}

#ifdef _DEBUG
void CDirectCable::AssertValid() const
{
	CObject::AssertValid();
	m_rNibbleModeDev.AssertValid();
}
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDCServer::CDCServer( CNibbleModeProto& lpt ) : CDirectCable( lpt )
{
	InvalidateOpcode();				// Invalidate Opcode
	m_pThread = NULL;				// Invalidate thread pointer
}

CDCServer::~CDCServer()
{
	_OutputDebugString( "CDCServer::Destructor called.\n" );

	Kill();							// Kill the other thread anyway.
    for( int i = 0; i < 10 && m_pThread; i ++ )
	{
		::Sleep( 20 );				// Give the other thread chances to quit.
	}
	if( m_pThread )
	{
		TRACE0( "CDCServer: *** Warning: deleting active thread! ***\n" );
		delete m_pThread;
	}
}

void CDCServer::ParseWorkDir( CString sFolderName )
{
	// The passed foldername should be an existent and been-checked folder name.
	char szShort[_MAX_PATH];
	::GetShortPathName( sFolderName, szShort, sizeof szShort );

	m_sDirName = szShort;
	m_sFileName = _ALLFILES;

	// CString::MakeUpper makes some chinese-named directories abnormal.
	m_sDirName.SetAt( 0, toupper( m_sDirName.GetAt( 0 ) ) );

	TRACE1( "Dir name = <%s>, ", m_sDirName );
	TRACE1( "File name = <%s>\n", m_sFileName );
}

void CDCServer::ReceiveCommand()
{
	ASSERT_VALID( this );

	InvalidateOpcode();							// Invalidate Opcode

	m_tmrWaitS6.SetTimer( 5000, "R:> " );		// turn on the timer

	for( int i = 0; i < sizeof( m_bCDB ); i ++ )
	{
		char sz[10];
		sprintf( sz, "R%1.1d:> ", i );
		m_tmrWaitS6.SetMsghdr( sz );			// change message header

/** these 2 lines below is replaced with for speeding up. ***
///        m_rNibbleModeDev.ReadByteFromPort( m_tmrWaitS6 );
///        m_bCDB[ i ] = m_rNibbleModeDev.GetByteRead();
***/
		m_bCDB[ i ] = m_rNibbleModeDev.ReadByteFromPort( m_tmrWaitS6 );
	}

	ASSERT( m_bCDB[0] != IllegalOpcode );
}

void CDCServer::RetCheckStatus( int nStatus )
{
	ASSERT_VALID( this );
	ASSERT( m_tmrWaitS6.m_fEnabled == TRUE );	// the timer was turned on

	//***JHC* Here has to be some delay for the opposite polling *JHC***
	gblQPCTimer.Delay( 600 );					// (adequate) delay for next command.

	m_tmrWaitS6.SetTimer( 5000, "SC:> " );		// re-turn on timer for message
	m_rNibbleModeDev.WriteNibbleToPort( nStatus, m_tmrWaitS6 );
	m_tmrWaitS6.m_fEnabled = FALSE;				// turn off the timer
}

void CDCServer::SendFileInfo()
{
	ASSERT_VALID( this );
	ASSERT( m_sFileName.IsEmpty() == FALSE );

	BYTE bAttribute = GetAt( 1 );				// b7:Lock
	int nIndex = (short) GetWord( 2 );			// int has 32 bits in Win32
	BYTE nAllocLen = GetAt( 6 );
	if( nAllocLen != _FI_LEN )					// byte length for DOS style info.
	{
		RetCheckStatus( NgBadFieldInCDB );
		return;
	}

	if( m_aFiInfoBase.GetSize() == 0 )			// none in the list.
	{
		RetCheckStatus( NgImproperComSeq );
		return;
	}
	else										// else, check the index legal or not.
	{
		if( nIndex < 0 || nIndex >= m_aFiInfoBase.GetSize() )
		{
			RetCheckStatus( NgIncrctFiIndex );
			return;
		}
	}

	if( m_fiInfo.m_fiArchive.m_hFile != CFile::hFileNull )
	{
		m_fiInfo.m_fiArchive.Close();
		m_fiInfo.m_bFileInUse = FALSE;
	}

	m_fiInfo.m_sFileName = m_sDirName + "\\" + ((CFileInfo*)(m_aFiInfoBase[ nIndex ]))->m_sFileName;

	TRY
	{
		m_fiInfo.GetStatus();
	}
	CATCH( CFileException, theException )
	{
		TRACE( " ==> could not retrieve status, negate the file size.\n" );

		m_fiInfo.m_size = -1;
	}
	END_CATCH

	if( (m_fiInfo.m_size < 0) || m_fiInfo != *(CFileInfo*)(m_aFiInfoBase[ nIndex ]) )
	{
		TRACE( "Media Changed: TOC/attributes have been changed.\n" );

		RetCheckStatus( NgMediaChanged );
		return;
	}

	RetCheckStatus( OkStatus );

	FormatOutput( "Transferring file info. <%s> %s",
		(const char*)((CFileInfo*)(m_aFiInfoBase[ nIndex ]))->m_sFileName,
		CTime::GetCurrentTime().Format( "at %H:%M:%S" )
	);

	m_fiInfo.m_bFileInUse = TRUE;			// set indicator for proper sequence

	CBArchive bar( *this, CBArchive::store, nAllocLen );
	bar << &( m_fiInfo );

	(this->*m_pfnSendFromBuffer)( nAllocLen );

#ifdef _DEBUG
	m_fiInfo.Dump( afxDump );
#endif
}

void CDCServer::SendData()
{
	ASSERT_VALID( this );

	if( m_fiInfo.m_bFileInUse == FALSE )
	{
		RetCheckStatus( NgImproperComSeq );
		return;
	}

	BYTE bAttribute = GetAt( 1 );			// b7:CRC b6:CF
	DWORD dwStartAddress = GetDword( 2 );
	WORD wTransferLen = GetWord( 6 );

	if( wTransferLen == 0 )
	{
		RetCheckStatus( NgBadFieldInCDB );
		return;
	}

	RetCheckStatus( OkStatus );

	/****************************************************************
	static BYTE abSticks[4] = { '-', '\\', '|', '/' };
	static int nStickNo = 0;
	FormatOutput( "%c\r", abSticks[ nStickNo ] );
	nStickNo = (nStickNo + 1) & 0x3;
	****************************************************************/

	m_fiInfo.ReadFile( dwStartAddress, wTransferLen, *this );

	(this->*m_pfnSendFromBuffer)( wTransferLen );

	if( bAttribute & b6_CF )				// close file after data sent?
	{
		m_fiInfo.m_fiArchive.Close();
		m_fiInfo.m_bFileInUse = FALSE;
	}
}

void CDCServer::ChangeDir()
{
	ASSERT_VALID( this );

	int nIndex = (short) GetWord( 2 );			// int has 32 bits in Win32
	WORD nAllocLen = GetWord( 6 );
	if( nAllocLen != _FI_LEN + 256 )
	{
		RetCheckStatus( NgBadFieldInCDB );
		return;
	}

	if( m_aFiInfoBase.GetSize() )
	{
		// request the working directory if nIndex set to -1.
		if( nIndex < -1 || nIndex >= m_aFiInfoBase.GetSize() )
		{
			RetCheckStatus( NgIncrctFiIndex );
			return;
		}
	}

	if( m_fiInfo.m_fiArchive.m_hFile != CFile::hFileNull )
	{
		m_fiInfo.m_fiArchive.Close();
		m_fiInfo.m_bFileInUse = FALSE;
	}

	if( nIndex == -1 )		// request the working dir.
	{
		m_fiInfo.m_sFileName = "Default.Dir";
		goto _goSend;
	}

	m_fiInfo.m_sFileName = m_sDirName + "\\" + ((CFileInfo*)(m_aFiInfoBase[ nIndex ]))->m_sFileName;

	TRY
	{
		m_fiInfo.GetStatus();
	}
	CATCH( CFileException, theException )
	{
		TRACE( " ==> could not retrieve status, zero the attribute byte.\n" );

		m_fiInfo.m_attribute = 0;
	}
	END_CATCH

	// check to see if go up for mother-dir
	if( ((CFileInfo*)(m_aFiInfoBase[ nIndex ]))->m_sFileName == _PARENT_DIR )
	{
		int pos = m_sDirName.ReverseFind( '\\' );
		if( pos != -1 )
		{
			// Copy file information for equal comparison below.
			m_sDirName = m_sDirName.Left( pos );
			m_fiInfo = *(CFileInfo*)(m_aFiInfoBase[ nIndex ]);
		}
		else   /// HERE MUST BE AN IMPOSSIBLE SITUATION.
		{
			TRACE( "System Error: TOC might have been corrupted.\n" );

			RetCheckStatus( NgMediaChanged );
			return;
		}
	}

	if( (m_fiInfo.m_attribute & CFile::directory) && m_fiInfo == *(CFileInfo*)(m_aFiInfoBase[ nIndex ]) )
	{
		// not going up for mother-dir
		if( m_fiInfo.m_sFileName != _PARENT_DIR )
		{
			m_sDirName += "\\" + ((CFileInfo*)(m_aFiInfoBase[ nIndex ]))->m_sFileName;
		}
	}
	else
	{
		TRACE( "Media Changed: TOC/attributes have been changed.\n" );

		RetCheckStatus( NgMediaChanged );
		return;
	}

_goSend:
	RetCheckStatus( OkStatus );

	CBArchive bar( *this, CBArchive::store, _FI_LEN );
	bar << &( m_fiInfo );

	SetString( 20, m_sDirName );

	(this->*m_pfnSendFromBuffer)( nAllocLen );
}

void CDCServer::SendTOC()
{
	ASSERT_VALID( this );
	WORD nAllocLen = GetWord( 6 );

	// limit buffer size so that TRY/CATCH will handle file count.
	CBArchive bar( *this, CBArchive::store, nAllocLen );
	bar += 2;

	DeleteBase();
	m_aFiInfoBase.SetSize( 0, 256 );

	// find first all sub-directories, then find additional matches.
	TRY
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFile;

		// First, see if there's anything in the directory
		hFile = FindFirstFile( m_sDirName + "\\"_ALLFILES, &FindFileData );

		if( INVALID_HANDLE_VALUE == hFile )
		{
			TRACE0( "FindFirstFile returned INVALID_HANDLE_VALUE.\n" );
			AfxThrowFileException( CFileException::invalidFile, GetLastError() );
		}

		// Next, loop through everything in the directory. If the found file
		// is itself a directory, add it to the list.
		while( INVALID_HANDLE_VALUE != hFile )
		{
			if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( strcmp( FindFileData.cFileName, _SELF_DIR ) )
				{
					// Add this item to the list
					AddIntoTOC( bar, &FindFileData );
				}
			}

			if( !FindNextFile( hFile, &FindFileData ) )
				break;
		}
		FindClose( hFile );

		// First, see if there are matching files in the directory
		hFile = FindFirstFile( m_sDirName + "\\" + m_sFileName, &FindFileData );

		// Next, loop through everything in the directory. If the found file
		// is itself a matching file, add it to the list.
		DWORD dwAttr = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN |
					FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_COMPRESSED;
		while( INVALID_HANDLE_VALUE != hFile )
		{
			if( (FindFileData.dwFileAttributes & dwAttr) && !(FindFileData.dwFileAttributes & ~dwAttr) )
			{
				// Add this item to the list
				AddIntoTOC( bar, &FindFileData );
			}

			if( !FindNextFile( hFile, &FindFileData ) )
				break;
		}
		FindClose( hFile );
	}
	CATCH( CInfoException, theException )
	{
		TRACE( " ==> revise/fake bar pointer for checking.\n" );

		// update pointer for error check in destructor,
		//  and bar is no longer usable!
		bar -= _FI_LEN;
	}
	END_CATCH

	TRACE1( "Totally collected %d items for TOC.\n", m_aFiInfoBase.GetSize() );

	// check the collected file info. for TOC
	if( m_aFiInfoBase.GetSize() > (int)(nAllocLen / _FI_LEN) )
	{
/**********************************************************
		RetCheckStatus( NgBadFieldInCDB );
		return;
**********************************************************/
		TRACE( "==> Occurred Weird logic (should not be shown here)!\n" );
	}

	RetCheckStatus( OkStatus );

	FormatOutput( "Transferring directory %s", (const char*)m_sDirName );

	// limit the dir/file count to be passed to the Guest.
	int nCount = m_aFiInfoBase.GetSize();
	if (nCount > m_nMaxSizeBase)
	{
		nCount = m_nMaxSizeBase;
		FormatOutput( "... Cut transferred items to %d.", nCount );
	}
	SetAt( 0, (WORD) nCount );

	(this->*m_pfnSendFromBuffer)( 2 + nCount * _FI_LEN );

	// it should not be nothing got, we checked file counts in constrcutor.
	ASSERT( m_aFiInfoBase.GetSize() > 0 );
}

void CDCServer::AddIntoTOC( CBArchive& bar, WIN32_FIND_DATA* pFileData )
{
	CFileInfo* pFiInfo = new CFileInfo;
	ASSERT_VALID( pFiInfo );

	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	pFiInfo->m_attribute = (BYTE) (pFileData->dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

	// get just the low DWORD of the file size
	ASSERT( pFileData->nFileSizeHigh == 0 );
	pFiInfo->m_size = (LONG) pFileData->nFileSizeLow;

	// convert times as appropriate
	pFiInfo->m_mtime = CTime( pFileData->ftLastWriteTime );

	// cFileName member might contain a classic 8.3 filename format
	// if cAlternateFileName is empty.
	pFiInfo->m_sFileName = pFileData->cAlternateFileName;
	if( pFiInfo->m_sFileName.IsEmpty() )
	{
		pFiInfo->m_sFileName = pFileData->cFileName;
	}

	TRY
	{
		bar << pFiInfo;
	}
	CATCH( CInfoException, theException )
	{
		TRACE( " ==> got it, delete allocated space and then rethrow.\n" );

		delete pFiInfo;
		THROW_LAST();
	}
	END_CATCH
	m_aFiInfoBase.Add( pFiInfo );
}

void CDCServer::SwitchBusSpeed()
{
	ASSERT_VALID( this );

	BYTE nSpeedMode = GetAt( 1 );
	if( nSpeedMode != b7_8Bit && nSpeedMode != b6_Test && nSpeedMode != 0x0 )
	{
		RetCheckStatus( NgBadFieldInCDB );
		return;
	}

	RetCheckStatus( OkStatus );

#ifdef POLL_S7_FOR_BUSY
	if( nSpeedMode == b6_Test )
	{
		BYTE nAllocLen = GetAt( 6 );
		ASSERT( nAllocLen == sizeof( abTestData ) );
		if( nAllocLen > sizeof( abTestData ) || nAllocLen == 0 )  // 16?
		{
			RetCheckStatus( NgBadFieldInCDB );
			return;
		}
		FillBuffer( 0, abTestData, nAllocLen );
		SendFromBufferInByte( nAllocLen );
	}
	else if( nSpeedMode == b7_8Bit )
	{
		m_pfnSendFromBuffer = SendFromBufferInByte;
		m_pfnReceiveIntoBuffer = ReceiveIntoBufferInByte;
	}
	else  // 0x0 for 4-bit
	{
		m_pfnSendFromBuffer = SendFromBuffer;
		m_pfnReceiveIntoBuffer = ReceiveIntoBuffer;
	}
#endif
}

void CDCServer::FormatOutput( LPCTSTR lpszFormat, ... )
{
	CString* ps = new CString;
	ASSERT( AfxIsValidString( lpszFormat ) );
	va_list argList;
	va_start( argList, lpszFormat );
	ps->FormatV( lpszFormat, argList );
	va_end( argList );

	::PostMessage( m_hWndOwner, UWM_ADD_STRING, 0, (LPARAM) ps );
}

UINT CDCServer::DoWork()
{
	InvalidateOpcode();				// Invalidate Opcode

	do
	{
		if( m_rNibbleModeDev.WatchForIncoming() == FALSE )
		{
			// A value of zero causes the thread to relinquish the remainder of
			// its time slice to any other thread of equal priority that is ready
			// to run. If there are no other threads of equal priority ready to
			// run, the function returns immediately.
			::Sleep( 0 );
			continue;
		}
		// Set this thread's priority as high as reasonably possible to
		// prevent timeslice interrupts.
		::SetThreadPriority( m_pThread, THREAD_PRIORITY_TIME_CRITICAL );

		ReceiveCommand();

#ifdef _DEBUG
		Dump( afxDump );
#endif
		switch( GetOpcode() )
		{
		case EndDCC:
			RetCheckStatus( OkStatus );
			break;
		case RequestFileInfo:
			SendFileInfo();
			break;
		case TransferData:
			SendData();
			break;
		case RequestChgDir:
			ChangeDir();
			break;
		case ReadTOC:
			SendTOC();
			break;
		case ChangeBandwidth:
			SwitchBusSpeed();
			break;
		default:
			RetCheckStatus( NgIllegalCommand );
			break;
		}

		// Reset this thread's priority back to normal.
		::SetThreadPriority( m_pThread, THREAD_PRIORITY_NORMAL );
	}
	while( m_bRunning && (GetOpcode() != EndDCC) );

	return 0;
}

//////////////////
// This converts the Windows/C-style thread procedure into an MFC/C++-style
// virtual function. To do the "work" of the thread, implement DoWork and
// don't worry about the thread proc.
//
UINT CDCServer::ThreadProc( LPVOID pObj )
{
	CDCServer* pJob = (CDCServer*)pObj;
	ASSERT_KINDOF( CDCServer, pJob );
	TRY
	{
		pJob->m_uErr = pJob->DoWork();		// call virt fn to do the work
	}
	CATCH( CExcptClass, theException )
	{
		theException->Handler();

		::PostMessage( pJob->m_hWndOwner, UWM_EXCEPT_BOX, 0, 0L );
	}
	AND_CATCH( CFileException, theException )
	{
		pJob->FormatOutput( "Error: caught a file exception <%d>.", theException->m_cause );
	}
	AND_CATCH( CException, theException )
	{
		CString sTemp;
		sTemp.Format( "Unexpected exception:> %s\r\n", theException->GetRuntimeClass()->m_lpszClassName );
		GetMyMainFrame()->m_ExceptDlg.AddStringToEdit( sTemp );

		::PostMessage( pJob->m_hWndOwner, UWM_EXCEPT_BOX, 0, 0L );
	}
	END_CATCH

	pJob->m_pThread = NULL;					// done: clear
	pJob->m_bRunning = FALSE;

	::PostMessage( pJob->m_hWndOwner, UWM_SERVER_END, 0, 0L );
	return pJob->m_uErr;					// ..and return error code to Windows
}

//////////////////
// Begin running the worker thread. Args are owner window and callback
// message ID to use for OnProgress notifications, if any. You could enhance
// this to expose pritority and other AfxBeginThread args.
//
BOOL CDCServer::Begin( CWnd* pWndOwner /* = NULL */, UINT ucbMsg /* = 0 */)
{
	m_hWndOwner = pWndOwner->GetSafeHwnd();	// NULL if used with a null CWnd pointer
	m_uErr = 0;
	m_bRunning = TRUE;
	m_pThread = AfxBeginThread( ThreadProc, this );
	ASSERT( m_pThread );
	return m_pThread != NULL;
}

//////////////////
// Abort the thread. All this does is set m_bAbort = TRUE.
// It's up to you to check m_bAbort periodically in your DoWork function.
// You can override to use CEvent if you need to.
//
void CDCServer::Kill()
{
	m_bRunning = FALSE;
}

#ifdef _DEBUG
void CDCServer::AssertValid() const
{
	CDirectCable::AssertValid();
}

void CDCServer::Dump( CDumpContext& dc ) const
{
	ASSERT_VALID( this );
	CString sTemp = "a Server CDB:";
	for( int i = 0; i < sizeof( m_bCDB ); i ++ )
	{	
		CString s;
		s.Format( " %2x", m_bCDB[i] );
		sTemp += s;
	}
	sTemp += "\n";
	dc << sTemp;
}
#endif
