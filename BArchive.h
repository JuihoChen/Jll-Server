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
	CBArchive& operator<<( CTime t );

	// Extraction operations
	CBArchive& operator>>( BYTE& by );
	CBArchive& operator>>( LONG& l );
	CBArchive& operator>>( CTime& t );

	// pointer movement operations
	CBArchive& operator+=( const int nOffset );
	CBArchive& operator-=( const int nOffset );

protected:
	void ReadBObject( CBObject* pBOb );
	void WriteBObject( const CBObject* pBOb );

private:
	UINT m_nMode;
	BYTE FAR* m_fpBufCur;
	BYTE FAR* m_fpBufMax;
	BYTE FAR* m_fpBufStart;

#ifdef _DEBUG
public:
    virtual void AssertValid() const;
#endif
};

class CTimeDos : public CTime
{
public:
	CTimeDos& operator=( CTime& other );
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
	void GetStatus();
	void SetStatus();
	void WriteFile( DWORD dwStart, UINT nLen, CDirectCable& dcc );
	void ReadFile( DWORD dwStart, UINT nLen, CDirectCable& dcc );
private:
	CTimeDos m_mtime;				// data & time the file modified
	LONG m_size;					// file size in bytes
	BYTE m_attribute;				// MS-DOS attribute byte
	CString m_sFileName;			// string of file name
	static CFile m_fiArchive;		// temp. file object for operation
	static BOOL m_bFileInUse;		// indicator of file in transfer
public:
	// Override the Serialize function
	virtual void Serialize( CBArchive& bar );
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif
};


inline BOOL CBArchive::IsLoading() const
	{ return (m_nMode & CBArchive::load) != 0; }
inline BOOL CBArchive::IsStoring() const
	{ return (m_nMode & CBArchive::load) == 0; }
inline CBArchive& CBArchive::operator+=( const int nOffset )
	{ m_fpBufCur += nOffset; return *this; }
inline CBArchive& CBArchive::operator-=( const int nOffset )
	{ m_fpBufCur -= nOffset; return *this; }
inline CBArchive& CBArchive::operator<<( BYTE by )
	{ *(BYTE FAR*)m_fpBufCur = by; m_fpBufCur += sizeof(BYTE); return *this; }
inline CBArchive& CBArchive::operator<<( LONG l )
	{ *(LONG FAR*)m_fpBufCur = l; m_fpBufCur += sizeof(LONG); return *this; }
inline CBArchive& CBArchive::operator<<( CTime t )
	{ return *this << t.GetTime(); }
inline CBArchive& CBArchive::operator>>( BYTE& by )
	{ by = *(BYTE FAR*)m_fpBufCur; m_fpBufCur += sizeof(BYTE); return *this; }
inline CBArchive& CBArchive::operator>>( LONG& l )
	{ l = *(LONG FAR*)m_fpBufCur; m_fpBufCur += sizeof(LONG); return *this; }
inline CBArchive& CBArchive::operator>>( CTime& t )
	{ t = *(time_t FAR*)m_fpBufCur; m_fpBufCur += sizeof(time_t); return *this; }
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
