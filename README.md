## Windows Process Hider

The pproect is seperated to two main obejcts:
  - The EXE - user space application which the user will use to hide the process
  - The Driver - Kernel mode application which will actually hide the process

The Driver is exporting an **ioctl** which gets a buffer of 4 bytes represents a PID
Then its jumping to the **ActiveProcessLinks** field and unlink it:

```bash
Previous <-> Target Process <-> Next
```
Usage:

```bash
hide.exe <PID>
```
