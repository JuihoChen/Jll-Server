// Direct.cpp: implementation of the CDirectCable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jll Server.h"
#include "Direct.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

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
#if 0
	ASSERT( nLen > 0 && nLen <= BF_MAXLEN );

	m_rNibbleModeDev.SetPollCounter( 0 );
	for( register UINT n = 0; -- nLen; n ++ )
	{
		m_rNibbleModeDev.WriteByteToPort( m_fpBuffer[ n ] );
	}

	m_rNibbleModeDev.SetPollCounter( 1 );
	m_rNibbleModeDev.WriteByteToPort( m_fpBuffer[ n ] );
#endif
}

void CDirectCable::ReceiveIntoBuffer( register UINT nIndex, UINT nLen )
// the reason to use param nIndex is that data from the opposite
// could be sent in different packets. (ex. header + data)

// NOTE: It's WIERD!
// register declarator for nIndex is necessary to fix MSC (v8.00c)
// compiler's BUG, if programmed as deleted code (2 lines below).
{
#if 0
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
#endif
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

	m_rNibbleModeDev.SetPollCounter( 1 );
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
	m_bCDB[ 0 ] = IllegalOpcode;	// Invalidate Opcode
}

#ifdef _DEBUG
void CDCServer::AssertValid() const
{
	CDirectCable::AssertValid();
}

void CDCServer::Dump( CDumpContext& dc ) const
{
	ASSERT_VALID( this );
	dc << "a Server CDB: ";
	for (int i = 0; i < sizeof( m_bCDB ); i ++)
		dc << m_bCDB[i] << " ";
	dc << "\n";
}
#endif
