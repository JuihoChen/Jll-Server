// BArchive.h: interface for the CBArchive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BARCHIVE_H__EFFC4E05_F289_46E3_BB32_264CBEFCD689__INCLUDED_)
#define AFX_BARCHIVE_H__EFFC4E05_F289_46E3_BB32_264CBEFCD689__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// We limit the buffer index to an 'unsigned int'
#define BF_MAXLEN	(0xfff0 - 1)

class CBObject;
class CDCGuest;
class CDCServer;
class CDirectCable;

class CTimeDos : public CObject
{
public:
// Constructors
	CTimeDos();
	CTimeDos( const CTimeDos& timeSrc );
	CTimeDos( WORD date, WORD time );
// Attributes
	LONG GetTime() const;
// Operations
	const CTimeDos& operator=( const CTime& timeSrc );
	const CTimeDos& operator=( const CTimeDos& timeSrc );
	const CTimeDos& operator=( DWORD mtime );
	BOOL operator!=( const CTimeDos other ) const;
private:
	union {
		DWORD m_time;
		struct {
			WORD m_wTime;
			WORD m_wDate;
		};
	};
};

class CBArchive : public CObject  
{
public:
	enum Mode { store = 0, load = 1 };

	CBArchive( CDirectCable& rDcc, UINT nMode, int nBufSize = BF_MAXLEN / 2 );
    virtual ~CBArchive() { ASSERT_VALID( this ); }

	BOOL IsLoading() const;
	BOOL IsStoring() const;

	void LoadFileName( CString& rFullName );
	void StoreFileName( const CString& rFullName );

public:
	// Insertion operations for Buffer Archive objects
	CBArchive& operator<<( const CBObject* pBOb );
	CBArchive& operator>>( CBObject* pBOb );

	// Insertion operations
	CBArchive& operator<<( BYTE by );
	CBArchive& operator<<( LONG l );
	CBArchive& operator<<( CTimeDos t );

	// Extraction operations
	CBArchive& operator>>( BYTE& by );
	CBArchive& operator>>( LONG& l );
	CBArchive& operator>>( CTimeDos& t );

	// pointer movement operations
	CBArchive& operator+=( const int nOffset );
	CBArchive& operator-=( const int nOffset );

protected:
	void ReadBObject( CBObject* pBOb );
	void WriteBObject( const CBObject* pBOb );

private:
	UINT m_nMode;
	BYTE* m_fpBufCur;
	BYTE* m_fpBufMax;
	BYTE* m_fpBufStart;

#ifdef _DEBUG
public:
    virtual void AssertValid() const;
#endif
};

class CBObject : public CObject
{
public:
	virtual void Serialize( CBArchive& bar ) = 0;
};

class CFileInfo : public CBObject
{
	friend CDCGuest; friend CDCServer;

public:
	CFileInfo() { m_size = 0; }
	BOOL IsEqual( const CFileInfo& other ) const;
	BOOL operator==( const CFileInfo& other ) const;
	BOOL operator!=( const CFileInfo& other ) const;
	CFileInfo& operator=( const CFileInfo& other );
protected:
	void GetStatus( BOOL fTryNetpath = FALSE );
	///void SetStatus();
	///void WriteFile( DWORD dwStart, UINT nLen, CDirectCable& dcc );
	void ReadFile( DWORD dwStart, UINT nLen, CDirectCable& dcc );
private:
	CTimeDos m_mtime;				// data & time the file modified
	LONG m_size;					// file size in bytes
	BYTE m_attribute;				// MS-DOS attribute byte
	CString m_sFileName;			// string of file name
	static CFile m_fiArchive;		// temp. file object for operation
	static BOOL m_bFileInUse;		// indicator of file in transfer
	static BOOL m_bUseCRC;			// indicator of CRC checking
public:
	// Override the Serialize function
	virtual void Serialize( CBArchive& bar );
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif
};


inline CTimeDos::CTimeDos()
	{ }
inline CTimeDos::CTimeDos( const CTimeDos& timeSrc )
	{ m_time = timeSrc.m_time; }
inline CTimeDos::CTimeDos( WORD date, WORD time )
	{ m_wTime = time; m_wDate = date; }
inline const CTimeDos& CTimeDos::operator=( const CTimeDos& timeSrc )
	{ m_time = timeSrc.m_time; return *this; }
inline const CTimeDos& CTimeDos::operator=( DWORD mtime )
	{ m_time = mtime; return *this; }
inline BOOL CTimeDos::operator!=( const CTimeDos other ) const
	{ return m_time != other.m_time; }
inline LONG CTimeDos::GetTime() const
	{ return m_time; }

inline BOOL CBArchive::IsLoading() const
	{ return (m_nMode & CBArchive::load) != 0; }
inline BOOL CBArchive::IsStoring() const
	{ return (m_nMode & CBArchive::load) == 0; }
inline CBArchive& CBArchive::operator+=( const int nOffset )
	{ m_fpBufCur += nOffset; return *this; }
inline CBArchive& CBArchive::operator-=( const int nOffset )
	{ m_fpBufCur -= nOffset; return *this; }
inline CBArchive& CBArchive::operator<<( BYTE by )
	{ *(BYTE*)m_fpBufCur = by; m_fpBufCur += sizeof(BYTE); return *this; }
inline CBArchive& CBArchive::operator<<( LONG l )
	{ *(LONG*)m_fpBufCur = l; m_fpBufCur += sizeof(LONG); return *this; }
inline CBArchive& CBArchive::operator<<( CTimeDos t )
	{ return *this << t.GetTime(); }
inline CBArchive& CBArchive::operator>>( BYTE& by )
	{ by = *(BYTE*)m_fpBufCur; m_fpBufCur += sizeof(BYTE); return *this; }
inline CBArchive& CBArchive::operator>>( LONG& l )
	{ l = *(LONG*)m_fpBufCur; m_fpBufCur += sizeof(LONG); return *this; }
inline CBArchive& CBArchive::operator>>( CTimeDos& t )
	{ t = *(LONG*)m_fpBufCur; m_fpBufCur += sizeof(LONG); return *this; }
inline CBArchive& CBArchive::operator<<( const CBObject* pBOb )
	{ WriteBObject( pBOb ); return *this; }
inline CBArchive& CBArchive::operator>>( CBObject* pBOb )
	{ ReadBObject( pBOb ); return *this; }

inline void CBObject::Serialize( CBArchive& bar )
	{ /* CBObject does not serialize anything by default */ }

inline BOOL CFileInfo::operator==( const CFileInfo& other ) const
	{ return IsEqual( other ); }
inline BOOL CFileInfo::operator!=( const CFileInfo& other ) const
	{ return !IsEqual( other ); }

#endif // !defined(AFX_BARCHIVE_H__EFFC4E05_F289_46E3_BB32_264CBEFCD689__INCLUDED_)
