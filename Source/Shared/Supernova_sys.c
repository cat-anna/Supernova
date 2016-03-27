
bool lib_sys_GetSystemInformation(SystemInfo_p info){
	return DoInt0x80(SYSCALL_GET_SYSTEM_INFORMATION, (uint32)info, 0, 0);
}

