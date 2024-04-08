/*
 *  Copyright (c) 2024 Infineon Technologies AG.
 *
 *  This file is part of TAS Client, an API for device access for Infineon's 
 *  automotive MCUs. 
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  **************************************************************************************************************** */

// tas_utils functions which require an OS specific implementation
// TAS includes
#include "tas_pkt.h"
#include "tas_utils.h"

// Standard includes
#include <cstdio>
#include <cassert>

#ifdef _WIN32
#include <windows.h>
#include <process.h>

void tasutil_get_user_name(char *user_name)
{
	char userNameTmp[256];
	DWORD usernameLen = 256;
	GetUserName(userNameTmp, &usernameLen);
	snprintf(user_name, TAS_NAME_LEN16, "%s", userNameTmp);
}

uint32_t tasutil_get_pid() { return (uint32_t)_getpid(); }

bool tasutil_check_local_tas_server_is_running()
{
	// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-openmutexw
	// https://docs.microsoft.com/en-us/windows/win32/termserv/kernel-object-namespaces

	GetLastError();  // Clear old errors
	HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, L"Global\\only_one_TasServer_on_host");
	DWORD lastError = GetLastError();
	if (lastError != ERROR_SUCCESS) {
		assert(lastError == ERROR_FILE_NOT_FOUND);
		return false;  // TasServer is NOT running
	}
	CloseHandle(hMutex);  // Not needed

	return true;  // TasServer is running
}

void tasutil_start_local_tas_server()
{
	// https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getwindowsdirectory

	char applicationPath[MAX_PATH+16];
	UINT len = GetSystemDirectoryA(applicationPath, MAX_PATH);
	if (len == 0) {
		assert(false);
		return;
	}
	snprintf(&applicationPath[len], MAX_PATH + 16, "\\start_tas_server.bat");

	// Disable redirection immediately prior to the native API function call.
	// https://social.msdn.microsoft.com/Forums/vstudio/en-US/f9a54564-1006-42f9-b4d1-b225f370c60c/getsystemdirectory-should-return-the-native-system32-directory-in-a-64bit-machine?forum=vcgeneral
	// https://learn.microsoft.com/en-us/windows/win32/api/wow64apiset/nf-wow64apiset-wow64disablewow64fsredirection
	PVOID OldValue = NULL;
	BOOL isWoW64 = FALSE;
	if (Wow64DisableWow64FsRedirection(&OldValue)) {
		isWoW64 = TRUE; // Wow64DisableWow64FsRedirection() is success in case of 32 bit applications
	}
	else {
		DWORD winError = GetLastError();  // Just for debugging
		assert(winError == ERROR_INVALID_FUNCTION); // For 64 bit applications, Wow64DisableWow64FsRedirection() fails with ERROR_INVALID_FUNCTION error
		isWoW64 = FALSE;
	}

	PROCESS_INFORMATION processInfo;
	STARTUPINFO         startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	//startupInfo.cb = sizeof startupInfo;
	//int success = CreateProcessA(applicationPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL,
	//	&startupInfo, &processInfo);

	//if (!success) {
	//	DWORD winError = GetLastError();  // Just for debugging
	//	return;
	//}

	//CloseHandle(processInfo.hThread);   // Not needed
	//CloseHandle(processInfo.hProcess);  // Not needed

	HINSTANCE success = ShellExecuteA(0, "open", applicationPath, NULL, NULL, 0);
	if (success < (HINSTANCE)32) {
		DWORD winError = GetLastError();  // Just for debugging
		return;
	}
	if (isWoW64 == TRUE) {
		//  Immediately re-enable redirection. Note that any resources
		//  associated with OldValue are cleaned up by this call.
		if (FALSE == Wow64RevertWow64FsRedirection(OldValue))
		{
			//  Failure to re-enable redirection should be considered
			//  a critical failure and execution aborted.
			return;
		}
	}
}



#else  // Unix

#include <unistd.h>

void tasutil_get_user_name(char* user_name)
{
	getlogin_r(user_name, TAS_NAME_LEN16);
}

uint32_t tasutil_get_pid() { return (uint32_t)getpid(); }

#endif