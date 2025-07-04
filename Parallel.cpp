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

CParPort::~CParPort()
{

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

CNibbleModeProto::CNibbleModeProto()
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

CNibbleModeProto::~CNibbleModeProto()
{

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

#pragma check_stack( off )	// Turn off switch for code speed...
