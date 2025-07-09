/******************************************************************
  FILE: PARALLE2.CPP
******************************************************************/

#include "stdafx.h"
#include "Jll Server.h"
#include "except.h"
#include "parallel.h"
#include <conio.h>

#ifdef _DEBUG
#undef THIS_FILE
static const char * THIS_FILE = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef POLL_S7_FOR_BUSY

void CNibbleModeProto::MakeControl4Input() const
// To read external inputs at the Control port, the corresponding
// output should be brought to high.
{
	ASSERT_VALID( this );
	CParPort::ControlPortWrite( m_cParaport.GetBaseAddr(), 0xf );
}

WORD CNibbleModeProto::ReadByteFromPortInByte()
// Read 2 bytes of data at the status and control port.
{
	ASSERT_VALID( this );

#define RRR
#ifdef RRR //RRR
	register WORD rwBase = m_cParaport.GetBaseAddr();

	// When S6=0, set D3=0.
	gblQPCTimer.ResetCounter();
	while( (__inbyte( rwBase + 1 ) & iBUSY_VAL) == 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	/// *!*
	/// *!* Cannot assure if status/control could be read in a word?
	/// *!*
	WORD wLData = __inword( rwBase + 1 );	// BYTE bHighNibble = _inp( rwBase+2 );
										// BYTE bLowNibble = _inp( rwBase+1 );
	__outbyte( rwBase, 0 );

	// When the peripheral responds by setting S6=1, set D3=1.
	// LowNibble holds 4 bits of data.
	gblQPCTimer.ResetCounter();
	while( (__inbyte( rwBase + 1 ) & iBUSY_VAL) != 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	WORD wHData = __inword( rwBase + 1 );
	__outbyte( rwBase, oSTROBE_VAL );

#if 1
	return (((BYTE)(((wHData & 0xff00) + (((BYTE)(wHData << 1)) & 0xff)) >> 4)) << 8)
		+ (BYTE)(((wLData & 0xff00) + (((BYTE)(wLData << 1)) & 0xff)) >> 4);
#else
	_asm {
		mov ax,wHData
		add al,al						// shift out bit3
		shl ax,4						// then shift right the whole 8 bits
		mov cx,wLData
		add cl,cl
		shr cx,4
		mov al,cl
		mov wHData,ax
	}
	return wHData;
#endif

#else //RRR
	// When S6=0, set D3=0.
	gblQPCTimer.ResetCounter();
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	BYTE bHighNibble = m_cParaport.rawControlPortRead();
	BYTE bLowNibble = m_cParaport.ReadStatusPort();
	m_cParaport.WriteDataPort( 0 );

	// When the peripheral responds by setting S6=1, set D3=1.
	// LowNibble holds 4 bits of data.
	gblQPCTimer.ResetCounter();
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	bHighNibble = m_cParaport.rawControlPortRead();
	bLowNibble = m_cParaport.ReadStatusPort();
	m_cParaport.WriteDataPort( oSTROBE_VAL );

///	return m_bDataRead = (bHighNibble << 4) + ((bLowNibble >> 3) & 0xf);
#endif
#undef RRR
}

void CNibbleModeProto::WriteByteToPortInByte( WORD wWordToWrite ) const
// Write 2 bytes to the data port.
// The remote system reads the data at its status port and control port.
{
	ASSERT_VALID( this );

#define WWW
#ifdef WWW //WWW
	register WORD rwBase = m_cParaport.GetBaseAddr();

	// When S6=1 (not busy), write the low nibble and set D3=0.
	gblQPCTimer.ResetCounter();
	while( (__inbyte( rwBase + 1 ) & iBUSY_VAL) != 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	BYTE bByteToWrite = (BYTE) wWordToWrite;
	__outbyte( rwBase + 2, bByteToWrite >> 4 );		// High Nibble
	__outbyte( rwBase, bByteToWrite & 0xf );		// Low Nibble

	// When the peripheral responds by setting S6=0, set D3=1.
	gblQPCTimer.ResetCounter();
	while( (__inbyte( rwBase + 1 ) & iBUSY_VAL) == 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	bByteToWrite = (BYTE) (wWordToWrite >> 8);
	__outbyte( rwBase + 2, bByteToWrite >> 4 );		// High Nibble
	__outbyte( rwBase, (bByteToWrite & 0xf) | oSTROBE_VAL );

	//***JHC* Wait the peripheral responds by setting S6=1. *JHC***
	//NOTICE: experimental result...
	//   In transients between CDB & Status nibble in command phase,
	//   a polling for not being busy is necessary as below.
	while( ((CNibbleModeProto*)this)->CheckForPolling() && (__inbyte( rwBase + 1 ) & iBUSY_VAL) != 0x0 )
		;

#else //WWW
	// When S6=1 (not busy), write the low nibble and set D3=0.
	gblQPCTimer.ResetCounter();
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	// **!**
	// **!** Strobe (status) has to be reset AFTER the Control port set.
	// **!**
	BYTE bByteToWrite = (BYTE) wWordToWrite;
	m_cParaport.rawControlPortWrite( bByteToWrite >> 4 );	// High Nibble
	m_cParaport.WriteDataPort( bByteToWrite & 0xf );		// Low Nibble

	// When the peripheral responds by setting S6=0, set D3=1.
	gblQPCTimer.ResetCounter();
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	bByteToWrite = (BYTE) (wWordToWrite >> 8);
	m_cParaport.rawControlPortWrite( bByteToWrite >> 4 );	// High Nibble
	m_cParaport.WriteDataPort( (bByteToWrite & 0xf) | oSTROBE_VAL );

	//***JHC* Wait the peripheral responds by setting S6=1. *JHC***
	//NOTICE: experimental result...
	//   In transients between CDB & Status nibble in command phase,
	//   a polling for not being busy is necessary as below.
	while( ((CNibbleModeProto*)this)->CheckForPolling() && (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
		;
#endif
#undef WWW
}

#endif // POLL_S7_FOR_BUSY
