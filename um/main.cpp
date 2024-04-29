#include <iostream>
#include <string>
#include <windows.h>

namespace driver {
	namespace ctl {
		ULONG hide_process = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}
	
	struct ioctl_hide_request {
		DWORD process_id;
	};

}

VOID hide_process(HANDLE driver_handle, DWORD pid)
{
	driver::ioctl_hide_request req = {};
	req.process_id = pid;

	DeviceIoControl(
		driver_handle,
		driver::ctl::hide_process,
		&req,
		sizeof(req),
		&req,
		sizeof(req),
		nullptr,
		nullptr
	);
}

INT main(INT argc, LPSTR argv[])
{
	DWORD pid = NULL;
	LPSTR pid_arg = nullptr;
	HANDLE driver_handle = CreateFile(
		L"\\\\.\\Hider",
		GENERIC_READ,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	pid_arg = argv[1];
	pid = std::atoi(pid_arg);
	hide_process(driver_handle, pid);
	
}