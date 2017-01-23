#include "ntddk.h"
#include "device.h"


NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING RegistryPath);
void DriverUnload(PDRIVER_OBJECT DriverObject);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (TEXT, DriverUnload)
#endif

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT  DriverObject,
	_In_ PUNICODE_STRING RegistryPath
	)
	/*++

	Routine Description:
	DriverEntry initializes the driver and is the first routine called by the
	system after the driver is loaded. DriverEntry specifies the other entry
	points in the function driver, such as EvtDevice and DriverUnload.

	Parameters Description:

	DriverObject - represents the instance of the function driver that is loaded
	into memory. DriverEntry must initialize members of DriverObject before it
	returns to the caller. DriverObject is allocated by the system before the
	driver is loaded, and it is released by the system after the system unloads
	the function driver from memory.

	RegistryPath - represents the driver specific path in the Registry.
	The function driver can use the path to store driver related data between
	reboots. The path does not store hardware instance specific data.

	Return Value:

	STATUS_SUCCESS if successful,
	STATUS_UNSUCCESSFUL otherwise.

	--*/
{
	NTSTATUS Status = 0;

	__debugbreak();
	UNREFERENCED_PARAMETER(RegistryPath);
	
	for (int i = IRP_MJ_CREATE; i < IRP_MJ_PNP; i++) {
		DriverObject->MajorFunction[i] = DispatchPassThrough;
	}
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = dispatchIrpInternalDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
	DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
	DriverObject->DriverUnload = DriverUnload;
	DriverObject->DriverExtension->AddDevice = AddDevice;


	DbgPrint("Driver loaded!\n");

	return Status;
}


void DriverUnload(PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	__debugbreak();

	DbgPrint("Driver unloaded!\n");
}
