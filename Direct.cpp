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

IMPLEMENT_DYNAMIC( CDirectCable, CObject )
IMPLEMENT_DYNAMIC( CDCServer, CDirectCable )

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

#if 0
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
	m_pThread = NULL;				// Invalidate thread pointer
	m_bCDB[ 0 ] = IllegalOpcode;	// Invalidate Opcode
}

CDCServer::~CDCServer()
{
	if( m_pThread )
	{
		TRACE0( "CDCServer: *** Warning: deleting active thread! ***\n" );
		delete m_pThread;
	}
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
	pJob->m_uErr = pJob->DoWork();			// call virt fn to do the work
	pJob->m_pThread = NULL;					// done: clear
	pJob->m_bRunning = FALSE;
	return pJob->m_uErr;					// ..and return error code to Windows
}

//////////////////
// Begin running the worker thread. Args are owner window and callback
// message ID to use for OnProgress notifications, if any. You could enhance
// this to expose pritority and other AfxBeginThread args.
//
BOOL CDCServer::Begin( CWnd* pWndOwner /* = NULL */, UINT ucbMsg /* = 0 */)
{
	m_hWndOwner = pWndOwner->GetSafeHwnd();
	m_ucbMsg = ucbMsg;
	m_uErr = 0;
	m_bRunning = TRUE;
	m_pThread = AfxBeginThread( ThreadProc, this );
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
	dc << "a Server CDB: ";
	for (int i = 0; i < sizeof( m_bCDB ); i ++)
		dc << m_bCDB[i] << " ";
	dc << "\n";
}
#endif

UINT CDCServer::DoWork()
{
	TRACE( "Hello, how are you!" );
	TRACE1( "port# is %x", m_rNibbleModeDev.GetBaseAddr() );
	return 0;
}
