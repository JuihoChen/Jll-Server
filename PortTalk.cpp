// PortTalk.cpp: implementation of the CPortTalk class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Jll Server.h"
#include "PortTalk.h"
#include "porttalk_IOCTL.h"
#include <winioctl.h>
#include <winsvc.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPortTalk::CPortTalk()
{
	m_hPortTalk = INVALID_HANDLE_VALUE;
	m_dwProcessId = GetCurrentProcessId();
}

CPortTalk::~CPortTalk()
{
	ClosePortTalk();
}

int CPortTalk::OpenPortTalk()
{
	int error;
	DWORD BytesReturned;

	HANDLE m_hLPT1 = CreateFile(
		"\\\\.\\LPT1",
		GENERIC_READ,
		0,									// exclusive access 
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if( m_hLPT1 == INVALID_HANDLE_VALUE )	// we can't open the drive
	{
		TRACE0( "CPortTalk: Couldn't exclusively open LPT1.\n" );
	}

	// Open PortTalk Driver. If we cannot open it, try installing and starting it.
	m_hPortTalk = CreateFile(
		"\\\\.\\PortTalk",
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if( m_hPortTalk == INVALID_HANDLE_VALUE )
	{
		// Start or Install PortTalk Driver.
		StartPortTalkDriver();
		// Then try to open once more, before failing.
		m_hPortTalk = CreateFile(
			"\\\\.\\PortTalk",
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
               
		if( m_hPortTalk == INVALID_HANDLE_VALUE )
		{
			TRACE0( "CPortTalk: Couldn't access PortTalk Driver.\n" );
			return InvalidHandleValue;
		}
    }

    // Once we have successfully opened a handle to the PortTalk Driver, we
	// must fill the driver's IOPM with 0xFF to restrict access to all ports
    error = DeviceIoControl( 
		m_hPortTalk,
		IOCTL_IOPM_RESTRICT_ALL_ACCESS,
		NULL, 0,						// no input buffer, so pass zero
		NULL, 0,						// output buffer
		&BytesReturned,
		NULL
	);

	if( !error )
	{
		TRACE1( "CPortTalk: error %d occured in IOCTL_IOPM_RESTRICT_ALL_ACCESS\n", GetLastError() );
		return IoControlError;
	}

	return Success;
}

void CPortTalk::ClosePortTalk()
{
	if( m_hPortTalk != INVALID_HANDLE_VALUE )
		CloseHandle( m_hPortTalk );
	m_hPortTalk = INVALID_HANDLE_VALUE;

	if( m_hLPT1 != INVALID_HANDLE_VALUE )
		CloseHandle( m_hLPT1 );
	m_hLPT1 = INVALID_HANDLE_VALUE;
}

int CPortTalk::EnableIOPM( WORD wOffset )
{
	int iOffset;	// ATTENTION: iOffset holds 4 bytes
	int error;
	DWORD BytesReturned;

	// As one byte represents 8 port addresses and that most devices will use
	// a bank of 8 or 16 addresses, you need not specify every port address,
	// only one port in the 8 byte boundary.
	iOffset = wOffset / 8;		// the 3rd byte holds bitmap to set (0)

    error = DeviceIoControl(
		m_hPortTalk,
		IOCTL_SET_IOPM,
		&iOffset, 3,
		NULL, 0,
		&BytesReturned,
		NULL
	);

	if( !error )
	{
		TRACE1( "CPortTalk: error %d granting access to assigned Address.\n", GetLastError() );
		return IoControlError;
	}

    error = DeviceIoControl(
		m_hPortTalk,
		IOCTL_ENABLE_IOPM_ON_PROCESSID,
		&m_dwProcessId, 4,
		NULL, 0,
		&BytesReturned,
		NULL
	);

	if( !error )
	{
		TRACE1( "CPortTalk: error occured talking to Device Driver %d.\n", GetLastError() );
		return IoControlError;
	}

	return Success;
}

void CPortTalk::StartPortTalkDriver()
{
	SC_HANDLE  SchSCManager;
	SC_HANDLE  schService;
	BOOL       ret;
	DWORD      err;

	// Open Handle to Service Control Manager.
	SchSCManager = OpenSCManager( NULL,						// machine (NULL == local)
								  NULL,						// database (NULL == default)
								  SC_MANAGER_ALL_ACCESS );	// access required
                         
	if( SchSCManager == NULL )
	{
		if( GetLastError() == ERROR_ACCESS_DENIED )
		{
			// We do not have enough rights to open the SCM, therefore we must
			// be a poor user with only user rights.
			TRACE0( "CPortTalk: have no rights to access the Service Control Manager.\n" );
			TRACE0( "CPortTalk: the PortTalk driver is not installed or started.\n" );
			return;
		}
	}

	do
	{
		// Open a Handle to the PortTalk Service Database.
		schService = OpenService( SchSCManager,			// handle to service control manager database
								  "PortTalk",			// pointer to name of service to start
								  SERVICE_ALL_ACCESS );	// type of access to service

		if( schService == NULL )
		{
			switch( GetLastError() )
			{
			case ERROR_ACCESS_DENIED:
				TRACE0( "CPortTalk: have no rights to the PortTalk service database.\n" );
				return;
			case ERROR_INVALID_NAME:
				TRACE0( "CPortTalk: The specified service name is invalid.\n" );
				return;
			case ERROR_SERVICE_DOES_NOT_EXIST:
				TRACE0( "CPortTalk: The PortTalk driver does not exist.\n" );
				/***************************************************************
				InstallPortTalkDriver();
				break;
				**************************/
				return;
			}
		}
	}
	while( schService == NULL );

    // Start the PortTalk Driver. Errors will occur here if PortTalk.SYS file doesn't exist.
	ret = StartService( schService,		// service identifier
						0,				// number of arguments
						NULL );			// pointer to arguments
                    
    if( ret )
		TRACE0( "CPortTalk: The PortTalk driver has been successfully started.\n" );
	else
	{
		err = GetLastError();
		if( err == ERROR_SERVICE_ALREADY_RUNNING )
			TRACE0( "CPortTalk: The PortTalk driver is already running.\n" );
		else
		{
			TRACE0( "CPortTalk: Unknown error while starting PortTalk driver service.\n" );
			TRACE0( "CPortTalk: Does PortTalk.SYS exist in System Directory?\n" );
			return;
		}
	}

	// Close handle to Service Control Manager.
	CloseServiceHandle( schService );
}
