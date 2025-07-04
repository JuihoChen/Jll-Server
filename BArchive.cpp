// BArchive.cpp: implementation of the CBArchive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jll Server.h"
#include "BArchive.h"
#include "direct.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
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
	ASSERT( AfxIsValidAddress( m_fpBufStart, nBufSize ) );
}

void CBArchive::LoadFileName( CString& rFullName )
{
	ASSERT_VALID( this );
	ASSERT( IsLoading() );

	char szBuffer[ 15 ], *psz = szBuffer;
	for( int i = 0; i < 8; i ++ )
	{
		if( m_fpBufCur[ i ] == '\0' || isspace( m_fpBufCur[ i ] ) )
			break;
		*psz++ = m_fpBufCur[ i ];
	}
	if( m_fpBufCur[ 8 ] != '\0' && !isspace( m_fpBufCur[ 8 ] ) )
	{
		*psz++ = '.';
		for( i = 8; i < 11; i ++ )
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

void CFileInfo::GetStatus()
// a GetStatus member is defined for reasons:
// a. use static version of CFile::GetStatus to get attribute right.
// b. size of CFileStatus is big.
// c. to isolate the data member in CFileInfo.
{
	ASSERT_VALID( this );
	ASSERT( m_sFileName.IsEmpty() == FALSE );

	// *!* Use static version of GetStatus to get attribute right *!*
	CFileStatus fsStat;
	if( CFile::GetStatus( m_sFileName, fsStat ) == FALSE )
	{
		THROW( new CFileException( CFileException::fileNotFound ) );
	}

	m_mtime = fsStat.m_mtime;
	m_size = fsStat.m_size;
	m_attribute = fsStat.m_attribute;
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
}

BOOL CFileInfo::IsEqual( const CFileInfo& other ) const
{
	ASSERT_VALID( this );
	ASSERT_VALID( &other );

	// pos = -1 if '\\' not found in string.
	int pos = m_sFileName.ReverseFind( '\\' );

	if( m_sFileName.Mid( pos + 1 ) != other.m_sFileName ) return FALSE;
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
	const long mtime = *(const long*)&m_mtime;
	char* psz = ctime( &mtime );

	if( (psz == NULL) || (mtime == 0) )
		sTemp1.Format( "CTime(invalid #%ld)", mtime );
	else
	{
		// format it
		psz[24] = '\0';         // nuke newline
		sTemp1.Format( "CTime(\"%s\")", psz );
	}

	sTemp2.Format( "FileInfo: <filename> %s <filesize> %d\n"
				   "          <attribute> %d <mtime> %s <%d>\n",
				   (const char*) m_sFileName, m_size, m_attribute,
				   (const char*)sTemp1, mtime );
	dc << sTemp2;
}
#endif
