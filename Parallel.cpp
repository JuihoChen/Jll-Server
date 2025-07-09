// Parallel.cpp: implementation of the CParPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jll Server.h"
#include "except.h"
#include "parallel.h"

#ifdef _DEBUG
#undef THIS_FILE
static const char * THIS_FILE = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC( CParPort, CObject )

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
	// Check if PRN driver found okay in the XP system?
	HANDLE hLpt = CreateFile(
		L"\\\\.\\LPT1",
		GENERIC_READ | GENERIC_WRITE,
		0,									// exclusive access 
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if( hLpt == INVALID_HANDLE_VALUE )		// we can't open the LPT device
	{
		TRACE1( "CParPort: Cannot open a handle to LPT1 - %d.\n", GetLastError() );
		return FALSE;
	}
	else
		CloseHandle( hLpt );				// we're done with the handle

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

inline WORD CNibbleModeProto::DivideByteIntoNibbles( BYTE bByteToDivide )
// Using 'BYTE' params forces index to limit in 0 & 255.
{
#ifdef POLL_S7_FOR_BUSY
	return ((bByteToDivide << 4) | bByteToDivide) & 0x0f0f;
#else
	static WORD awNibbles[] =
	{
		0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,	// 0x00-0x07
		0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,	// 0x08-0x0f
		0x0100,0x0101,0x0102,0x0103,0x0104,0x0105,0x0106,0x0107,	// 0x10-0x17
		0x0110,0x0111,0x0112,0x0113,0x0114,0x0115,0x0116,0x0117,	// 0x18-0x1f
		0x0200,0x0201,0x0202,0x0203,0x0204,0x0205,0x0206,0x0207,	// 0x20-0x27
		0x0210,0x0211,0x0212,0x0213,0x0214,0x0215,0x0216,0x0217,	// 0x28-0x2f
		0x0300,0x0301,0x0302,0x0303,0x0304,0x0305,0x0306,0x0307,	// 0x30-0x37
		0x0310,0x0311,0x0312,0x0313,0x0314,0x0315,0x0316,0x0317,	// 0x38-0x3f
		0x0400,0x0401,0x0402,0x0403,0x0404,0x0405,0x0406,0x0407,	// 0x40-0x47
		0x0410,0x0411,0x0412,0x0413,0x0414,0x0415,0x0416,0x0417,	// 0x48-0x4f
		0x0500,0x0501,0x0502,0x0503,0x0504,0x0505,0x0506,0x0507,	// 0x50-0x57
		0x0510,0x0511,0x0512,0x0513,0x0514,0x0515,0x0516,0x0517,	// 0x58-0x5f
		0x0600,0x0601,0x0602,0x0603,0x0604,0x0605,0x0606,0x0607,	// 0x60-0x67
		0x0610,0x0611,0x0612,0x0613,0x0614,0x0615,0x0616,0x0617,	// 0x68-0x6f
		0x0700,0x0701,0x0702,0x0703,0x0704,0x0705,0x0706,0x0707,	// 0x70-0x77
		0x0710,0x0711,0x0712,0x0713,0x0714,0x0715,0x0716,0x0717,	// 0x78-0x7f
		0x1000,0x1001,0x1002,0x1003,0x1004,0x1005,0x1006,0x1007,	// 0x80-0x87
		0x1010,0x1011,0x1012,0x1013,0x1014,0x1015,0x1016,0x1017,	// 0x88-0x8f
		0x1100,0x1101,0x1102,0x1103,0x1104,0x1105,0x1106,0x1107,	// 0x90-0x97
		0x1110,0x1111,0x1112,0x1113,0x1114,0x1115,0x1116,0x1117,	// 0x98-0x9f
		0x1200,0x1201,0x1202,0x1203,0x1204,0x1205,0x1206,0x1207,	// 0xa0-0xa7
		0x1210,0x1211,0x1212,0x1213,0x1214,0x1215,0x1216,0x1217,	// 0xa8-0xaf
		0x1300,0x1301,0x1302,0x1303,0x1304,0x1305,0x1306,0x1307,	// 0xb0-0xb7
		0x1310,0x1311,0x1312,0x1313,0x1314,0x1315,0x1316,0x1317,	// 0xb8-0xbf
		0x1400,0x1401,0x1402,0x1403,0x1404,0x1405,0x1406,0x1407,	// 0xc0-0xc7
		0x1410,0x1411,0x1412,0x1413,0x1414,0x1415,0x1416,0x1417,	// 0xc8-0xcf
		0x1500,0x1501,0x1502,0x1503,0x1504,0x1505,0x1506,0x1507,	// 0xd0-0xd7
		0x1510,0x1511,0x1512,0x1513,0x1514,0x1515,0x1516,0x1517,	// 0xd8-0xdf
		0x1600,0x1601,0x1602,0x1603,0x1604,0x1605,0x1606,0x1607,	// 0xe0-0xe7
		0x1610,0x1611,0x1612,0x1613,0x1614,0x1615,0x1616,0x1617,	// 0xe8-0xef
		0x1700,0x1701,0x1702,0x1703,0x1704,0x1705,0x1706,0x1707,	// 0xf0-0xf7
		0x1710,0x1711,0x1712,0x1713,0x1714,0x1715,0x1716,0x1717		// 0xf8-0xff
	};
	return awNibbles[ bByteToDivide ];
#endif
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNibbleModeProto::~CNibbleModeProto()
{
/**** delete to prevent IO violation in case PortTalk is not opened.
	ASSERT_VALID( this );
	if( PortIsPresent() )		// release the bus (set not busy).
		m_cParaport.WriteDataPort( oSTROBE_VAL );
*****/
}

LONG CNibbleModeProto::GetLptPortInTheRegistry( CString myKey, int myPort )
{
	HKEY phkResult;
	wchar_t myData[255];
	LONG res;
	DWORD mySize;
	DWORD myType;

	swprintf( myData, _countof(myData), L"%s\\%d", (LPCTSTR) myKey, myPort );
	TRACE1( "key value for \"Configuration Data\" = %s\n", myData );

	res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myData, 0, KEY_READ, &phkResult );
	if( res != ERROR_SUCCESS )
		return (-1);

	mySize = _countof(myData);
	myType = REG_BINARY;

	res = RegQueryValueEx(
			phkResult,					// handle to key to query
			L"Configuration Data",	// address of name of value to query
			NULL,						// reserved
			&myType,					// address of buffer for value type
			(LPBYTE)myData,				// address of data buffer
			&mySize	);					// address of data buffer size

	if( res != ERROR_SUCCESS )
		return (-1);
 
	return (myData[0x14] | (myData[0x15]<<8));
}

LONG CNibbleModeProto::GetParallelControllerKey( CString& rKey )
{
	HKEY hKey;
	wchar_t myData[255];
	LONG res;
	DWORD mySize;
	FILETIME ftLastWriteTime;

	rKey.Empty();

	CString myKey;
	myKey.Format( L"HARDWARE\\DESCRIPTION\\System" );

	res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey, 0, KEY_READ, &hKey );
 
	if( res != ERROR_SUCCESS )
		return (-1);

	DWORD dwIndex1;
	CString myKey1;
	for( dwIndex1 = 0; dwIndex1 <= 10; dwIndex1 ++ )
	{
		mySize = _countof(myData);
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
				mySize = _countof(myData);
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
						mySize = _countof(myData);
						res = RegEnumKeyEx( hKey2, dwIndex3, myData, &mySize, NULL, NULL, NULL, &ftLastWriteTime );
         
						if( res == ERROR_SUCCESS ) // ERROR_SUCCESS 3
						{
							if( 0 == _tcsicmp( myData, L"ParallelController" ) )
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

LONG CNibbleModeProto::GetLptPortInACPI()
{
	HKEY hKey;
	wchar_t myData[255];
	LONG res;
	DWORD mySize;
	DWORD myType;
	FILETIME ftLastWriteTime;

	CString myKey;
	myKey.Format( L"SYSTEM\\CurrentControlSet\\Enum\\ACPI" );

	res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey, 0, KEY_READ, &hKey );
 
	if( res != ERROR_SUCCESS )
		return (-1);

	DWORD dwIndex1;
	CString myKey1;
	for( dwIndex1 = 0; dwIndex1 <= 20; dwIndex1 ++ )	// loop for ...ACPI\PNP0xxx
	{
		mySize = _countof(myData);
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
			for( dwIndex2 = 0; dwIndex2 <= 10; dwIndex2 ++ )	// loop for ACPI\PNP0xxx\4&61f3b4b&0
			{
				mySize = _countof(myData);
				res = RegEnumKeyEx( hKey1, dwIndex2, myData, &mySize, NULL, NULL, NULL, &ftLastWriteTime );
         
				if( res == ERROR_SUCCESS ) // ERROR_SUCCESS 2
				{
					myKey2 = myKey1 + "\\" + myData;
         
					HKEY hKey2;
					res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey2, 0, KEY_READ, &hKey2 );
         
					if( res != ERROR_SUCCESS )
						return (-1);
         
					/****************************************/
					mySize = _countof(myData);
					myType = REG_SZ;

					res = RegQueryValueEx(
							hKey2,				// handle to key to query
							L"Service",			// address of name of value to query
							NULL,				// reserved
							&myType,			// address of buffer for value type
							(LPBYTE)myData,		// address of data buffer
							&mySize	);			// address of data buffer size

					if( res != ERROR_SUCCESS )
						continue;

					if( 0 != _tcsicmp( myData, L"Parport"  ) )
						continue;

					/****************************************/
					CString myKey3 = myKey2 + "\\Device Parameters";

					HKEY hKey3;
					res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey3, 0, KEY_READ, &hKey3 );
					if( res != ERROR_SUCCESS )
						continue;

					mySize = _countof(myData);
					myType = REG_SZ;

					res = RegQueryValueEx(
						hKey3,				// handle to key to query
						L"PortName",		// address of name of value to query
						NULL,				// reserved
						&myType,			// address of buffer for value type
						(LPBYTE)myData,		// address of data buffer
						&mySize	);			// address of data buffer size

					if( res != ERROR_SUCCESS )
						continue;

					if( 0 != _tcsicmp( myData, L"LPT1" ) )
						continue;

					/****************************************/
					myKey3 = myKey2 + "\\Control";

					res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, myKey3, 0, KEY_READ, &hKey3 );
					if( res != ERROR_SUCCESS )
						continue;

					mySize = _countof(myData);
					myType = REG_SZ;

					res = RegQueryValueEx(
						hKey3,				// handle to key to query
						L"AllocConfig",		// address of name of value to query
						NULL,				// reserved
						&myType,			// address of buffer for value type
						(LPBYTE)myData,		// address of data buffer
						&mySize	);			// address of data buffer size

					if( res == ERROR_SUCCESS )
					{
						return (myData[0x18] | (myData[0x19]<<8));
					}
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
	CString myKey;
	LONG res;

	// Get Registry to HARDWARE\DESCRIPTION\System\...\ParallelController
	res = GetParallelControllerKey( myKey );
	if( res >= 0 )
	{
		// Only LPT1 & LPT2 are tested against.
		if( (res = GetLptPortInTheRegistry( myKey, 0 )) < 0 )
		{
			res = GetLptPortInTheRegistry(	myKey, 1 );
		}
	}

	// Last try, Get Registry to SYSTEM\CurrentControlSet\Enum\ACPI\...\Control
	if( res < 0 )
	{
		res = GetLptPortInACPI();
	}

	// Port address got to set?
	if( res >= 0 )
	{
		m_cParaport.SetBaseAddr( (WORD)res );
	}
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
	ASSERT( PortIsPresent() );

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
				TRACE1( "Error from Guest to echo number %d.\n", theNo );
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
	TRACE0( "Connection is built.\n" );
	return TRUE;
}

//=====================================================================

BOOL CNibbleModeProto::WatchForIncoming() const
// If S6=0, the opposite end is ready for sending.
{
	if( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0 )
		return FALSE;
	else
	{
		TRACE0( "Sensed for data incoming.\n" );
		return TRUE;
	}
}

void CNibbleModeProto::WriteNibbleToPort( BYTE bNibbleToWrite, CTimer& tmrWaitS6 ) const
// Write a nibble for Check Status to the data port.
// The remote system reads the data at its status port.
{
	ASSERT_VALID( this );

	BYTE bNibblesToWrite = (BYTE) DivideByteIntoNibbles( bNibbleToWrite );

	// When S6=1 (not busy), write the low nibble and set D3=0.
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
	{
		tmrWaitS6.CheckTimeout();		// use TRY/CATCH mechanism
	}
	m_cParaport.WriteDataPort( bNibblesToWrite );  // Low Nibble

	// When the peripheral responds by setting S6=0, set D3=1.
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0 )
	{
		tmrWaitS6.CheckTimeout();		// use TRY/CATCH mechanism
	}
	m_cParaport.WriteDataPort( bNibblesToWrite | oSTROBE_VAL );

	//***JHC* Wait the peripheral responds by setting S6=1. *JHC***
	//NOTICE: experimental result...
	//   In transients between CDB & Status nibble in command phase,
	//   a polling for not being busy is necessary as below.
#if 1	//!v0.21
	while( ((CNibbleModeProto*)this)->CheckForPolling() && (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
		;
#else
	while ((m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0)
	{
		tmrWaitS6.CheckTimeout();		// use TRY/CATCH mechanism
	}
#endif
}

BYTE CNibbleModeProto::ReadByteFromPort( CTimer& tmrWaitS6 )
// Read a byte of data at the status port, in 2 nibbles.
{
	ASSERT_VALID( this );

	// When S6=0, set D3=0.
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0 )
	{
		tmrWaitS6.CheckTimeout();		// use TRY/CATCH mechanism
	}
	BYTE bLowNibble = m_cParaport.ReadStatusPort();
	m_cParaport.WriteDataPort( 0 );

	// When the peripheral responds by setting S6=1, set D3=1.
	// LowNibble holds 4 bits of data.
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
	{
		tmrWaitS6.CheckTimeout();		// use TRY/CATCH mechanism
	}
	BYTE bHighNibble = m_cParaport.ReadStatusPort();
	m_cParaport.WriteDataPort( oSTROBE_VAL );

	return m_bDataRead = MakeByteFromNibbles( bLowNibble, bHighNibble );
}

BYTE CNibbleModeProto::ReadByteFromPort()
// Read a byte of data at the status port, in 2 nibbles.
{
	ASSERT_VALID( this );

///#define RRR
#ifdef RRR //RRR
	register WORD rwBase = m_cParaport.GetBaseAddr();

	// When S6=0, set D3=0.
	gblQPCTimer.ResetCounter();
	while( (StatusPortRead( rwBase ) & iBUSY_VAL) != 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	BYTE bLowNibble = StatusPortRead( rwBase );
	_outp( rwBase, 0 );

	// When the peripheral responds by setting S6=1, set D3=1.
	// LowNibble holds 4 bits of data.
	gblQPCTimer.ResetCounter();
	while( (StatusPortRead( rwBase ) & iBUSY_VAL) == 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	BYTE bHighNibble = StatusPortRead( rwBase );
	_outp( rwBase, oSTROBE_VAL );

#else //RRR
	// When S6=0, set D3=0.
	gblQPCTimer.ResetCounter();
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	BYTE bLowNibble = m_cParaport.ReadStatusPort();
	m_cParaport.WriteDataPort( 0 );

	// When the peripheral responds by setting S6=1, set D3=1.
	// LowNibble holds 4 bits of data.
	gblQPCTimer.ResetCounter();
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	BYTE bHighNibble = m_cParaport.ReadStatusPort();
	m_cParaport.WriteDataPort( oSTROBE_VAL );
#endif
#undef RRR

	return m_bDataRead = MakeByteFromNibbles( bLowNibble, bHighNibble );
}

void CNibbleModeProto::WriteByteToPort( BYTE bByteToWrite ) const
// Write a byte to the data port, in 2 nibbles.
// The remote system reads the data at its status port.
// The data bits are D0, D1, D2, and D4.
// D3 is the strobe.
{
	ASSERT_VALID( this );

	WORD wNibblesToWrite = DivideByteIntoNibbles( bByteToWrite );
#define WWW
#ifdef WWW //WWW
	register WORD rwBase = m_cParaport.GetBaseAddr();

	// When S6=1 (not busy), write the low nibble and set D3=0.
	gblQPCTimer.ResetCounter();
	while( (CParPort::StatusPortRead( rwBase ) & iBUSY_VAL) == 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	__outbyte( rwBase, (BYTE) wNibblesToWrite );

	// When the peripheral responds by setting S6=0, set D3=1.
	gblQPCTimer.ResetCounter();
	while( (CParPort::StatusPortRead( rwBase ) & iBUSY_VAL) != 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	__outbyte( rwBase, (wNibblesToWrite >> 8) | oSTROBE_VAL );

	//***JHC* Wait the peripheral responds by setting S6=1. *JHC***
	//NOTICE: experimental result...
	//   In transients between CDB & Status nibble in command phase,
	//   a polling for not being busy is necessary as below.
	while( ((CNibbleModeProto*)this)->CheckForPolling() && (CParPort::StatusPortRead( rwBase ) & iBUSY_VAL) == 0x0 )
		;

#else //WWW
	// When S6=1 (not busy), write the low nibble and set D3=0.
	gblQPCTimer.ResetCounter();
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	m_cParaport.WriteDataPort( wNibblesToWrite );  // Low Nibble

	// When the peripheral responds by setting S6=0, set D3=1.
	gblQPCTimer.ResetCounter();
	while( (m_cParaport.ReadStatusPort() & iBUSY_VAL) != 0x0 )
	{
		gblQPCTimer.CounterExceedToCheck();
	}
	m_cParaport.WriteDataPort( (wNibblesToWrite >> 8) | oSTROBE_VAL );

	//***JHC* Wait the peripheral responds by setting S6=1. *JHC***
	//NOTICE: experimental result...
	//   In transients between CDB & Status nibble in command phase,
	//   a polling for not being busy is necessary as below.
	while( ((CNibbleModeProto*)this)->CheckForPolling() && (m_cParaport.ReadStatusPort() & iBUSY_VAL) == 0x0 )
		;
#endif
#undef WWW
}
