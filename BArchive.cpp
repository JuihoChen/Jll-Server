// BArchive.cpp: implementation of the CBArchive class.
//
//////////////////////////////////////////////////////////////////////

#include <tchar.h>
#include "stdafx.h"
#include "Jll Server.h"
#include "BArchive.h"
#include "direct.h"

#ifdef _DEBUG
#undef THIS_FILE
static const char * THIS_FILE = __FILE__;
#define new DEBUG_NEW
#endif

// Because the difference of mktime() between VC6 & MSC8, we don't use CTime
// to transfer on communication. DOS style date & time (4 bytes) are used instead.
const CTimeDos& CTimeDos::operator=( const CTime& timeSrc )
{
	ASSERT_VALID( this );

	// There's a CTime difference between VC++ 6.0 & MSC8
	SYSTEMTIME sys;
	timeSrc.GetAsSystemTime( sys );
	FILETIME local;
	SystemTimeToFileTime( &sys, &local );
	FileTimeToDosDateTime( &local, &m_wDate, &m_wTime );
	return *this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBArchive::CBArchive( CDirectCable& rDcc, UINT nMode, int nBufSize /* = BF_MAXLEN / 2 */)
{
	ASSERT_VALID( &rDcc );
	m_nMode = nMode;
	m_fpBufStart = m_fpBufCur = rDcc.m_fpBuffer;
	m_fpBufMax = m_fpBufStart + nBufSize;
	m_fpOrig = 0;
	ASSERT( AfxIsValidAddress( m_fpBufStart, nBufSize ) );
}

void CBArchive::LoadFileName( CString& rFullName )
{
	ASSERT_VALID( this );
	ASSERT( IsLoading() );

	char szBuffer[ 15 ] = {0}, * psz = szBuffer;
	for( int i = 0; i < 8; i ++ )
	{
		if( m_fpBufCur[ i ] == '\0' || isspace( m_fpBufCur[ i ] ) )
			break;
		*psz++ = m_fpBufCur[ i ];
	}
	if( m_fpBufCur[ 8 ] != '\0' && !isspace( m_fpBufCur[ 8 ] ) )
	{
		*psz++ = '.';
		for( int i = 8; i < 11; i ++ )
		{
			if( m_fpBufCur[ i ] == '\0' || isspace( m_fpBufCur[ i ] ) )
				break;
			*psz++ = m_fpBufCur[ i ];
		}
	}
	*psz = '\0';

	rFullName = szBuffer;
	m_fpBufCur += 11;
}

void CBArchive::StoreFileName( const CString& rFullName )
{
	/*!*/// Buffer has to be checked here to throw for release version!
	if( m_fpBufCur >= m_fpBufMax )
	{
		RestorePosition();
		TRACE( "Error: buffer archive overflows.\n" );
		THROW( new CInfoException( CInfoException::BArchiveOverflow ) );
	}

	ASSERT_VALID( this );
	ASSERT( IsStoring() );

	CString sExt, sFileName = rFullName;
	int pos = sFileName.ReverseFind( '\\' );	// get rid of path name.
	if( pos != -1 )
		sFileName = sFileName.Mid( pos + 1 );

	ASSERT( sFileName.IsEmpty() == FALSE );

	// skip this very dir. & skip the mother dir.
	if( (sFileName != _SELF_DIR) && (sFileName != _PARENT_DIR) )
	{
		pos = sFileName.Find( '.' );
		if( pos != -1 )
		{
			sExt = sFileName.Mid( pos + 1 );
			sFileName = sFileName.Left( pos );
		}
	}

	//*v0.25*
	// We have to check for an exception (or a bug) for this file:
	//   "C:\Program Files\Common Files\Symantec Shared\EENGINE\EraserUtilDrvI1.sys"
	if( sFileName.GetLength() > 8 || sExt.GetLength() > 3 )
	{
		RestorePosition();
		::OutputDebugString( L"CInfoException::BadShortPathName -- " );
		::OutputDebugString( (LPCTSTR)(sFileName + "." + sExt) );
		TRACE( "Error: erroneous \"shortpathname\" occurred.\n" );
		THROW( new CInfoException( CInfoException::BadShortPathName ) );
	}

	memset( m_fpBufCur, 0, 11 );
	memcpy( m_fpBufCur, sFileName, sFileName.GetLength() );
	memcpy( m_fpBufCur + 8, sExt, sExt.GetLength() );

	m_fpBufCur += 11;
}

void CBArchive::WriteBObject( const CBObject* pBOb )
{
	ASSERT_VALID( this );
	ASSERT_VALID( pBOb );
	ASSERT( IsStoring() );

	RecordPosition();
	((CBObject*) pBOb)->Serialize( *this );
}

#ifdef _DEBUG
void CBArchive::AssertValid() const
{
	CObject::AssertValid();
	ASSERT( m_fpBufStart != NULL );
	ASSERT( m_fpBufCur != NULL );
	ASSERT( m_fpBufCur <= m_fpBufMax );
}
#endif

// definition for static member.
CFile CFileInfo::m_fiArchive;
BOOL CFileInfo::m_bFileInUse = FALSE;

void CFileInfo::GetStatus( BOOL fTryNetpath /* = FALSE */)
// a GetStatus member is defined for reasons:
// a. use static version of CFile::GetStatus to get attribute right.
// b. size of CFileStatus is big.
// c. to isolate the data member in CFileInfo.
{
	ASSERT_VALID( this );
	ASSERT( m_sFileName.IsEmpty() == FALSE );

	// some old MS-DOS files incur system assertion!
	// and we have to make operation on.... (v0.16)
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile( m_sFileName, &findFileData );

	// pathname on network need wildcards to find!
	if( hFind == INVALID_HANDLE_VALUE && fTryNetpath )
	{
		hFind = FindFirstFile( CDirectCable::ConcatDir( m_sFileName, _ALLFILES ), &findFileData );
	}
	if( hFind == INVALID_HANDLE_VALUE )
	{
		TRACE2( "FindFirstFile (%s) not successful - GetLastError = %d\n", (LPCTSTR) m_sFileName, GetLastError() );
		THROW( new CFileException( CFileException::fileNotFound ) );
	}
	VERIFY( FindClose( hFind ) );

	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	m_attribute = (BYTE) (findFileData.dwFileAttributes & ~FA_FATXX_DOESNT_SUPPORT_BITS);

	// get just the low DWORD of the file size
	ASSERT( findFileData.nFileSizeHigh == 0 );
	m_size = (LONG) findFileData.nFileSizeLow;

	// convert times as appropriate
	FILETIME localTime;
	WORD date, time;
	if( !FileTimeToLocalFileTime( &findFileData.ftLastWriteTime, &localTime ) )
	{
		TRACE1( "FileTimeToLocalFileTime not successful - GetLastError = %d\n", GetLastError() );
		date = time = 0;
	}
	else
		FileTimeToDosDateTime( &localTime, &date, &time );

	m_mtime = CTimeDos( date, time );
}

void CFileInfo::ReadFile( DWORD dwStart, UINT nLen, CDirectCable& dcc )
{
	ASSERT_VALID( this );
	ASSERT_VALID( &dcc );
	ASSERT( m_sFileName.IsEmpty() == FALSE );

	if( m_fiArchive.m_hFile == CFile::hFileNull )
	{
		CFileException* e = new CFileException;
		if( m_fiArchive.Open( m_sFileName, CFile::modeRead, e ) == 0 )
			THROW( e );
		// use: pException->Delete(), do not use: delete pException
		//delete e;
		e->Delete();
	}
	m_fiArchive.Seek( dwStart, CFile::begin );
	m_fiArchive.Read( dcc.m_fpBuffer, nLen );
}

void CFileInfo::Serialize( CBArchive& bar )
{
	ASSERT_VALID( this );
	ASSERT_VALID( &bar );

	if( bar.IsStoring() )
	{
		TRACE( "Serializing a CFileInfo out.\n" );

		bar << m_size;
		bar.StoreFileName( m_sFileName );
		bar << m_attribute
			<< m_mtime;
	}
	else
	{
		TRACE( "Serializing a CFileInfo in.\n" );

		bar >> m_size;
		bar.LoadFileName( m_sFileName );
		bar >> m_attribute
			>> m_mtime;
	}

#ifdef _DEBUG
	Dump( afxDump );
#endif
}

BOOL CFileInfo::IsEqual( const CFileInfo& other ) const
{
	ASSERT_VALID( this );
	ASSERT_VALID( &other );

	// pos = -1 if '\\' not found in string.
	int pos = m_sFileName.ReverseFind( '\\' );

	if( m_sFileName.Mid( pos + 1 ).CompareNoCase( other.m_sFileName ) ) return FALSE;
	if( m_size != other.m_size ) return FALSE;
	if( m_attribute != other.m_attribute ) return FALSE;
	if( m_mtime != other.m_mtime ) return FALSE;
	return TRUE;
}

CFileInfo& CFileInfo::operator=( const CFileInfo& other )
{
	ASSERT_VALID( this );
	ASSERT_VALID( &other );

	if( &other != this )		// check for self-assignment
	{
		m_sFileName = other.m_sFileName;
		m_size = other.m_size;
		m_attribute = other.m_attribute;
		m_mtime = other.m_mtime;
	}
	return *this;
}

#ifdef _DEBUG
void CFileInfo::AssertValid() const
{
	CObject::AssertValid();
}

void CFileInfo::Dump( CDumpContext& dc ) const
{
	ASSERT_VALID( this );
	CString sTemp1, sTemp2;

	const time_t mtime = static_cast<time_t>(*(const long*)&m_mtime);

	char szTimeBuffer[26]; // Buffer for ctime_s, includes null terminator
	errno_t err = ctime_s(szTimeBuffer, sizeof(szTimeBuffer), &mtime);

	if (err != 0 || mtime == 0) // Check for error from ctime_s or invalid time
		sTemp1.Format( L"CTime(invalid #%I64d)", mtime );

	else
	{
		// ctime_s produces a newline at the end. Nuke it.
		char* newline_pos = strchr(szTimeBuffer, '\n');
		if (newline_pos != NULL)
		{
			*newline_pos = '\0';
		}
		sTemp1.Format( L"CTime(\"%s\")", (LPCTSTR) szTimeBuffer);
	}

	sTemp2.Format( L"FileInfo: <filename> %s <filesize> %d\n"
					"          <attribute> 0x%0x <mtime> %s <%I64d>\n",
				   (LPCTSTR) m_sFileName, m_size, m_attribute,
				   (LPCTSTR) sTemp1, mtime );
	dc << sTemp2;
}
#endif
