#pragma once
#define _WIN32_WINNT 0x0500
#include "scope_guard.hpp"
#include <memory>
#include <exception>
#include <string>
#define WIN32_NO_STATUS
#include <windows.h>
#include <winternl.h>
#undef WIN32_NO_STATUS

// this is a documented structure, but needs to be included here
// because the Windows headers don't define it
// http://www.mschaef.com/windows_h_is_wierd
typedef struct _PROCESSOR_POWER_INFORMATION {
	ULONG Number;
	ULONG MaxMhz;
	ULONG CurrentMhz;
	ULONG MhzLimit;
	ULONG MaxIdleState;
	ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

typedef enum _PROCESS_INFORMATION_CLASS {
	ProcessMemoryPriority,
	ProcessMemoryExhaustionInfo,
	ProcessAppMemoryInfo,
	ProcessInPrivateInfo,
	ProcessPowerThrottling,
	ProcessReservedValue1,
	ProcessTelemetryCoverageInfo,
	ProcessProtectionLevelInfo,
	ProcessLeapSecondInfo,
	ProcessMachineTypeInfo,
	ProcessInformationClassMax
} PROCESS_INFORMATION_CLASS;

typedef struct _PROCESS_POWER_THROTTLING_STATE {
	ULONG Version;
	ULONG ControlMask;
	ULONG StateMask;
} PROCESS_POWER_THROTTLING_STATE, *PPROCESS_POWER_THROTTLING_STATE;

#define PROCESS_POWER_THROTTLING_CURRENT_VERSION 1

#define PROCESS_POWER_THROTTLING_EXECUTION_SPEED 0x1
#define PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION 0x4

#define PROCESS_POWER_THROTTLING_VALID_FLAGS (PROCESS_POWER_THROTTLING_EXECUTION_SPEED | PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION)

typedef BOOL(WINAPI *SetProcessInformationProc)(_In_ HANDLE hProcess, _In_ PROCESS_INFORMATION_CLASS ProcessInformationClass, _In_reads_bytes_(ProcessInformationSize) LPVOID ProcessInformation, _In_ DWORD ProcessInformationSize);

typedef NTSTATUS(NTAPI *NtSuspendProcessProc)(__in HANDLE ProcessHandle);
typedef NTSTATUS(NTAPI *NtResumeProcessProc)(__in HANDLE ProcessHandle);

typedef LONG KPRIORITY;

typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID;

typedef struct ___SYSTEM_PROCESS_INFORMATION {
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	BYTE Reserved1[48];
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	PVOID Reserved2;
	ULONG HandleCount;
	ULONG SessionId;
	PVOID Reserved3;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG Reserved4;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	PVOID Reserved5;
	SIZE_T QuotaPagedPoolUsage;
	PVOID Reserved6;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER Reserved7[6];
} __SYSTEM_PROCESS_INFORMATION, *__PSYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_THREAD_INFORMATION {
	LARGE_INTEGER Reserved1[3];
	ULONG Reserved2;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	LONG BasePriority;
	ULONG Reserved3;
	ULONG ThreadState;
	ULONG WaitReason;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef NTSTATUS(NTAPI *NtQuerySystemInformationProc)(__in SYSTEM_INFORMATION_CLASS SystemInformationClass, __inout PVOID SystemInformation, __in ULONG SystemInformationLength, __out_opt PULONG ReturnLength);

enum SYNC_MODE {
	SYNC_MODE_SUSPEND_PROCESS,
	SYNC_MODE_QUERY_SYSTEM_INFORMATION,
	SYNC_MODE_TOOLHELP_SNAPSHOT
};

inline bool stringNullOrEmpty(const char* str) {
	return !str || !*str;
}

inline bool stringsCaseInsensitiveEqual(const char* leftHandSide, const char* rightHandSide) {
	return !_stricmp(leftHandSide, rightHandSide);
}

inline UINT clamp(UINT number, UINT min, UINT max) {
	return min(max, max(min, number));
}

inline UINT gcd(UINT a, UINT b) {
	return a ? gcd(b % a, a) : b;
}

inline bool closeHandle(HANDLE &handle) {
	if (handle && handle != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(handle)) {
			return false;
		}
	}

	handle = NULL;
	return true;
}

inline bool closeMutex(HANDLE &mutex) {
	if (mutex && mutex != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(mutex)) {
			return false;
		}
	}

	mutex = NULL;
	return true;
}

inline bool releaseMutex(HANDLE &mutex) {
	bool result = true;

	if (mutex && mutex != INVALID_HANDLE_VALUE) {
		if (!ReleaseMutex(mutex)) {
			result = false;
		}

		if (!CloseHandle(mutex)) {
			return false;
		}
	}

	mutex = NULL;
	return result;
}

inline bool closeProcess(HANDLE &process) {
	if (process) {
		if (!CloseHandle(process)) {
			return false;
		}
	}

	process = NULL;
	return true;
}

inline bool terminateProcess(HANDLE &process) {
	if (process) {
		if (!TerminateProcess(process, 0)) {
			if (GetLastError() != ERROR_ACCESS_DENIED) {
				return false;
			}
		}

		if (!CloseHandle(process)) {
			return false;
		}
	}

	process = NULL;
	return true;
}

inline bool closeThread(HANDLE &thread) {
	return closeProcess(thread);
}

void consoleLog(const char* str = 0, short newline = true, short tab = false, bool err = false, const char* file = 0, unsigned int line = 0);
bool getMaxMhz(ULONG &maxMhz);
bool setProcessAffinity(HANDLE process, DWORD affinity);
bool honorTimerResolutionRequests(HANDLE process, SetProcessInformationProc setProcessInformation);
std::string getArgumentSliceFromCommandLine(std::string commandLine, int begin = 0, int end = -1);