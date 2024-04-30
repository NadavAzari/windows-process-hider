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
		GENERIC_ALL,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	//printf("Got the handle: %d\n", driver_handle);
	if (INVALID_HANDLE_VALUE == driver_handle)
	{
		printf("Driver is not loaded\n");
		std::cin.get();
		return 0;
	}
	pid_arg = argv[1];
	//printf("Trying to convert to int the pid\n");
	std::cin.get();
	pid = std::atoi(pid_arg);
	//printf("Hiding process with PID: %d\nPress enter to hide the process", pid);
	std::cin.get();
	hide_process(driver_handle, pid);
	//printf("Finished hiding the process\n");
	
}