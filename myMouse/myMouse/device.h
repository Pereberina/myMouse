#include "ntddk.h"
#include "ntddkbd.h"
#include "ntddmou.h"
#include "kbdmou.h"
#include "ntdd8042.h"
#define STDCALL __stdcall

#pragma once
NTSTATUS STDCALL DispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS STDCALL DispatchPassThrough(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS STDCALL DispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS STDCALL DispatchPnp(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS STDCALL DispatchPower(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS STDCALL DispatchInternalDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS STDCALL AddDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT DeviceObject);
NTSTATUS STDCALL StartDeviceCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PKEVENT myEvent);
NTSTATUS CompleteRequest(PIRP Irp, NTSTATUS status, ULONG_PTR Information);
BOOLEAN MyIsrRoutine(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA CurrentInput, POUTPUT_PACKET CurrentOutput, UCHAR StatusByte, PUCHAR Byte, PBOOLEAN ContinueProcessing, PMOUSE_STATE MouseState, PMOUSE_RESET_SUBSTATE ResetSubState);
void checkInversion(PMOUSE_INPUT_DATA pCursor);
void MyClassServiceCallback(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed);


typedef void(*PSERVICE_CALLBACK)(PDEVICE_OBJECT DeviceObject,
	PMOUSE_INPUT_DATA InputDataStart,
	PMOUSE_INPUT_DATA InputDataEnd,
	PULONG InputDataConsumed);


typedef struct _MyDeviceExtension {
	PDEVICE_OBJECT myDeviceObject;
	PDEVICE_OBJECT mouseDeviceObject;
	PDEVICE_OBJECT attachedDeviceObject;
	LONG createCloseCount;
	void *oldHookContext;
	PI8042_MOUSE_ISR oldHookIsr;
	PI8042_ISR_WRITE_PORT isrWritePort;
	void *callContext;
	PI8042_QUEUE_PACKET queueMousePacket;
	//PDEVICE_OBJECT oldClassDeviceObject;
	//PSERVICE_CALLBACK oldMouseClassServiceCallback;
	CONNECT_DATA oldConnectData;
	DEVICE_POWER_STATE powerState;
	BOOLEAN deviceStarted;
	BOOLEAN surpriseRemoval;
	BOOLEAN deviceRemoved;
} MyDeviceExtension;
/*
typedef struct _ConnectData {
	PDEVICE_OBJECT ClassDeviceObject;
	PSERVICE_CALLBACK MouseClassServiceCallback;
} ConnectData;*/

/*
typedef struct INTERNAL_I842_HOOK_MOUSE {
	void *Context;
	void *IsrRoutine;
	void *IsrWritePort;
	void *QueueMousePacket;
	void *CallContext;
} MyHook;*/
