#include "ProcessSync.h"
#include <vector>
#include <math.h>
#include <windows.h>

#define PROCESS_SYNC_OUT true, 1
#define PROCESS_SYNC_ERR true, 1, true, __FILE__, __LINE__

void ProcessSync::destroy() {
	//consoleLog("Destroying Process Sync", PROCESS_SYNC_OUT);

	if (syncedProcess) {
		if (!CloseHandle(syncedProcess)) {
			consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
			goto error4;
		}

		syncedProcess = NULL;
	}

	error4:
	if (syncedThread) {
		if (!CloseHandle(syncedThread)) {
			consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
			goto error3;
		}

		syncedThread = NULL;
	}

	error3:
	if (jobObject && jobObject != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(jobObject)) {
			consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
			goto error2;
		}

		jobObject = NULL;
	}

	error2:
	if (timeEvent && timeEvent != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(timeEvent)) {
			consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
			goto error;
		}

		timeEvent = NULL;
	}

	error:
	if (systemInformation) {
		delete[] systemInformation;
		systemInformation = NULL;
		systemInformationSize = 0;
	}

	setProcessInformation = NULL;

	ntSuspendProcess = NULL;
	ntResumeProcess = NULL;
	ntQuerySystemInformation = NULL;
}

bool ProcessSync::duplicate(const ProcessSync &processSync) {
	//consoleLog("Duplicating Process Sync", PROCESS_SYNC_OUT);
	opened = processSync.opened;

	suspended = processSync.suspended;

	setProcessPriorityHigh = processSync.setProcessPriorityHigh;
	syncedProcessMainThreadOnly = processSync.syncedProcessMainThreadOnly;
	setSyncedProcessAffinityOne = processSync.setSyncedProcessAffinityOne;
	refreshHzFloorFifteen = processSync.refreshHzFloorFifteen;

	pageSize = processSync.pageSize;

	syncedProcess = processSync.syncedProcess;
	syncedThread = processSync.syncedThread;

	jobObject = processSync.jobObject;

	setProcessInformation = processSync.setProcessInformation;

	timeEvent = processSync.timeEvent;

	//extra = processSync.extra;

	ms = processSync.ms;
	s = processSync.s;
	//suspendExtraMs = processSync.suspendExtraMs;
	//resumeExtraMs = processSync.resumeExtraMs;
	suspendWaitMs = processSync.suspendWaitMs;
	resumeWaitMs = processSync.resumeWaitMs;

	systemInformationSize = processSync.systemInformationSize;

	if (processSync.systemInformation) {
		if (systemInformation) {
			delete[] systemInformation;
			systemInformation = NULL;
		}

		systemInformation = new BYTE[systemInformationSize];

		if (!systemInformation) {
			consoleLog("Failed to Allocate systemInformation", PROCESS_SYNC_ERR);
			return false;
		}

		if (memcpy_s(systemInformation, systemInformationSize, processSync.systemInformation, systemInformationSize)) {
			consoleLog("Failed to Copy Memory", PROCESS_SYNC_ERR);
			goto error;
		}
	}

	suspendedThreadsVector = processSync.suspendedThreadsVector;
	suspendedThreadsMap = processSync.suspendedThreadsMap;

	resumedThreadsVector = processSync.resumedThreadsVector;
	
	ntSuspendProcess = processSync.ntSuspendProcess;
	ntResumeProcess = processSync.ntResumeProcess;
	ntQuerySystemInformation = processSync.ntQuerySystemInformation;
	return true;
	error:
	if (systemInformation) {
		delete[] systemInformation;
		systemInformation = NULL;
		systemInformationSize = 0;
	}
	return false;
}

ProcessSync::ProcessSync(bool setProcessPriorityHigh, bool syncedProcessMainThreadOnly, bool setSyncedProcessAffinityOne, bool refreshHzFloorFifteen) : setProcessPriorityHigh(setProcessPriorityHigh), syncedProcessMainThreadOnly(syncedProcessMainThreadOnly), setSyncedProcessAffinityOne(setSyncedProcessAffinityOne), refreshHzFloorFifteen(refreshHzFloorFifteen) {
	SYSTEM_INFO systemInfo = {};
	GetSystemInfo(&systemInfo);
	
	pageSize = systemInfo.dwPageSize;

	if (!pageSize) {
		throw "Failed to Get Page Size";
	}

	HMODULE kernel32ModuleHandle = LoadLibrary("KERNEL32.DLL");

	if (kernel32ModuleHandle) {
		setProcessInformation = (SetProcessInformationProc)GetProcAddress(kernel32ModuleHandle, "SetProcessInformation");
	}

	HMODULE ntdllModuleHandle = LoadLibrary("NTDLL.DLL");

	if (ntdllModuleHandle) {
		ntSuspendProcess = (NtSuspendProcessProc)GetProcAddress(ntdllModuleHandle, "NtSuspendProcess");
		ntResumeProcess = (NtResumeProcessProc)GetProcAddress(ntdllModuleHandle, "NtResumeProcess");
		ntQuerySystemInformation = (NtQuerySystemInformationProc)GetProcAddress(ntdllModuleHandle, "NtQuerySystemInformation");
	}
}

ProcessSync::~ProcessSync() {
	//consoleLog("Deconstructing Process Sync", PROCESS_SYNC_OUT);
	destroy();
}

ProcessSync::ProcessSync(const ProcessSync &processSync) {
	//consoleLog("Copy Constructing Process Sync", PROCESS_SYNC_OUT);

	if (!duplicate(processSync)) {
		throw "Failed to Duplicate Process Sync";
	}
}

ProcessSync &ProcessSync::operator=(const ProcessSync &processSync) {
	//consoleLog("Setting Process Sync", PROCESS_SYNC_OUT);

	if (this == &processSync) {
		return *this;
	}

	destroy();

	if (!duplicate(processSync)) {
		throw "Failed to Duplicate Process Sync";
	}
	return *this;
}

bool ProcessSync::open(std::string commandLine) {
	consoleLog("Opening Process Sync", PROCESS_SYNC_OUT);

	if (opened) {
		return true;
	}

	HANDLE currentProcess = GetCurrentProcess();
	BOOL processIsInJob = FALSE;

	if (!IsProcessInJob(currentProcess, NULL, &processIsInJob)) {
		consoleLog("Failed to Test Process Is In Job", PROCESS_SYNC_ERR);
		return false;
	}

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobObjectInformation = {};
	const size_t JOB_OBJECT_INFORMATION_SIZE = sizeof(jobObjectInformation);

	if (processIsInJob) {
		// if process is in job we need to check if it's the required job
		if (!QueryInformationJobObject(NULL, JobObjectExtendedLimitInformation, &jobObjectInformation, JOB_OBJECT_INFORMATION_SIZE, NULL)) {
			consoleLog("Failed to Query Job Object Information", PROCESS_SYNC_ERR);
			return false;
		}

		processIsInJob = jobObjectInformation.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	}

	bool result = false;

	if (!processIsInJob) {
		// we create a job so that if either the processHandle or the synced processHandle ends
		// for whatever reason, we don't sync the processHandle anymore
		jobObject = CreateJobObject(NULL, NULL);

		if (!jobObject || jobObject == INVALID_HANDLE_VALUE) {
			consoleLog("Failed to Create Job Object", PROCESS_SYNC_ERR);
			return false;
		}

		// this is how we kill both processes if either ends
		jobObjectInformation.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

		if (!SetInformationJobObject(jobObject, JobObjectExtendedLimitInformation, &jobObjectInformation, JOB_OBJECT_INFORMATION_SIZE)) {
			consoleLog("Failed to Set Job Object Information", PROCESS_SYNC_ERR);
			goto error;
		}

		// assign the current processHandle to the job object
		// we assign the synced processHandle later
		if (!AssignProcessToJobObject(jobObject, currentProcess)) {
			consoleLog("Failed to Assign Process to Job Object", PROCESS_SYNC_ERR);
			goto error;
		}
	}

	size_t _commandLineSize = commandLine.size() + 1;
	LPSTR _commandLine = new CHAR[_commandLineSize];

	if (!_commandLine) {
		consoleLog("Failed to Allocate commandLine", PROCESS_SYNC_ERR);
		goto error;
	}

	if (strncpy_s(_commandLine, _commandLineSize, commandLine.c_str(), _commandLineSize)) {
		consoleLog("Failed to Copy String", PROCESS_SYNC_ERR);
		goto error2;
	}

	{
		STARTUPINFO startupInfo = {};
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInformation = {};

		// create the processHandle, fail if we can't
		opened = CreateProcess(NULL, _commandLine, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInformation)
			&& processInformation.hProcess
			&& processInformation.hThread;

		if (!opened) {
			consoleLog("Failed to Create Process", PROCESS_SYNC_ERR);
			goto error2;
		}

		syncedProcessID = processInformation.dwProcessId;
		syncedThreadID = processInformation.dwThreadId;
		syncedProcess = processInformation.hProcess;
		syncedThread = processInformation.hThread;
	}
	result = true;
	error2:
	delete[] _commandLine;
	_commandLine = NULL;
	
	if (result) {
		return true;
	}
	error:
	if (!CloseHandle(jobObject)) {
		consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
		goto error;
	}

	jobObject = NULL;
	return result;
}

bool ProcessSync::close() {
	consoleLog("Closing Process Sync", PROCESS_SYNC_OUT);

	if (!opened) {
		return true;
	}

	bool result = true;

	if (jobObject && jobObject != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(jobObject)) {
			consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
			result = false;
			goto error3;
		}

		jobObject = NULL;
	}
	error3:
	if (syncedThread) {
		if (!CloseHandle(syncedThread)) {
			consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
			result = false;
			goto error2;
		}

		syncedThread = NULL;
	}
	error2:
	if (syncedProcess) {
		if (!TerminateProcess(syncedProcess, 0)) {
			if (GetLastError() != ERROR_ACCESS_DENIED) {
				consoleLog("Failed to Terminate Process", PROCESS_SYNC_ERR);
				result = false;
			}
		}

		if (!CloseHandle(syncedProcess)) {
			consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
			result = false;
			goto error;
		}

		syncedProcess = NULL;
	}
	error:
	return result;
}

bool ProcessSync::run(SYNC_MODE syncMode, ULONG mhzLimit, ULONG targetMhz, UINT refreshHz) {
	consoleLog("Running Process Sync", PROCESS_SYNC_OUT);

	if (!mhzLimit) {
		consoleLog("Rate Limit must not be zero", PROCESS_SYNC_ERR);
		return false;
	}

	if (!targetMhz) {
		consoleLog("Target Rate must not be zero", PROCESS_SYNC_ERR);
		return false;
	}

	if (!refreshHz) {
		consoleLog("Refresh Rate must not be zero", PROCESS_SYNC_ERR);
		return false;
	}

	if (!syncedProcess) {
		consoleLog("Synced Process must not be NULL", PROCESS_SYNC_ERR);
		return false;
	}

	if (!syncedThread) {
		consoleLog("Synced Thread must not be NULL", PROCESS_SYNC_ERR);
		return false;
	}

	if (targetMhz >= mhzLimit) {
		//consoleLog("targetMhz must be less than mhzLimit", PROCESS_SYNC_ERR);
		//return false;
		consoleLog("Ignoring Sync Mode: Target Rate is greater than or equal to Rate Limit", PROCESS_SYNC_OUT);

		if (WaitForSingleObject(syncedProcess, INFINITE) != WAIT_OBJECT_0) {
			consoleLog("Failed to Wait For Single Object", PROCESS_SYNC_ERR);
			return false;
		}
		return true;
	}

	double suspend = (double)(mhzLimit - targetMhz) / mhzLimit;

	if (suspend <= 0) {
		consoleLog("suspend must not be less than or equal to zero", PROCESS_SYNC_ERR);
		return false;
	}

	double resume = (double)targetMhz / mhzLimit;

	if (resume <= 0) {
		consoleLog("resume must not be less than or equal to zero", PROCESS_SYNC_ERR);
		return false;
	}

	HANDLE currentProcess = GetCurrentProcess();

	if (setProcessPriorityHigh) {
		if (!SetPriorityClass(currentProcess, HIGH_PRIORITY_CLASS)) {
			consoleLog("Failed to Set Process Priority", PROCESS_SYNC_ERR);
			return false;
		}
	}

	if (setSyncedProcessAffinityOne) {
		if (!setProcessAffinity(syncedProcess, 1)) {
			consoleLog("Failed to Set Synced Process Affinity", PROCESS_SYNC_ERR);
			return false;
		}
	}

	if (setProcessInformation) {
		honorTimerResolutionRequests(currentProcess, setProcessInformation);
	}

	// create message only window
	// the PostMessage/GetMessage here has been replaced with a SetEvent/WaitForSingleObject so that
	// we are not a window-holding process (to fulfill Windows 11 timer throttling requirement)
	/*
	WNDCLASSEX windowClassEx = {};
	windowClassEx.cbSize = sizeof(windowClassEx);
	windowClassEx.lpfnWndProc = DefWindowProc;
	windowClassEx.hInstance = GetModuleHandle(NULL);
	windowClassEx.lpszClassName = "OLD_CPU_SIMULATOR";

	if (!windowClassEx.hInstance) {
		consoleLog("Instance Handle must not be NULL", PROCESS_SYNC_ERR);
		return false;
	}

	if (!RegisterClassEx(&windowClassEx)) {
		consoleLog("Failed to Register Window Class", PROCESS_SYNC_ERR);
		return false;
	}

	messageOnlyWindowHandle = CreateWindowEx(WS_OVERLAPPED, windowClassEx.lpszClassName, "Old CPU Simulator", WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, NULL, windowClassEx.hInstance, NULL);

	if (!messageOnlyWindowHandle) {
		consoleLog("Failed to Create Message Only Window", PROCESS_SYNC_ERR);
		goto error;
	}
	*/

	timeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!timeEvent || timeEvent == INVALID_HANDLE_VALUE) {
		consoleLog("Failed to Create Event", PROCESS_SYNC_ERR);
		return false;
	}

	bool result = false;

	TIMECAPS timeDevCaps = {};
	const size_t TIME_DEV_CAPS_SIZE = sizeof(timeDevCaps);

	if (timeGetDevCaps(&timeDevCaps, TIME_DEV_CAPS_SIZE) != MMSYSERR_NOERROR) {
		consoleLog("Failed to Get Time Dev Caps", PROCESS_SYNC_ERR);
		goto error2;
	}

	// one millisecond (approximately)
	ms = clamp(1, timeDevCaps.wPeriodMin, timeDevCaps.wPeriodMax);
	// one second (approximately)
	s = clamp(1000, timeDevCaps.wPeriodMin, timeDevCaps.wPeriodMax);

	if (!ms || !s || s < ms) {
		consoleLog("Invalid Time Dev Caps", PROCESS_SYNC_ERR);
		goto error2;
	}

	// if we're suspended 3 / 4
	// and resumed 1 / 4
	// we'll be suspended a minimum of 3 Ms
	// and resumed a minimum of 1 Ms
	// (3 / 4) / (1 / 4) = 3 Ms
	// 3 Ms + 1 Ms = 4 Ms, our minRefreshMs
	double minRefreshMs = (max(suspend, resume) / min(suspend, resume) * ms) + ms;
	double maxRefreshHz = (minRefreshMs > 0) ? ((double)s / minRefreshMs) : ms;
	refreshHz = clamp((UINT)min(refreshHz, maxRefreshHz), ms, s);

	// we do this after in case the Refresh Rate before was well above the maximum
	if (refreshHzFloorFifteen) {
		maxRefreshHz = floor(maxRefreshHz / 15) * 15;
		refreshHz = clamp((UINT)min(floor(refreshHz / 15) * 15, maxRefreshHz), ms, s);
	}

	UINT refreshMs = clamp(s / refreshHz, (UINT)ceil(minRefreshMs), s);

	// should never in any circumstance be lower than ms
	suspendWaitMs = (UINT)max(round(suspend * refreshMs), ms);
	resumeWaitMs = (UINT)max(round(resume * refreshMs), ms);

	UINT gcdMs = gcd(suspendWaitMs, resumeWaitMs);

	// set precision to highest value that will work for both suspend/resume wait time
	if (timeBeginPeriod(gcdMs) != TIMERR_NOERROR) {
		consoleLog("Failed to Begin Time Period", PROCESS_SYNC_OUT);
		goto error2;
	}

	suspended = false;

	if (syncedProcessMainThreadOnly) {
		consoleLog("Syncing Main Thread Only", PROCESS_SYNC_OUT);

		// if the thread terminated then try suspending the process
		if (suspendThread()) {
			if (suspended) {
				for (;;) {
					if (!suspendWait()) {
						consoleLog("Failed to Suspend Wait Process Sync", PROCESS_SYNC_ERR);
						goto error3;
					}

					if (!resumeThread()) {
						// main thread terminated
						// try suspending the process
						break;
					}

					if (!resumeWait()) {
						consoleLog("Failed to Resume Wait Process Sync", PROCESS_SYNC_ERR);
						goto error3;
					}

					if (!suspendThread()) {
						// main thread terminated
						// try suspending the process
						break;
					}
				}
			}
		}
	}

	if (syncMode == SYNC_MODE_SUSPEND_PROCESS) {
		consoleLog("Testing Sync Mode: Suspend Process", PROCESS_SYNC_OUT);

		if (ntSuspendProcess && ntResumeProcess) {
			// if the process terminated then return
			if (suspendProcess()) {
				if (suspended) {
					for (;;) {
						if (!suspendWait()) {
							consoleLog("Failed to Suspend Wait Process Sync", PROCESS_SYNC_ERR);
							goto error3;
						}

						if (!resumeProcess()) {
							// process terminated
							break;
						}

						if (!resumeWait()) {
							consoleLog("Failed to Resume Wait Process Sync", PROCESS_SYNC_ERR);
							goto error3;
						}

						if (!suspendProcess()) {
							// process terminated
							break;
						}
					}
				} else {
					// the process isn't suspended
					// try a different sync mode
					syncMode = SYNC_MODE_QUERY_SYSTEM_INFORMATION;
				}
			}
		} else {
			syncMode = SYNC_MODE_QUERY_SYSTEM_INFORMATION;
		}
	}

	if (syncMode == SYNC_MODE_QUERY_SYSTEM_INFORMATION) {
		consoleLog("Testing Sync Mode: Query System Information", PROCESS_SYNC_OUT);

		OSVERSIONINFO osVersionInfo = {};
		osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

		if (ntQuerySystemInformation
			&& GetVersionEx(&osVersionInfo)
			&& ((osVersionInfo.dwMajorVersion > 5)
			|| ((osVersionInfo.dwMajorVersion == 5) && (osVersionInfo.dwMinorVersion >= 1)))) {
			try {
				allocateSystemInformation();

				if (querySystemInformation()) {
					if (suspendedThreadsVector.empty()) {
						syncMode = SYNC_MODE_TOOLHELP_SNAPSHOT;
					} else {
						for (;;) {
							if (!suspendWait()) {
								consoleLog("Failed to Suspend Wait Process Sync", PROCESS_SYNC_ERR);
								goto error4;
							}

							resumeThreads();

							if (!resumeWait()) {
								consoleLog("Failed to Resume Wait Process Sync", PROCESS_SYNC_ERR);
								goto error4;
							}

							if (!querySystemInformation()) {
								// process terminated
								break;
							}
						}
					}
				}
			} catch (...) {
				consoleLog("Failed to Query System Information", PROCESS_SYNC_ERR);
				goto error4;
			}
		} else {
			syncMode = SYNC_MODE_TOOLHELP_SNAPSHOT;
		}
	}

	if (syncMode == SYNC_MODE_TOOLHELP_SNAPSHOT) {
		consoleLog("Testing Sync Mode: Toolhelp Snapshot", PROCESS_SYNC_OUT);

		try {
			if (toolhelpSnapshot()) {
				if (suspendedThreadsVector.empty()) {
					consoleLog("No Sync Mode", PROCESS_SYNC_ERR);
					goto error4;
				} else {
					for (;;) {
						if (!suspendWait()) {
							consoleLog("Failed to Suspend Wait Process Sync", PROCESS_SYNC_ERR);
							goto error4;
						}

						resumeThreads();

						if (!resumeWait()) {
							consoleLog("Failed to Resume Wait Process Sync", PROCESS_SYNC_ERR);
							goto error4;
						}

						if (!toolhelpSnapshot()) {
							// process terminated
							break;
						}
					}
				}
			}
		} catch (...) {
			consoleLog("Failed to Toolhelp Snapshot", PROCESS_SYNC_ERR);
			goto error4;
		}
	}

	result = true;
	error4:
	closeResumedThreads();

	if (!resumedThreadsVector.empty()) {
		consoleLog("Failed to Close Resumed Threads", PROCESS_SYNC_ERR);
		result = false;
	}
	error3:
	if (timeEndPeriod(gcdMs) != TIMERR_NOERROR) {
		consoleLog("Failed to End Time Period", PROCESS_SYNC_ERR);
		result = false;
	}
	error2:
	if (timeEvent && timeEvent != INVALID_HANDLE_VALUE) {
		/*
		if (!DestroyWindow(messageOnlyWindowHandle)) {
			consoleLog("Failed to Destroy Window", PROCESS_SYNC_ERR);
			result = false;
			goto error;
		}

		messageOnlyWindowHandle = NULL;
		*/
		if (!CloseHandle(timeEvent)) {
			consoleLog("Failed to Close Handle", PROCESS_SYNC_ERR);
			result = false;
			goto error;
		}

		timeEvent = NULL;
	}
	/*
	if (!UnregisterClass(windowClassEx.lpszClassName, windowClassEx.hInstance)) {
		consoleLog("Failed to Unregister Window Class", PROCESS_SYNC_ERR);
		result = false;
	}
	*/
	error:
	return result;
}