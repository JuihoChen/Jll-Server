// Direct.h: interface for the CDirectCable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECT_H__87D71C20_71C8_4D12_9AC7_9DC50ABBBF65__INCLUDED_)
#define AFX_DIRECT_H__87D71C20_71C8_4D12_9AC7_9DC50ABBBF65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "except.h"
#include "parallel.h"
#include "barchive.h"

#define  _ALLFILES		"*.*"
#define  _PARENT_DIR	".."
#define  _SELF_DIR		"."

class CBArchive;
class CFileInfo;
class SelectWin;

class CDirectCable : public CObject  
{
	friend CBArchive;
	friend CFileInfo;

	typedef void (CDirectCable::*ptrSendFunc)( UINT nLen ) const;
	typedef void (CDirectCable::*ptrRecvFunc)( UINT nIndex, UINT nLen );

	DECLARE_DYNAMIC( CDirectCable )
public:
	enum EOpCode {				// list of commands in CDB
		EndDCC			= 0x0,
		RequestFileInfo	= 0x13,
		RequestChgDir	= 0x15,
		TransferData	= 0x29,
		ReadTOC			= 0x43,
		ChangeBandwidth	= 0xef,
		IllegalOpcode	= 0xff
	};
	enum EStatus {				// maximum 16 members allowed (in a nibble)
		OkStatus = 0,
		NgIllegalCommand,
		NgBadFieldInCDB,
		NgImproperComSeq,
		NgIncrctFiIndex,
		NgMediaChanged
	};
	enum {
		b7_Lock = 0x80,

		b7_NCRC = 0x80,
		b6_CF   = 0x40,

		b7_8Bit = 0x80,
		b6_Test = 0x40
	};
	CDirectCable( CNibbleModeProto& lpt );
	virtual ~CDirectCable();
protected:
	void SendFromBuffer( UINT nLen ) const;
	void ReceiveIntoBuffer( UINT nIndex, UINT nLen );
	void SendFromBufferInByte( UINT nLen ) const;
	void ReceiveIntoBufferInByte( UINT nIndex, UINT nLen );
	void FillBuffer( UINT nIndex, BYTE* src, UINT nLen );
	WORD GetWord( UINT nIndex ) const;
	void SetAt( UINT nIndex, WORD wElement );
	void SetString( UINT nIndex, CString& rString );
	void DeleteBase();
	UINT Get_CRC_CheckSum( ULONG ulSize ) const;
protected:
	CNibbleModeProto& m_rNibbleModeDev;
	ptrSendFunc m_pfnSendFromBuffer;
	ptrRecvFunc m_pfnReceiveIntoBuffer;
	CTimer m_tmrWaitS6;
	CFileInfo m_fiInfo;
	CObArray m_aFiInfoBase;
	static const int m_nMaxSizeBase;
	static BYTE abTestData[16];
private:
	BYTE FAR* m_fpBuffer;
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
	virtual BOOL Begin( CWnd* pWndOwner = NULL );
	virtual void KillThread();
	BOOL IsRunning() const;
protected:
	virtual UINT DoWork();
	void SendFileInfo();
	void SendData();
	void ChangeDir();
	void SendTOC();
	void SwitchBusSpeed();
	EOpCode GetOpcode() const;
	void InvalidateOpcode();
	void ReceiveCommand();
	void RetCheckStatus( int nStatus );
	void AddIntoTOC( CBArchive& bar, WIN32_FIND_DATA* pFileData );
	void FormatOutput( LPCTSTR lpszFormat, ... );
protected:
	BYTE GetAt( int nIndex ) const;
	WORD GetWord( int nIndex ) const;
	DWORD GetDword( int nIndex ) const;
private:
	BYTE m_bCDB[8];
	CString m_sDirName;
	CString m_sFileName;		// Wildcards incl.
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif
};


inline WORD CDirectCable::GetWord( UINT nIndex ) const
	{ ASSERT( nIndex >= 0 && nIndex <= BF_MAXLEN - 1 ); return *(WORD FAR*)(m_fpBuffer + nIndex); }
inline void CDirectCable::SetAt( UINT nIndex, WORD wElement )
	{ ASSERT( nIndex >= 0 && nIndex <= BF_MAXLEN - 1 ); *(WORD FAR*)(m_fpBuffer + nIndex) = wElement; }

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
