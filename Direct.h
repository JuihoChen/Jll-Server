// Direct.h: interface for the CDirectCable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECT_H__87D71C20_71C8_4D12_9AC7_9DC50ABBBF65__INCLUDED_)
#define AFX_DIRECT_H__87D71C20_71C8_4D12_9AC7_9DC50ABBBF65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "parallel.h"

#define  _ALLFILES		"*.*"
#define  _PARENT_DIR	".."
#define  _SELF_DIR		"."

class CDirectCable : public CObject  
{
    typedef void (CDirectCable::*ptrSendFunc)( UINT nLen ) const;
    typedef void (CDirectCable::*ptrRecvFunc)( UINT nIndex, UINT nLen );

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

		b7_CRC  = 0x80,
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
	void DeleteBase();
protected:
	CNibbleModeProto& m_rNibbleModeDev;
	ptrSendFunc m_pfnSendFromBuffer;
	ptrRecvFunc m_pfnReceiveIntoBuffer;
//	CTimer m_tmrWaitS6;
//	CFileInfo m_fiInfo;
	CObArray m_aFiInfoBase;
//	static const int m_nMaxSizeBase;
private:
	BYTE FAR* m_fpBuffer;
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
#endif
};

class CDCServer : public CDirectCable
{
public:
	CDCServer( CNibbleModeProto& lpt );
private:
	BYTE m_bCDB[8];
	CString m_sDirName;
	CString m_sFileName;	// Wildcards incl.
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif
};

#endif // !defined(AFX_DIRECT_H__87D71C20_71C8_4D12_9AC7_9DC50ABBBF65__INCLUDED_)
