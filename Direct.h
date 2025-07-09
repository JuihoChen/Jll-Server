// Direct.h: interface for the CDirectCable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECT_H__87D71C20_71C8_4D12_9AC7_9DC50ABBBF65__INCLUDED_)
#define AFX_DIRECT_H__87D71C20_71C8_4D12_9AC7_9DC50ABBBF65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "except.h"
#include "parallel.h"
#include "barchive.h"

#define  _ALLFILES		L"*.*"
#define  _PARENT_DIR	L".."
#define  _SELF_DIR		L"."

// *v0.24*  NTFS supports some file attributes that FATxx doesn't maintain. So these bits
// have to be included to open files on disc. In the other hand, these bits have to be
// excluded before the file attribute is to be transferred to the guest or to be compared.
#define  FA_FATXX_DOESNT_SUPPORT_BITS	(FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)

typedef	CTypedPtrArray<CObArray, CFileInfo*> tmplTOCARRAY;

class CBArchive;
class CFileInfo;
class SelectWin;

class CDirectCable : public CObject  
{
	friend CBArchive;
	friend CFileInfo;

	typedef void (CDirectCable::*ptrSendFunc)( SIZE_T nLen ) const;
	typedef void (CDirectCable::*ptrRecvFunc)( SIZE_T nIndex, SIZE_T nLen );

	DECLARE_DYNAMIC( CDirectCable )
public:
	enum EOpCode {				// list of commands in CDB
		EndDCC			= 0x0,
		RequestFileInfo	= 0x13,
		RequestChgDir	= 0x15,
		TransferData	= 0x29,
		ReadTOC			= 0x43,
		RequestSpecific	= 0x67,
		ChangeBandwidth	= 0xef,
		IllegalOpcode	= 0xff
	};
	enum EStatus {				// maximum 16 members allowed (in a nibble)
		OkStatus = 0,
		NgIllegalCommand,
		NgBadFieldInCDB,
		NgImproperComSeq,
		NgIncrctFiIndex,
		NgMediaChanged,
		NgBadCRCInComm,
		NgFileException
	};
	enum {
		b7_Lock = 0x80,
		b6_SubDir = 0x40,

		b7_NCRC = 0x80,
		b6_CF   = 0x40,

		b7_8Bit = 0x80,
		b6_Test = 0x40
	};
	CDirectCable( CNibbleModeProto& lpt );
	virtual ~CDirectCable();
protected:
	void SendFromBuffer( SIZE_T nLen ) const;
	void ReceiveIntoBuffer( SIZE_T nIndex, SIZE_T nLen );
	void SendFromBufferInByte( SIZE_T nLen ) const;
	void ReceiveIntoBufferInByte( SIZE_T nIndex, SIZE_T nLen );
	void FillBuffer( UINT nIndex, BYTE* src, UINT nLen );
	WORD GetWord( UINT nIndex ) const;
	CString GetString( int nIndex, int nMaxLen ) const;
	void SetAt( UINT nIndex, WORD wElement );
	void SetString( UINT nIndex, CString& rString );
	UINT Get_CRC_CheckSum( ULONG ulSize ) const;
	static void DeleteBase( tmplTOCARRAY* pTocAr );
	static CString ConcatDir( const CString& sDir, LPCTSTR pzName ); 
protected:
	CNibbleModeProto& m_rNibbleModeDev;
	ptrSendFunc m_pfnSendFromBuffer;
	ptrRecvFunc m_pfnReceiveIntoBuffer;
	CTimer m_tmrWaitS6;
	CFileInfo m_fiInfo;
	tmplTOCARRAY m_aFiInfoBase;
	static const int m_nMaxSizeBase;
	static BYTE abTestData[16];
private:
	BYTE* m_fpBuffer;
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};

class CDCServer : public CDirectCable
{
	DECLARE_DYNAMIC( CDCServer )
private:
	static UINT ThreadProc( LPVOID pObj );
	CWinThread* volatile m_pThread;		// running thread, if any
protected:
	volatile BOOL m_bRunning;			// whether to abort: DoWork must check this
	HANDLE m_hEventServerThreadKilled;	// event for thread been killed
	HWND m_hWndOwner;					// HWND, *not* CWnd* of owner window
public:
	CDCServer( CNibbleModeProto& lpt );
	~CDCServer();
	void ParseWorkDir( CString sFolderName );
	CString& GetWorkDir();
	virtual BOOL Begin( CWnd* pWndOwner = NULL );
	virtual void KillThread();
	BOOL IsRunning() const;
protected:
	virtual UINT DoWork();
	void SendFileInfo();
	void SendData();
	void ChangeDir();
	void SendTOC();
	void SendSpecific();
	void SwitchBusSpeed();
	EOpCode GetOpcode() const;
	void InvalidateOpcode();
	void ReceiveCommand();
	void RetCheckStatus( int nStatus );
	void CreateTOC( WORD nAlloc, BOOL fResp = TRUE );
	void AddIntoTOC( CBArchive& bar, WIN32_FIND_DATA* pFileData );
	void FormatOutput( LPCTSTR lpszFormat, ... ) const;
protected:
	BYTE GetAt( int nIndex ) const;
	WORD GetWord( int nIndex ) const;
	DWORD GetDword( int nIndex ) const;
private:
	BYTE m_bCDB[8];
	CString m_sDirName;
	CString m_sFileName;		// Wildcards incl.

	CString m_sTocDir;
	tmplTOCARRAY* m_pTocAr;

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif
};


inline WORD CDirectCable::GetWord( UINT nIndex ) const
	{ ASSERT( nIndex >= 0 && nIndex <= BF_MAXLEN - 1 ); return *(WORD*)(m_fpBuffer + nIndex); }
inline void CDirectCable::SetAt( UINT nIndex, WORD wElement )
	{ ASSERT( nIndex >= 0 && nIndex <= BF_MAXLEN - 1 ); *(WORD*)(m_fpBuffer + nIndex) = wElement; }

inline CString& CDCServer::GetWorkDir()
	{ return m_sDirName; }
inline BOOL CDCServer::IsRunning() const
	{ ASSERT_VALID( this ); return m_pThread && m_bRunning; }
inline CDirectCable::EOpCode CDCServer::GetOpcode() const
	{ return (EOpCode) GetAt( 0 ); }
inline void CDCServer::InvalidateOpcode()
	{ m_bCDB[ 0 ] = IllegalOpcode; }
inline BYTE CDCServer::GetAt( int nIndex ) const
	{ ASSERT( nIndex >= 0 && nIndex < sizeof( m_bCDB ) ); return m_bCDB[ nIndex ]; }
inline WORD CDCServer::GetWord( int nIndex ) const
	{ ASSERT( nIndex >= 0 && nIndex < sizeof( m_bCDB ) - 1 ); return *(WORD*)(m_bCDB + nIndex); }
inline DWORD CDCServer::GetDword( int nIndex ) const
	{ ASSERT( nIndex >= 0 && nIndex < sizeof( m_bCDB ) - 3 ); return *(DWORD*)(m_bCDB + nIndex); }

#endif // !defined(AFX_DIRECT_H__87D71C20_71C8_4D12_9AC7_9DC50ABBBF65__INCLUDED_)
