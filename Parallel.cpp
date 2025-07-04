// Parallel.cpp: implementation of the CParPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jll Server.h"
#include "parallel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParPort::CParPort()
{
	m_wBaseAddress = 0x378;	// value set for AssertValid checking
	m_nPortType = ptNONE;
}

void CParPort::SetBaseAddr( WORD base )
{
	ASSERT_VALID( this );
	ASSERT( base == 0x378 || base == 0x278 );
	m_wBaseAddress = base;
	TestPort();
}

BOOL CParPort::TestPort()
// Test for a port's presence and, if it exists, the type of port.
// In order, check for presence of ECP, EPP, SPP, and PS/2 port.
// Update the information in the Port data.
{
	// For easy case, ECP mode is assumed.
	m_nPortType = ptECP;
	m_nPortType |= ptICTL;

	return TRUE;
}

#ifdef _DEBUG
void CParPort::AssertValid() const
{
	CObject::AssertValid();
	ASSERT( m_wBaseAddress == 0x378 || m_wBaseAddress == 0x278 );
}
#endif

//////////////////////////////////////////////////////////////////////
// CNibbleModeProto Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNibbleModeProto::~CNibbleModeProto()
{
	ASSERT_VALID( this );
	if( PortIsPresent() )		// release the bus (set not busy).
		m_cParaport.WriteDataPort( oSTROBE_VAL );
}

LONG CNibbleModeProto::GetLptPortInTheRegistry( int myPort )
{
	HKEY phkResult;
	CString myKey;
	char myData[255];
	LONG res;
	DWORD mySize;
	DWORD myType;
         
	res = GetParallelControllerKey( myKey );
	if( res < 0 )
		return (-1);
         
	sprintf( myData, "%s\\%d", (const char*)myKey, myPort );
	TRACE1( "key value for \"Configuration Data\" = %s\n", myData );

	res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myData, 0, KEY_READ, &phkResult );
	if( res != ERROR_SUCCESS )
		return (-1);

	mySize = sizeof myData;
	myType = REG_BINARY;

	res = RegQueryValueEx(
			phkResult,					// handle to key to query
			"Configuration Data",		// address of name of value to query
			NULL,						// reserved
			&myType,					// address of buffer for value type
			(unsigned char *)myData,	// address of data buffer
			&mySize						// address of data buffer size
		);
	if( res != ERROR_SUCCESS )
		return (-1);
 
	return (myData[0x14] | (myData[0x15]<<8));
}

LONG CNibbleModeProto::GetParallelControllerKey( CString& rKey )
{
	HKEY hKey;
	char myData[255];
	LONG res;
	DWORD mySize;
	FILETIME ftLastWriteTime;

	rKey.Empty();

	CString myKey;
	myKey.Format( "HARDWARE\\DESCRIPTION\\System" );

	res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey, 0, KEY_READ, &hKey );
 
	if( res != ERROR_SUCCESS )
		return (-1);

	DWORD dwIndex1;
	CString myKey1;
	for( dwIndex1 = 0; dwIndex1 <= 10; dwIndex1 ++ )
	{
		mySize = sizeof myData;
		res = RegEnumKeyEx( hKey, dwIndex1, myData, &mySize, NULL, NULL, NULL, &ftLastWriteTime );
         
		if( res == ERROR_SUCCESS ) // ERROR_SUCCESS 1
		{
			myKey1 = myKey + "\\" + myData;
         
			HKEY hKey1;
			res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey1, 0, KEY_READ, &hKey1 );
 
			if( res != ERROR_SUCCESS )
				return (-1);
         
			DWORD dwIndex2;
			CString myKey2;
			for( dwIndex2 = 0; dwIndex2 <= 10; dwIndex2 ++ )
			{
				mySize = sizeof myData;
				res = RegEnumKeyEx( hKey1, dwIndex2, myData, &mySize, NULL, NULL, NULL, &ftLastWriteTime );
         
				if( res == ERROR_SUCCESS ) // ERROR_SUCCESS 2
				{
					myKey2 = myKey1 + "\\" + myData;
         
					HKEY hKey2;
					res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey2, 0, KEY_READ, &hKey2 );
         
					if( res != ERROR_SUCCESS )
						return (-1);
         
					DWORD dwIndex3;
					for( dwIndex3 = 0; dwIndex3 <= 10; dwIndex3 ++ )
					{
						mySize = sizeof myData;
						res = RegEnumKeyEx( hKey2, dwIndex3, myData, &mySize, NULL, NULL, NULL, &ftLastWriteTime );
         
						if( res == ERROR_SUCCESS ) // ERROR_SUCCESS 3
						{
							if( 0 == strcmp( myData, "ParallelController" ) )
							{
								rKey = myKey2 + "\\" + myData;
								return 0;
							}
						} // ERROR_SUCCESS 3
					} // for (dwIndex3
				} // // ERROR_SUCCESS 2
			} // for (dwIndex2
		} // ERROR_SUCCESS 1
	} // for (dwIndex1
         
	return (-1);
}

#ifdef _DEBUG
void CNibbleModeProto::AssertValid() const
{
	CObject::AssertValid();
	m_cParaport.AssertValid();
}
#endif

void CNibbleModeProto::Setup()
{
	LONG res;

	// Only LPT1 & LPT2 are tested against.
	if( (res = GetLptPortInTheRegistry( 0 )) < 0 )
	{
		if( (res = GetLptPortInTheRegistry(	1 )) < 0 )
		{
			return;
		}
	}

	m_cParaport.SetBaseAddr( (WORD)res );
}

inline BYTE CNibbleModeProto::MakeByteFromNibbles(BYTE bLow, BYTE bHigh)
{
#ifdef POLL_S7_FOR_BUSY
	return ((bLow & 0x78) >> 3) + ((bHigh & 0x78) << 1);
#else
	static BYTE abLoCnvs[] =
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,			// 0x00-0x07
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,			// 0x08-0x0f
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,			// 0x10-0x17
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f			// 0x18-0x2f
	};
	static BYTE abHiCnvs[] =
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,			// 0x00-0x07
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,			// 0x08-0x0f
		0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,			// 0x10-0x17
		0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0			// 0x18-0x2f
	};
	return abLoCnvs[ bLow >> 3 ] + abHiCnvs[ bHigh >> 3 ];
#endif
}

#pragma check_stack( off )	// Turn off switch for code speed...

//===================== Connection Build Up ===========================

void CNibbleModeProto::EnterIdleCondition() const
{
	ASSERT_VALID( this );
	ASSERT( PortIsPresent() );
	m_cParaport.WriteDataPort( oSTROBE_VAL ); // Initialize strobe to 1.
}

BOOL CNibbleModeProto::DetectTheGuest() const
{
	ASSERT_VALID( this );

	if( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0) // S6=1 (not busy)
	{
		return FALSE;
	}

	for( int theNo = 0xF; theNo >= 0; theNo -- )
	{
		DWORD dwStart = GetTickCount();
		for( ;; )
		{
			if( GetTickCount() - dwStart > 20 )
			{
				TRACE1( "Error from Guest to echo number %d.", theNo );
				return FALSE;
			}
			if( theNo & 0x1 )
			{
				if( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0)
				{
					continue;
				}
			}
			else if( (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0)
			{
				continue;
			}
			// pass conditions, check if echo right number.
            if( MakeByteFromNibbles( m_cParaport.ReadStatusPort(), 0 ) == theNo )
			{
				break;
			}
		}
		if( theNo & 0x1 )
			m_cParaport.WriteDataPort( theNo );
		else
			m_cParaport.WriteDataPort( theNo | oSTROBE_VAL );
	}
	TRACE0( "Connection is built." );
	return TRUE;
}

//=====================================================================


