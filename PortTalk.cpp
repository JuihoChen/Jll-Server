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
	m_dwProcessId = GetCurrentProcessId();
	m_hPortTalk = m_hLPT1 = INVALID_HANDLE_VALUE;
	m_nAllocPortCounter = 0;
}

CPortTalk::~CPortTalk()
{
	if( m_nAllocPortCounter )		// parallel port is still acquired?
		ParFreePort();				// free parallel port from Parport.sys
	ClosePortTalk();
}

int CPortTalk::OpenPortTalk()
{
	int fResult;
	PVOID ioBuffer;
	DWORD BytesReturned;

	// Open PortTalk Driver. If we cannot open it, try installing and starting it.
	m_hPortTalk = CreateFile(
		"\\\\.\\PortTalk",
		GENERIC_READ,
		0,									// exclusive access
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

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
			NULL );
               
		if( m_hPortTalk == INVALID_HANDLE_VALUE )
		{
			TRACE0( "CPortTalk: Couldn't access PortTalk Driver.\n" );
			return InvalidHandleValue;
		}
    }

    // Once we have successfully opened a handle to the PortTalk Driver, we
	// must fill the driver's IOPM with 0xFF to restrict access to all ports
    fResult = DeviceIoControl( 
		m_hPortTalk,
		IOCTL_IOPM_RESTRICT_ALL_ACCESS,
		NULL, 0,						// no input buffer, so pass zero
		NULL, 0,						// output buffer
		&BytesReturned,
		NULL );

	if( !fResult )
	{
		TRACE1( "CPortTalk: error %d occured in IOCTL_IOPM_RESTRICT_ALL_ACCESS\n", GetLastError() );
		return IoControlError;
	}

	// Issue to get DEVICE OBJECT POINTER of parallel port 0.
    fResult = DeviceIoControl( 
		m_hPortTalk,
		IOCTL_PARALLEL_PORT_GET_DEVICE_OBJECT,
		NULL, 0,						// no input buffer, so pass zero
		&ioBuffer, 4,					// output buffer
		&BytesReturned,
		NULL );

	if( fResult )
		m_hLPT1 = ioBuffer;
	else
	{
		TRACE0( "CPortTalk: Couldn't get device object pointer to parallel port 0.\n" );
		return InvalidHandleLPT1;
	}

	return Success;
}

void CPortTalk::ClosePortTalk()
{
	// close first before PortTalk!
	if( m_hLPT1 != INVALID_HANDLE_VALUE )
	{
		DWORD BytesReturned;

		// Issue to dereference OBJECT (of parallel port 0).
	    if( !DeviceIoControl(
			m_hPortTalk,
			IOCTL_PARALLEL_PORT_FREE_DEVICE_OBJECT,
			NULL, 0,						// no input buffer, so pass zero
			NULL, 0,						// output buffer
			&BytesReturned,
			NULL ) )
		{
			TRACE0( "CPortTalk: Couldn't dereference object (of parallel port 0).\n" );
		}
	}
	m_hLPT1 = INVALID_HANDLE_VALUE;

	// m_hPortTalk has to be remained effect untill m_hLPT1 is closed.
	if( m_hPortTalk != INVALID_HANDLE_VALUE )
		CloseHandle( m_hPortTalk );
	m_hPortTalk = INVALID_HANDLE_VALUE;
}

int CPortTalk::EnableIOPM( WORD wOffset )
{
	int iOffset;	// ATTENTION: iOffset holds 4 bytes
	int fResult;
	DWORD BytesReturned;

	// As one byte represents 8 port addresses and that most devices will use
	// a bank of 8 or 16 addresses, you need not specify every port address,
	// only one port in the 8 byte boundary.
	iOffset = wOffset / 8;		// the 3rd byte holds bitmap to set (0)

    fResult = DeviceIoControl(
		m_hPortTalk,
		IOCTL_SET_IOPM,
		&iOffset, 3,
		NULL, 0,
		&BytesReturned,
		NULL );

	if( !fResult )
	{
		TRACE1( "CPortTalk: error %d granting access to assigned Address.\n", GetLastError() );
		return IoControlError;
	}

    fResult = DeviceIoControl(
		m_hPortTalk,
		IOCTL_ENABLE_IOPM_ON_PROCESSID,
		&m_dwProcessId, 4,
		NULL, 0,
		&BytesReturned,
		NULL );

	if( !fResult )
	{
		TRACE1( "CPortTalk: error %d occured enabling IOPM for process.\n", GetLastError() );
		return IoControlError;
	}

	return Success;
}

void CPortTalk::ParAllocPort()
{
	if( m_nAllocPortCounter )
	{
		TRACE0( "CPortTalk: port already been acquired, not to allocate & return.\n" );
		return;
	}

	ASSERT( m_hLPT1 != INVALID_HANDLE_VALUE );
	if( m_hLPT1 != INVALID_HANDLE_VALUE )
	{
		DWORD BytesReturned;
		int fResult = DeviceIoControl(
			m_hPortTalk,
			IOCTL_PARALLEL_PORT_ALLOCATE,
			NULL, 0,
			NULL, 0,
			&BytesReturned,
			NULL );

		if( fResult )
			m_nAllocPortCounter ++;
		else
			TRACE1( "CPortTalk: error %d occured in IOCTL_PARALLEL_PORT_ALLOCATE\n", GetLastError() );
	}
}

void CPortTalk::ParFreePort()
{
	if( m_nAllocPortCounter == 0 )
	{
		TRACE0( "CPortTalk: port not been acquired, not to free & return.\n" );
		return;
	}

	ASSERT( m_hLPT1 != INVALID_HANDLE_VALUE );
	if( m_hLPT1 != INVALID_HANDLE_VALUE )
	{
		DWORD BytesReturned;
		int fResult = DeviceIoControl(
			m_hPortTalk,
			IOCTL_PARALLEL_PORT_FREE,
			NULL, 0,
			NULL, 0,
			&BytesReturned,
			NULL );

		if( fResult )
			m_nAllocPortCounter --;
		else
			TRACE1( "CPortTalk: error %d occured in IOCTL_PARALLEL_PORT_FREE\n", GetLastError() );
	}
}

void CPortTalk::StartPortTalkDriver() const
{
	SC_HANDLE  SchSCManager;
	SC_HANDLE  schService;
	BOOL       ret;

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
				if( InstallPortTalkDriver() ) break;
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
		if( GetLastError() == ERROR_SERVICE_ALREADY_RUNNING )
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

BOOL CPortTalk::InstallPortTalkDriver() const
{
	SC_HANDLE  SchSCManager;
	SC_HANDLE  schService;
	CHAR       DriverFileName[80];
	CString    s;
	BOOL       retv = TRUE;

	// Play the system exclamation sound.
	MessageBeep( MB_ICONEXCLAMATION );

	s = "Our application needs driver PortTalk.sys to be installed in system.\n\r"
		"Continue to install the driver or Cancel to leave.";

	if(	IDOK != MessageBox( NULL, s, AfxGetAppName(), MB_ICONQUESTION | MB_OKCANCEL | MB_OK ) )
		return FALSE;

	// Get Current Directory. Assumes PortTalk.SYS driver is in this directory.
	// Doesn't detect if file exists, nor if file is on removable media - if this
	// is the case then when windows next boots, the driver will fail to load and
	// a error entry is made in the event viewer to reflect this

	// Get System Directory. This should be something like c:\windows\system32 or
	// c:\winnt\system32 with a Maximum Character lenght of 20. As we have a
	// buffer of 80 bytes and a string of 24 bytes to append, we can go for a max
	// of 55 bytes

	if( !GetSystemDirectory( DriverFileName, 55 ) )
	{
		TRACE0( "PortTalk: Failed to get System Directory. Is System Directory Path > 55 Characters?\n" );
		TRACE0( "PortTalk: Please manually copy driver to your system32/driver directory.\n" );
	}

	// Append our Driver Name.
	lstrcat( DriverFileName, "\\Drivers\\PortTalk.sys" );
	TRACE1( "PortTalk: Copying driver to %s\n", DriverFileName );

	// Copy Driver to System32/drivers directory. This fails if the file doesn't exist.

	if( !CopyFile( "PortTalk.sys", DriverFileName, FALSE ) )
	{
		s.Format( "Failed to copy driver to %s\n\r", DriverFileName );
		s += "Please manually copy driver to your system32/driver directory.";
		MessageBox( NULL, s, AfxGetAppName(), MB_ICONSTOP | MB_OK );
		return FALSE;
	}

	// Open Handle to Service Control Manager.
	SchSCManager = OpenSCManager( NULL,						// machine (NULL == local)
								  NULL,						// database (NULL == default)
								  SC_MANAGER_ALL_ACCESS );	// access required

	// Create Service/Driver - This adds the appropriate registry keys in
	// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services - It doesn't
	// care if the driver exists, or if the path is correct.
	schService = CreateService( SchSCManager,				// SCManager database
								"PortTalk",					// name of service
								"PortTalk",					// name to display
								SERVICE_ALL_ACCESS,			// desired access
								SERVICE_KERNEL_DRIVER,		// service type
								SERVICE_DEMAND_START,		// start type
								SERVICE_ERROR_NORMAL,		// error control type
								"System32\\Drivers\\PortTalk.sys", // service's binary
								NULL,						// no load ordering group
								NULL,						// no tag identifier
								NULL,						// no dependencies
								NULL,						// LocalSystem account
								NULL );						// no password

	if( schService == NULL )
	{
		if( GetLastError() == ERROR_SERVICE_EXISTS )
			TRACE0( "PortTalk: Driver already exists. No action taken.\n" );
		else
		{
			retv = FALSE;
			TRACE0( "PortTalk: Unknown error while creating Service.\n" );
		}
	}
	else TRACE0( "PortTalk: Driver successfully installed.\n" );

	// Close Handle to Service Control Manager.
	CloseServiceHandle( schService );

	return retv;
}
