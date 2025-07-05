/******************************************************************************/
/*                                                                            */
/*                    PortTalk Driver for Windows NT/2000/XP                  */
/*                        Version 2.0, 12th January 2002                      */
/*                          http://www.beyondlogic.org                        */
/*                                                                            */
/* Copyright © 2002 Craig Peacock. Craig.Peacock@beyondlogic.org               */
/* Any publication or distribution of this code in source form is prohibited  */
/* without prior written permission of the copyright holder. This source code */
/* is provided "as is", without any guarantee made as to its suitability or   */
/* fitness for any particular use. Permission is herby granted to modify or   */
/* enhance this sample code to produce a derivative program which may only be */
/* distributed in compiled object form only.                                  */
/******************************************************************************/
//
//     V3.0   DEC 10, 2004   add internal ioctl to request allocates a parallel port
//                           for exclusive access.
//

#include <ntddk.h>
#include "C:\WINDDK\2505\inc\ddk\wxp\parallel.h"
#include "..\porttalk_IOCTL.h"

#define    IOPM_SIZE    0x2000

typedef UCHAR IOPM[IOPM_SIZE];

typedef struct _DEVICE_EXTENSION {
	IOPM *IOPM_local;
	PDEVICE_OBJECT ipDeviceObject;
	PFILE_OBJECT ipFileObject;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

void Ke386SetIoAccessMap(int, IOPM *);
void Ke386QueryIoAccessMap(int, IOPM *);
void Ke386IoSetAccessProcess(PEPROCESS, int);

NTSTATUS PortTalkDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PsLookupProcessByProcessId(IN ULONG ulProcId,OUT struct _EPROCESS ** pEProcess);
VOID PortTalkUnload(IN PDRIVER_OBJECT DriverObject);

NTSTATUS TryGetObjectPointer( IN PDEVICE_EXTENSION DeviceExtension );
NTSTATUS FreeObjectPointer( IN PDEVICE_EXTENSION DeviceExtension );
NTSTATUS TryLockPortNoSelect( IN PDEVICE_EXTENSION DeviceExtension );
NTSTATUS UnlockPortNoDeselect( IN PDEVICE_EXTENSION DeviceExtension );

NTSTATUS PortTalkCreateDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
	IOPM *IOPM_local = 0;
	PDEVICE_OBJECT deviceObject;
	PDEVICE_EXTENSION DeviceExtension;
	NTSTATUS status;
	WCHAR NameBuffer[] = L"\\Device\\PortTalk";
	WCHAR DOSNameBuffer[] = L"\\DosDevices\\PortTalk";
	UNICODE_STRING uniNameString, uniDOSString;

    KdPrint( ("PORTTALK: Porttalk V3.0 12/27/2004 has Loaded") );

    IOPM_local = MmAllocateNonCachedMemory( sizeof(IOPM) );
    if( IOPM_local == 0 ) return STATUS_INSUFFICIENT_RESOURCES;

    RtlFillMemory( IOPM_local, sizeof(IOPM), 0xFF );

    KdPrint( ("PORTTALK: Memory Allocated at %X\n", IOPM_local) );

    RtlInitUnicodeString(&uniNameString, NameBuffer);
    RtlInitUnicodeString(&uniDOSString, DOSNameBuffer);

	status = IoCreateDevice( DriverObject,
							 sizeof( DEVICE_EXTENSION ), // DeviceExtensionSize
							 &uniNameString,
							 FILE_DEVICE_UNKNOWN,
							 0,
							 FALSE,
							 &deviceObject );

    if(!NT_SUCCESS(status))
        return status;

	DeviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;
	DeviceExtension->IOPM_local = IOPM_local;
	DeviceExtension->ipDeviceObject = NULL;
	DeviceExtension->ipFileObject = NULL;

    status = IoCreateSymbolicLink (&uniDOSString, &uniNameString);

    if (!NT_SUCCESS(status))
        return status;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = PortTalkCreateDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PortTalkDeviceControl;
    DriverObject->DriverUnload = PortTalkUnload;

    return STATUS_SUCCESS;
}

NTSTATUS
PortTalkDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )

{
	PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION  irpSp;
	NTSTATUS            ntStatus = STATUS_SUCCESS;   

	ULONG               inBufLength;   /* Input buffer length */
	ULONG               outBufLength;  /* Output buffer length */
	/*ULONG               inBuf;         /* Pointer to Input and output buffer */

    PUCHAR              CharBuffer; 
    PUSHORT             ShortBuffer;
    PULONG              LongBuffer;
    PVOID               ioBuffer;

    USHORT Offset;
    UCHAR Value;
 
    ULONG ProcessID;    
    struct _EPROCESS *Process;

    irpSp = IoGetCurrentIrpStackLocation( pIrp );
    inBufLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    outBufLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

    ioBuffer    = pIrp->AssociatedIrp.SystemBuffer;

    CharBuffer  = (PUCHAR) ioBuffer;
    ShortBuffer = (PUSHORT) ioBuffer;
    LongBuffer  = (PULONG) ioBuffer;

	switch ( irpSp->Parameters.DeviceIoControl.IoControlCode )
	{
	case IOCTL_IOPM_RESTRICT_ALL_ACCESS:

		KdPrint( ("PORTTALK: IOCTL_IOPM_RESTRICT_ALL_ACCESS - RTLFillMemory") );
		RtlFillMemory( DeviceExtension->IOPM_local, sizeof(IOPM), 0xFF );
		pIrp->IoStatus.Information = 0; /* Output Buffer Size */
		ntStatus = STATUS_SUCCESS;
		break;

	case IOCTL_IOPM_ALLOW_EXCUSIVE_ACCESS:

		KdPrint( ("PORTTALK: IOCTL_IOPM_ALLOW_EXCUSIVE_ACCESS - RTLZeroMemory") );
		RtlZeroMemory( DeviceExtension->IOPM_local, sizeof(IOPM) );
		pIrp->IoStatus.Information = 0; /* Output Buffer Size */
		ntStatus = STATUS_SUCCESS;
		break;

	case IOCTL_SET_IOPM: 

		KdPrint( ("PORTTALK: IOCTL_SET_IOPM - Set IO Permission Bitmap") );
		if (inBufLength >= 3) {
			Offset = ShortBuffer[0];
			if (Offset >= 0x2000) {
				ntStatus = STATUS_ARRAY_BOUNDS_EXCEEDED;
				break;
			}
			Value  = CharBuffer[2];
			KdPrint( ("PORTTALK: Offset = %X, Value = %X\n", Offset, Value) );
			*(*DeviceExtension->IOPM_local + Offset) = Value;
			ntStatus = STATUS_SUCCESS;
		} else ntStatus = STATUS_BUFFER_TOO_SMALL;
		pIrp->IoStatus.Information = 0; /* Output Buffer Size */
		////ntStatus = STATUS_SUCCESS;
		break;

	case IOCTL_ENABLE_IOPM_ON_PROCESSID: 

		KdPrint( ("PORTTALK: IOCTL_ENABLE_IOPM_ON_PROCESSID") );
		if (inBufLength >= 4) {
			ProcessID = LongBuffer[0];
			KdPrint( ("PORTTALK: ProcessID Received is %d\n",ProcessID) );
			PsLookupProcessByProcessId(ProcessID, &Process);
			KdPrint( ("PORTTALK: Pointer to Process is %X\n",Process) );
			KdPrint( ("PORTTALK: Address = %X\n",*(*DeviceExtension->IOPM_local + 0x6F) ) );
			Ke386SetIoAccessMap(1, DeviceExtension->IOPM_local);
			Ke386IoSetAccessProcess(Process, 1);
			ntStatus = STATUS_SUCCESS;
		} else ntStatus = STATUS_BUFFER_TOO_SMALL;
		pIrp->IoStatus.Information = 0; /* Output Buffer Size */
		////ntStatus = STATUS_SUCCESS;
		break;

	case IOCTL_READ_PORT_UCHAR:

		if ((inBufLength >= 2) && (outBufLength >= 1)) {
			KdPrint( ("PORTTALK: IOCTL_READ_PORT_UCHAR 0x%X",ShortBuffer[0]) );
			(UCHAR)Value = READ_PORT_UCHAR((PUCHAR)ShortBuffer[0]);
			KdPrint( ("PORTTALK: Value Read %X",Value) );
			CharBuffer[0] = Value;
			ntStatus = STATUS_SUCCESS;
		} else ntStatus = STATUS_BUFFER_TOO_SMALL;
		pIrp->IoStatus.Information = 1; /* Output Buffer Size */
		////ntStatus = STATUS_SUCCESS;
		break;

	case IOCTL_WRITE_PORT_UCHAR:

		if (inBufLength >= 3) {
			KdPrint( ("PORTTALK: IOCTL_WRITE_PORT_UCHAR(0x%X,0x%X)",ShortBuffer[0], CharBuffer[2]) );
			WRITE_PORT_UCHAR((PUCHAR)ShortBuffer[0], CharBuffer[2]);
			ntStatus = STATUS_SUCCESS;
		} else ntStatus = STATUS_BUFFER_TOO_SMALL;
		pIrp->IoStatus.Information = 0; /* Output Buffer Size */
		////ntStatus = STATUS_SUCCESS;
		break;

	case IOCTL_PARALLEL_PORT_GET_DEVICE_OBJECT:

		KdPrint( ("PORTTALK: Getting device object pointer to Parallel0.\n") );
		ntStatus = STATUS_BUFFER_TOO_SMALL;
		pIrp->IoStatus.Information = 0;
		if( outBufLength >= sizeof( PFILE_OBJECT ) ) {
			ntStatus = TryGetObjectPointer( DeviceExtension );
			if( NT_SUCCESS( ntStatus ) )
			{
				LongBuffer[0] = (LONG) DeviceExtension->ipFileObject;
				pIrp->IoStatus.Information = sizeof( PFILE_OBJECT );
			}
		}
		break;

	case IOCTL_PARALLEL_PORT_FREE_DEVICE_OBJECT:

		KdPrint( ("PORTTALK: Dereference device object pointer to Parallel0.\n") );
		ntStatus = FreeObjectPointer( DeviceExtension );
		pIrp->IoStatus.Information = 0;
		break;

	case IOCTL_PARALLEL_PORT_ALLOCATE:

		KdPrint( ("PORTTALK: IOCTL_PARALLEL_PORT_ALLOCATE - allocate port\n") );
		ntStatus = TryLockPortNoSelect( DeviceExtension );
		pIrp->IoStatus.Information = 0;
		break;

	case IOCTL_PARALLEL_PORT_FREE:

		KdPrint( ("PORTTALK: IOCTL_PARALLEL_PORT_FREE - free port\n") );
		ntStatus = UnlockPortNoDeselect( DeviceExtension );
		pIrp->IoStatus.Information = 0;
		break;

	default:

		KdPrint( ("PORTTALK: Unsupported IOCTL Call\n") );
		ntStatus = STATUS_UNSUCCESSFUL;
		pIrp->IoStatus.Information = 0;
		break;
	}

    pIrp->IoStatus.Status = ntStatus;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return ntStatus;
}

VOID PortTalkUnload(IN PDRIVER_OBJECT DriverObject)
{
	PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DriverObject->DeviceObject->DeviceExtension;
	WCHAR DOSNameBuffer[] = L"\\DosDevices\\PortTalk";
	UNICODE_STRING uniDOSString;

    KdPrint( ("PORTTALK: PortTalk is Unloading . .\n") );

    if( DeviceExtension->IOPM_local )
		MmFreeNonCachedMemory( DeviceExtension->IOPM_local, sizeof(IOPM) );
    RtlInitUnicodeString(&uniDOSString, DOSNameBuffer);
    IoDeleteSymbolicLink (&uniDOSString);
    IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS TryGetObjectPointer( IN PDEVICE_EXTENSION DeviceExtension )
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING deviceName;
	PDEVICE_OBJECT ipDeviceObject = NULL;
	PFILE_OBJECT ipFileObject = NULL;

	// first of all, we have to get a pointer to Parallel Device
	RtlInitUnicodeString( &deviceName, L"\\Device\\Parallel0" );
	status = IoGetDeviceObjectPointer( &deviceName, STANDARD_RIGHTS_ALL, &ipFileObject, &ipDeviceObject );
/*******
	status = ObReferenceObjectByHandle(
							objectHandle,
							THREAD_ALL_ACCESS,
							NULL,
							KernelMode,
							&ipDeviceObject,
							NULL
							);
**************/

	if( NT_SUCCESS( status ) )
	{
		DeviceExtension->ipDeviceObject = ipDeviceObject;
		DeviceExtension->ipFileObject = ipFileObject;
	}
	else
		KdPrint( ("PORTTALK: Error while getting device object pointer.\n") );

	return status;
}

NTSTATUS TryLockPortNoSelect( IN PDEVICE_EXTENSION DeviceExtension )
{
	NTSTATUS status = STATUS_SUCCESS, waitStatus = STATUS_SUCCESS;
	KEVENT event;
	IO_STATUS_BLOCK ioStatus;
	PIRP irp;

	if( DeviceExtension->ipDeviceObject )
	{
		// we need initialize the event used later by the Driver to signal us
		// when it finished its work
		KeInitializeEvent( &event, NotificationEvent, FALSE );

		// we build the irp needed to establish fitler function
		irp = IoBuildDeviceIoControlRequest( IOCTL_INTERNAL_LOCK_PORT_NO_SELECT,
			  							     DeviceExtension->ipDeviceObject,
											 NULL, 0,
											 NULL, 0,
											 TRUE,		// call IRP_MJ_INTERNAL_DEVICE_CONTROL
											 &event,
											 &ioStatus );

		if( irp != NULL )
		{
			// we send the IRP
			status = IoCallDriver( DeviceExtension->ipDeviceObject, irp );

			//and finally, we wait for "acknowledge" of IpDriverFilter
			if( status == STATUS_PENDING ) 
			{
				waitStatus = KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );

				if( waitStatus != STATUS_SUCCESS ) 
					KdPrint( ("PORTTALK: Error waiting for ParPort response.\n") );
			}

			status = ioStatus.Status;

			if( !NT_SUCCESS( status ) )
				KdPrint( ("PORTTALK: Error, IO error with ParPort.\n") );
		}
		
		else
		{
			// if we can't allocate the space, we return the corresponding code error
			status = STATUS_INSUFFICIENT_RESOURCES;

			KdPrint( ("PORTTALK: Error building ParPort IRP.\n") );
		}
	}

	else
	{
		status = STATUS_FILE_INVALID;

		KdPrint( ("PORTTALK: invalid device object pointer.\n") );
	}
	
	return status;
}

NTSTATUS FreeObjectPointer( IN PDEVICE_EXTENSION DeviceExtension )
{
	if( DeviceExtension->ipFileObject == NULL )
		return STATUS_FILE_INVALID;

	ObDereferenceObject( DeviceExtension->ipFileObject );

	DeviceExtension->ipDeviceObject = NULL;
	DeviceExtension->ipFileObject = NULL;
	return STATUS_SUCCESS;
}

NTSTATUS UnlockPortNoDeselect( IN PDEVICE_EXTENSION DeviceExtension )
{
	NTSTATUS status = STATUS_SUCCESS, waitStatus = STATUS_SUCCESS;
	KEVENT event;
	IO_STATUS_BLOCK ioStatus;
	PIRP irp;

	if( DeviceExtension->ipDeviceObject )
	{
		// we need initialize the event used later by the Driver to signal us
		// when it finished its work
		KeInitializeEvent( &event, NotificationEvent, FALSE );

		// we build the irp needed to establish fitler function
		irp = IoBuildDeviceIoControlRequest( IOCTL_INTERNAL_UNLOCK_PORT_NO_DESELECT,
			  							     DeviceExtension->ipDeviceObject,
											 NULL, 0,
											 NULL, 0,
											 TRUE,		// call IRP_MJ_INTERNAL_DEVICE_CONTROL
											 &event,
											 &ioStatus );

		if( irp != NULL )
		{
			// we send the IRP
			status = IoCallDriver( DeviceExtension->ipDeviceObject, irp );

			//and finally, we wait for "acknowledge" of IpDriverFilter
			if( status == STATUS_PENDING ) 
			{
				waitStatus = KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );

				if( waitStatus != STATUS_SUCCESS ) 
					KdPrint( ("PORTTALK: Error waiting for ParPort response.\n") );
			}

			status = ioStatus.Status;

			if( !NT_SUCCESS( status ) )
				KdPrint( ("PORTTALK: Error, IO error with ParPort.\n") );
		}
		
		else
		{
			// if we can't allocate the space, we return the corresponding code error
			status = STATUS_INSUFFICIENT_RESOURCES;

			KdPrint( ("PORTTALK: Error building ParPort IRP.\n") );
		}
	}

	else
	{
		status = STATUS_FILE_INVALID;

		KdPrint( ("PORTTALK: invalid device object pointer.\n") );
	}

	return status;
}
