#include "ntddk.h"
#include "device.h"
#define MAX_VALUE 65535	
#define ON(num) ((flags & (num)) != 0)
#define SET(num) do { flags = flags | (num); } while (0); 
static ULONG flags = 0x00000000;
static BOOLEAN inversion = FALSE;


NTSTATUS CompleteRequest(PIRP Irp, NTSTATUS status, ULONG_PTR Information) {
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = Information;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

// FOR PS/2
/*
BOOLEAN MyIsrRoutine(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA CurrentInput, POUTPUT_PACKET CurrentOutput, UCHAR StatusByte, PUCHAR Byte, PBOOLEAN ContinueProcessing, PMOUSE_STATE MouseState, PMOUSE_RESET_SUBSTATE ResetSubState) {
	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)DeviceObject->DeviceExtension;
	BOOLEAN retVal = TRUE;
	//__debugbreak();
	if (myDeviceExtension->oldHookIsr) {
		retVal = myDeviceExtension->oldHookIsr(myDeviceExtension->oldHookContext, CurrentInput, CurrentOutput, StatusByte, Byte, ContinueProcessing, MouseState, ResetSubState);
		
		if (!retVal || !(*ContinueProcessing)) {
			return retVal;
		}
	}

	*ContinueProcessing = TRUE;
	return retVal;
}*/

void checkInversion(PMOUSE_INPUT_DATA pCursor) {
	
	switch (pCursor->Buttons) {
	case MOUSE_RIGHT_BUTTON_DOWN:
		//__debugbreak();
		DbgPrint("flags %x setright %d setleft %d\n", flags, ON(MOUSE_RIGHT_BUTTON_DOWN), ON(MOUSE_LEFT_BUTTON_DOWN));
		if (ON(MOUSE_RIGHT_BUTTON_DOWN)) {
			if (ON(MOUSE_LEFT_BUTTON_DOWN)) {
				inversion = !inversion;
				pCursor->Buttons = 0;
			}
			flags = 0;
		}
		else {
			SET(MOUSE_RIGHT_BUTTON_DOWN)
			DbgPrint("flags %x setright %d\n", flags, ON(MOUSE_RIGHT_BUTTON_DOWN));
			pCursor->Buttons = 0;
		}
		break;
	case MOUSE_LEFT_BUTTON_DOWN:
		//__debugbreak();
		DbgPrint("flags %x setright %d\n", flags, ON(MOUSE_RIGHT_BUTTON_DOWN));
		if (ON(MOUSE_RIGHT_BUTTON_DOWN)) {
			SET(MOUSE_LEFT_BUTTON_DOWN)
				DbgPrint("flags %x setleft %d\n", flags, ON(MOUSE_LEFT_BUTTON_DOWN));
			pCursor->Buttons = 0;
		} else {
			flags = 0;
		}
		break;
	case MOUSE_RIGHT_BUTTON_UP:
	case MOUSE_LEFT_BUTTON_UP:
		//__debugbreak();
		break;
	default:
		flags = 0;
	}

}

void MyClassServiceCallback(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed) {
	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)DeviceObject->DeviceExtension;
	//__debugbreak();
	PMOUSE_INPUT_DATA pCursor;
	
	for (pCursor = InputDataStart; pCursor < InputDataEnd; pCursor++) {
		DbgPrint("MOVED: LastY: %d lastx %d\n", pCursor->LastY, pCursor->LastX);
		if (pCursor->LastX == 0) {
			checkInversion(pCursor);
		}
		if (!inversion) {
			continue;
		}

		
		if ((pCursor->Flags & MOUSE_MOVE_ABSOLUTE) != 0) {
			pCursor->LastY = MAX_VALUE - pCursor->LastY;
		}
		else {
			pCursor->LastY = -pCursor->LastY;
		}
		

		DbgPrint("PATCHED: LastY: %d lastx %d\n", pCursor->LastY, pCursor->LastX);
	}
	
	(*(PSERVICE_CALLBACK_ROUTINE) myDeviceExtension->oldConnectData.ClassService)(myDeviceExtension->oldConnectData.ClassDeviceObject, InputDataStart, InputDataEnd, InputDataConsumed);
}

NTSTATUS STDCALL DispatchPassThrough(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	//__debugbreak();
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	DbgPrint("PassThrough Irp %x\n", stack->MajorFunction);
	
	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)DeviceObject->DeviceExtension;
	IoSkipCurrentIrpStackLocation(Irp);

	return IoCallDriver(myDeviceExtension->attachedDeviceObject, Irp);
}

NTSTATUS STDCALL DispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	//__debugbreak();

	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)DeviceObject->DeviceExtension;

	if (myDeviceExtension->oldConnectData.ClassService == NULL) {
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
	} else {
		if (InterlockedIncrement(&myDeviceExtension->createCloseCount) >= 1) {
			DbgPrint("Filter created\n");
		}
	}


	return DispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS STDCALL DispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	//__debugbreak();

	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)DeviceObject->DeviceExtension;

	if (InterlockedDecrement(&myDeviceExtension->createCloseCount) <= 0) {
		DbgPrint("Filter closed\n");
	}


	return DispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS STDCALL DispatchPnp(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG info = 0;
	KEVENT myEvent;

	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG fcn = stack->MinorFunction;

	DbgPrint("Minor Irp %x\n", fcn);

	switch (fcn) {
	case IRP_MN_START_DEVICE:
		//__debugbreak();
		KeInitializeEvent(&myEvent, NotificationEvent, FALSE);
		IoCopyCurrentIrpStackLocationToNext(Irp);
		IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE)StartDeviceCompletionRoutine, &myEvent, TRUE, TRUE, TRUE);
		Status = IoCallDriver(myDeviceExtension->attachedDeviceObject, Irp);

		//__debugbreak();
		if (Status == STATUS_PENDING) {
			KeWaitForSingleObject(&myEvent, Executive, KernelMode, FALSE, NULL);
		}
		if (NT_SUCCESS(Status) && NT_SUCCESS(Irp->IoStatus.Status)) {
			myDeviceExtension->deviceStarted = TRUE;
			myDeviceExtension->deviceRemoved = FALSE;
			myDeviceExtension->surpriseRemoval = FALSE;
		}
		CompleteRequest(Irp, Status, info);
		
		break;
		
	case IRP_MN_REMOVE_DEVICE:
		myDeviceExtension->deviceRemoved = TRUE;
		IoSkipCurrentIrpStackLocation(Irp);
		Status = IoCallDriver(myDeviceExtension->attachedDeviceObject, Irp);
		IoDetachDevice(myDeviceExtension->attachedDeviceObject);
		IoDeleteDevice(DeviceObject);

		break;
	case IRP_MN_SURPRISE_REMOVAL:
		myDeviceExtension->surpriseRemoval = TRUE;
	default:
		IoSkipCurrentIrpStackLocation(Irp);
		Status = IoCallDriver(myDeviceExtension->attachedDeviceObject, Irp);

		break;
	}

	return Status;
}


NTSTATUS STDCALL DispatchPower(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	//__debugbreak();
	DbgPrint("Power IRP\n");
	if ((stack->MinorFunction == IRP_MN_SET_POWER)
		&& (stack->Parameters.Power.Type == DevicePowerState)) {
		myDeviceExtension->powerState = stack->Parameters.Power.State.DeviceState;
	}

	PoStartNextPowerIrp(Irp);
	IoSkipCurrentIrpStackLocation(Irp); 
	return PoCallDriver(myDeviceExtension->attachedDeviceObject, Irp);
}

NTSTATUS STDCALL DispatchInternalDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)DeviceObject->DeviceExtension;
	NTSTATUS Status = STATUS_SUCCESS;
	//__debugbreak();
	
	ULONG info = 0;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
	DbgPrint("Internal Device Control %x", code);
	switch (code) {
	case IOCTL_INTERNAL_MOUSE_CONNECT:
		if (myDeviceExtension->oldConnectData.ClassService == NULL) {
			if (cbin >= sizeof(CONNECT_DATA)) {
				CONNECT_DATA *connectData = stack->Parameters.DeviceIoControl.Type3InputBuffer;

				myDeviceExtension->oldConnectData.ClassDeviceObject = connectData->ClassDeviceObject;
				myDeviceExtension->oldConnectData.ClassService = connectData->ClassService;
				connectData->ClassDeviceObject = myDeviceExtension->myDeviceObject;
				connectData->ClassService = MyClassServiceCallback;
				return DispatchPassThrough(DeviceObject, Irp); 
			}
			else {
				//STATUS_INVALID_PARAMETER
				Status = STATUS_INVALID_PARAMETER;
				break;
			}
		}
		else {
			//STATUS_SHARING_VIOLATION
			Status = STATUS_SHARING_VIOLATION;
			break;
		}
	case IOCTL_INTERNAL_MOUSE_DISCONNECT:
		Status = STATUS_NOT_IMPLEMENTED;
		break;
	case IOCTL_INTERNAL_I8042_HOOK_MOUSE:
		// FOR PS/2
		/*if (cbin >= sizeof(INTERNAL_I8042_HOOK_MOUSE)) {
			PINTERNAL_I8042_HOOK_MOUSE myHook = stack->Parameters.DeviceIoControl.Type3InputBuffer;
			myDeviceExtension->oldHookContext = myHook->Context;
			myHook->Context = DeviceObject;
			if (myHook->IsrRoutine) {
				myDeviceExtension->oldHookIsr = myHook->IsrRoutine;
			}
			
			myDeviceExtension->isrWritePort = myHook->IsrWritePort;
			myHook->IsrRoutine = MyIsrRoutine;
			myDeviceExtension->callContext = myHook->CallContext;
			myDeviceExtension->queueMousePacket = myHook->QueueMousePacket;
			return DispatchPassThrough(DeviceObject, Irp); 
		} else {
			Status = STATUS_INVALID_PARAMETER;
			break;
		}*/
	default:
		//__debugbreak();
		//DbgPrint("code %x type %x method %x\n", code, DEVICE_TYPE_FROM_CTL_CODE(code), METHOD_FROM_CTL_CODE(code));
		return DispatchPassThrough(DeviceObject, Irp);
	}


	if (!NT_SUCCESS(Status)) {
		CompleteRequest(Irp, Status, info);
	}
		
	return Status;
}

NTSTATUS STDCALL StartDeviceCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PKEVENT myEvent) {
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	//__debugbreak();
	KeSetEvent(myEvent, 0, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS STDCALL AddDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT TargetDevice) {
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_OBJECT myDeviceObject;
	//__debugbreak();

	Status = IoCreateDevice(DriverObject, sizeof(MyDeviceExtension), NULL, FILE_DEVICE_MOUSE, 0, FALSE, &myDeviceObject);
	if (!NT_SUCCESS(Status)) {
		DbgPrint("Can't create device\n");
		return Status;
	}
	RtlZeroMemory(myDeviceObject->DeviceExtension, sizeof(MyDeviceExtension));

	MyDeviceExtension *myDeviceExtension = (MyDeviceExtension *)myDeviceObject->DeviceExtension;

	do {
		myDeviceExtension->myDeviceObject = myDeviceObject;
		myDeviceExtension->mouseDeviceObject = TargetDevice;
		myDeviceExtension->attachedDeviceObject = IoAttachDeviceToDeviceStack(myDeviceObject, TargetDevice);
		myDeviceExtension->powerState = PowerDeviceD0;
		myDeviceExtension->deviceStarted = FALSE;
		myDeviceExtension->deviceRemoved = FALSE;
		myDeviceExtension->surpriseRemoval = FALSE;
		myDeviceObject->Flags |= (DO_BUFFERED_IO | DO_POWER_PAGABLE);
		myDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	} while(FALSE);
	
	if (myDeviceExtension->attachedDeviceObject == NULL) {
		IoDeleteDevice(myDeviceObject);
		DbgPrint("Can't connect device");
		return STATUS_DEVICE_NOT_CONNECTED;
	}

	return Status;
}