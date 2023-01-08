#include "OldCPUSimulator.h"
#include <vector>
#include <math.h>
#include <windows.h>

#define OLD_CPU_SIMULATOR_OUT true, 1
#define OLD_CPU_SIMULATOR_ERR true, 1, true, __FILE__, __LINE__

void OldCPUSimulator::destroy() {
	//consoleLog("Destroying Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (syncedProcess) {
		if (!CloseHandle(syncedProcess)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			goto error3;
		}

		syncedProcess = NULL;
	}

	error3:
	if (syncedThread) {
		if (!CloseHandle(syncedThread)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			goto error2;
		}

		syncedThread = NULL;
	}

	error2:
	if (jobObject && jobObject != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(jobObject)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			goto error;
		}

		jobObject = NULL;
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

bool OldCPUSimulator::duplicate(const OldCPUSimulator &oldCPUSimulator) {
	//consoleLog("Duplicating Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);
	opened = oldCPUSimulator.opened;

	setProcessPriorityHigh = oldCPUSimulator.setProcessPriorityHigh;
	syncedProcessMainThreadOnly = oldCPUSimulator.syncedProcessMainThreadOnly;
	setSyncedProcessAffinityOne = oldCPUSimulator.setSyncedProcessAffinityOne;
	refreshHzFloorFifteen = oldCPUSimulator.refreshHzFloorFifteen;

	syncedProcessID = oldCPUSimulator.syncedProcessID;
	syncedThreadID = oldCPUSimulator.syncedThreadID;

	syncedProcess = oldCPUSimulator.syncedProcess;
	syncedThread = oldCPUSimulator.syncedThread;

	jobObject = oldCPUSimulator.jobObject;

	suspended = oldCPUSimulator.suspended;

	suspendedThreadIDsVector = oldCPUSimulator.suspendedThreadIDsVector;
	suspendedThreadIDsMap = oldCPUSimulator.suspendedThreadIDsMap;

	resumedThreadsVector = oldCPUSimulator.resumedThreadsVector;

	setProcessInformation = oldCPUSimulator.setProcessInformation;

	systemInformationSize = oldCPUSimulator.systemInformationSize;

	if (oldCPUSimulator.systemInformation) {
		if (systemInformation) {
			delete[] systemInformation;
			systemInformation = NULL;
		}

		systemInformation = new BYTE[systemInformationSize];

		if (!systemInformation) {
			consoleLog("Failed to Allocate systemInformation", OLD_CPU_SIMULATOR_ERR);
			return false;
		}
	}
	
	ntSuspendProcess = oldCPUSimulator.ntSuspendProcess;
	ntResumeProcess = oldCPUSimulator.ntResumeProcess;
	ntQuerySystemInformation = oldCPUSimulator.ntQuerySystemInformation;
	return true;
}

OldCPUSimulator::OldCPUSimulator(bool setProcessPriorityHigh, bool syncedProcessMainThreadOnly, bool setSyncedProcessAffinityOne, bool refreshHzFloorFifteen) : setProcessPriorityHigh(setProcessPriorityHigh), syncedProcessMainThreadOnly(syncedProcessMainThreadOnly), setSyncedProcessAffinityOne(setSyncedProcessAffinityOne), refreshHzFloorFifteen(refreshHzFloorFifteen) {
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

OldCPUSimulator::~OldCPUSimulator() {
	//consoleLog("Deconstructing Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);
	destroy();
}

OldCPUSimulator::OldCPUSimulator(const OldCPUSimulator &oldCPUSimulator) {
	//consoleLog("Copy Constructing Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (!duplicate(oldCPUSimulator)) {
		throw "Failed to Duplicate Old CPU Simulator";
	}
}

OldCPUSimulator &OldCPUSimulator::operator=(const OldCPUSimulator &oldCPUSimulator) {
	//consoleLog("Setting Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (this == &oldCPUSimulator) {
		return *this;
	}

	destroy();

	if (!duplicate(oldCPUSimulator)) {
		throw "Failed to Duplicate Old CPU Simulator";
	}
	return *this;
}

bool OldCPUSimulator::open(std::string commandLine) {
	consoleLog("Opening Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (opened) {
		return true;
	}

	bool result = false;

	// we create a job so that if either the processHandle or the synced processHandle ends
	// for whatever reason, we don't sync the processHandle anymore
	jobObject = CreateJobObject(NULL, NULL);

	if (!jobObject || jobObject == INVALID_HANDLE_VALUE) {
		consoleLog("Failed to Create Job Object", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	// this is how we kill both processes if either ends
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobObjectInformation = {};
	jobObjectInformation.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE & JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;

	const size_t JOB_OBJECT_INFORMATION_SIZE = sizeof(jobObjectInformation);

	if (!SetInformationJobObject(jobObject, JobObjectExtendedLimitInformation, &jobObjectInformation, JOB_OBJECT_INFORMATION_SIZE)) {
		consoleLog("Failed to Set Job Object Information", OLD_CPU_SIMULATOR_ERR);
		goto error;
	}

	size_t _commandLineSize = commandLine.size() + 1;
	LPSTR _commandLine = new CHAR[_commandLineSize];

	if (!_commandLine) {
		consoleLog("Failed to Allocate commandLine", OLD_CPU_SIMULATOR_ERR);
		goto error;
	}

	if (strncpy_s(_commandLine, _commandLineSize, commandLine.c_str(), _commandLineSize)) {
		consoleLog("Failed to Copy String", OLD_CPU_SIMULATOR_ERR);
		goto error2;
	}

	{
		STARTUPINFO startupInfo = {};
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInformation = {};

		// create the processHandle, fail if we can't
		opened = CreateProcess(NULL, _commandLine, NULL, NULL, FALSE, CREATE_BREAKAWAY_FROM_JOB, NULL, NULL, &startupInfo, &processInformation)
			&& processInformation.hProcess
			&& processInformation.hThread;

		if (!opened) {
			consoleLog("Failed to Create Process", OLD_CPU_SIMULATOR_ERR);
			goto error2;
		}

		syncedProcessID = processInformation.dwProcessId;
		syncedThreadID = processInformation.dwThreadId;
		syncedProcess = processInformation.hProcess;
		syncedThread = processInformation.hThread;

		if (!AssignProcessToJobObject(jobObject, syncedProcess)) {
			consoleLog("Failed to Assign Process to Job Object", OLD_CPU_SIMULATOR_ERR);
			goto error2;
		}
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
		consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
		goto error;
	}

	jobObject = NULL;
	return result;
}

bool OldCPUSimulator::close() {
	consoleLog("Closing Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (!opened) {
		return true;
	}

	bool result = true;

	if (jobObject && jobObject != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(jobObject)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			result = false;
			goto error3;
		}

		jobObject = NULL;
	}
	error3:
	if (syncedThread) {
		if (!CloseHandle(syncedThread)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			result = false;
			goto error2;
		}

		syncedThread = NULL;
	}
	error2:
	if (syncedProcess) {
		if (!TerminateProcess(syncedProcess, 0)) {
			if (GetLastError() != ERROR_ACCESS_DENIED) {
				consoleLog("Failed to Terminate Process", OLD_CPU_SIMULATOR_ERR);
				result = false;
			}
		}

		if (!CloseHandle(syncedProcess)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			result = false;
			goto error;
		}

		syncedProcess = NULL;
	}
	error:
	return result;
}

bool OldCPUSimulator::run(SYNC_MODE syncMode, ULONG maxMhz, ULONG targetMhz, UINT refreshHz) {
	consoleLog("Running Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (!maxMhz) {
		consoleLog("Max Rate must not be zero", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	if (!targetMhz) {
		consoleLog("Target Rate must not be zero", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	if (!refreshHz) {
		consoleLog("Refresh Rate must not be zero", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	if (!syncedProcess) {
		consoleLog("syncedProcess must not be NULL", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	if (!syncedThread) {
		consoleLog("syncedThread must not be NULL", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	if (targetMhz >= maxMhz) {
		//consoleLog("targetMhz must be less than maxMhz", OLD_CPU_SIMULATOR_ERR);
		//return false;
		consoleLog("Ignoring Sync Mode: Target Rate is greater than or equal to Max Rate", OLD_CPU_SIMULATOR_OUT);

		if (WaitForSingleObject(syncedProcess, INFINITE) != WAIT_OBJECT_0) {
			consoleLog("Failed to Wait For Single Object", OLD_CPU_SIMULATOR_ERR);
			return false;
		}
		return true;
	}

	double suspend = (double)(maxMhz - targetMhz) / maxMhz;

	if (suspend <= 0) {
		consoleLog("suspend must not be less than or equal to zero", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	double resume = (double)targetMhz / maxMhz;

	if (resume <= 0) {
		consoleLog("resume must not be less than or equal to zero", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	HANDLE currentProcess = GetCurrentProcess();

	if (setProcessPriorityHigh) {
		if (!SetPriorityClass(currentProcess, HIGH_PRIORITY_CLASS)) {
			consoleLog("Failed to Set Process Priority", OLD_CPU_SIMULATOR_ERR);
			return false;
		}
	}

	if (setSyncedProcessAffinityOne) {
		if (!setProcessAffinity(syncedProcess, 1)) {
			consoleLog("Failed to Set Synced Process Affinity", OLD_CPU_SIMULATOR_ERR);
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
		consoleLog("Instance Handle must not be NULL", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	if (!RegisterClassEx(&windowClassEx)) {
		consoleLog("Failed to Register Window Class", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	messageOnlyWindowHandle = CreateWindowEx(WS_OVERLAPPED, windowClassEx.lpszClassName, "Old CPU Simulator", WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, NULL, windowClassEx.hInstance, NULL);

	if (!messageOnlyWindowHandle) {
		consoleLog("Failed to Create Message Only Window", OLD_CPU_SIMULATOR_ERR);
		goto error;
	}
	*/

	HANDLE timeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!timeEvent || timeEvent == INVALID_HANDLE_VALUE) {
		consoleLog("Failed to Create Event", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	bool result = false;

	TIMECAPS timeDevCaps = {};
	const size_t TIME_DEV_CAPS_SIZE = sizeof(timeDevCaps);

	if (timeGetDevCaps(&timeDevCaps, TIME_DEV_CAPS_SIZE) != MMSYSERR_NOERROR) {
		consoleLog("Failed to Get Time Dev Caps", OLD_CPU_SIMULATOR_ERR);
		goto error2;
	}

	// one millisecond (approximately)
	UINT ms = clamp(1, timeDevCaps.wPeriodMin, timeDevCaps.wPeriodMax);
	// one second (approximately)
	UINT s = clamp(1000, timeDevCaps.wPeriodMin, timeDevCaps.wPeriodMax);
	// two seconds (we should never hit this)
	UINT s2 = max(2000, s + s);

	if (!ms || !s || !s2 || s < ms || s2 < s) {
		consoleLog("Invalid Time Dev Caps", OLD_CPU_SIMULATOR_ERR);
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

	// should never in any circumstance be lower than ms or higher than s
	UINT suspendMs = clamp((UINT)round(suspend * refreshMs), ms, s);
	UINT resumeMs = clamp((UINT)round(resume * refreshMs), ms, s);

	UINT gcdMs = gcd(suspendMs, resumeMs);

	// set precision to highest value that will work for both suspend/resume wait time
	if (timeBeginPeriod(gcdMs) != TIMERR_NOERROR) {
		consoleLog("Failed to Begin Time Period", OLD_CPU_SIMULATOR_OUT);
		goto error2;
	}

	suspended = false;

	if (syncedProcessMainThreadOnly) {
		consoleLog("Syncing Main Thread Only", OLD_CPU_SIMULATOR_OUT);

		// if the thread terminated then try suspending the process
		if (suspendThread()) {
			if (suspended) {
				for (;;) {
					if (!wait(suspendMs, s2, timeEvent)) {
						consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
						goto error3;
					}

					if (!resumeThread()) {
						// main thread terminated
						// try suspending the process
						break;
					}

					if (!wait(resumeMs, s2, timeEvent)) {
						consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
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
		consoleLog("Testing Sync Mode: Suspend Process", OLD_CPU_SIMULATOR_OUT);

		if (ntSuspendProcess && ntResumeProcess) {
			// if the process terminated then return
			if (suspendProcess()) {
				if (suspended) {
					for (;;) {
						if (!wait(suspendMs, s2, timeEvent)) {
							consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
							goto error3;
						}

						if (!resumeProcess()) {
							// process terminated
							break;
						}

						if (!wait(resumeMs, s2, timeEvent)) {
							consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
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
		consoleLog("Testing Sync Mode: Query System Information", OLD_CPU_SIMULATOR_OUT);

		OSVERSIONINFO osVersionInfo = {};
		osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

		if (ntQuerySystemInformation
			&& GetVersionEx(&osVersionInfo)
			&& ((osVersionInfo.dwMajorVersion > 5)
			|| ((osVersionInfo.dwMajorVersion == 5) && (osVersionInfo.dwMinorVersion >= 1)))) {
			try {
				allocateSystemInformation();

				if (querySystemInformation()) {
					if (suspendedThreadIDsVector.empty()) {
						syncMode = SYNC_MODE_TOOLHELP_SNAPSHOT;
					} else {
						for (;;) {
							if (!wait(suspendMs, s2, timeEvent)) {
								consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
								goto error4;
							}

							resumeThreads();

							if (!wait(resumeMs, s2, timeEvent)) {
								consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
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
				consoleLog("Failed to Query System Information", OLD_CPU_SIMULATOR_ERR);
				goto error4;
			}
		} else {
			syncMode = SYNC_MODE_TOOLHELP_SNAPSHOT;
		}
	}

	if (syncMode == SYNC_MODE_TOOLHELP_SNAPSHOT) {
		consoleLog("Testing Sync Mode: Toolhelp Snapshot", OLD_CPU_SIMULATOR_OUT);

		try {
			if (toolhelpSnapshot()) {
				if (suspendedThreadIDsVector.empty()) {
					consoleLog("No Sync Mode", OLD_CPU_SIMULATOR_ERR);
					goto error4;
				} else {
					for (;;) {
						if (!wait(suspendMs, s2, timeEvent)) {
							consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
							goto error4;
						}

						resumeThreads();

						if (!wait(resumeMs, s2, timeEvent)) {
							consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
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
			consoleLog("Failed to Toolhelp Snapshot", OLD_CPU_SIMULATOR_ERR);
			goto error4;
		}
	}

	result = true;
	error4:
	closeResumedThreads();

	if (!resumedThreadsVector.empty()) {
		consoleLog("Failed to Close Resumed Threads", OLD_CPU_SIMULATOR_ERR);
		result = false;
	}
	error3:
	if (timeEndPeriod(gcdMs) != TIMERR_NOERROR) {
		consoleLog("Failed to End Time Period", OLD_CPU_SIMULATOR_ERR);
		result = false;
	}
	error2:
	if (timeEvent && timeEvent != INVALID_HANDLE_VALUE) {
		/*
		if (!DestroyWindow(messageOnlyWindowHandle)) {
			consoleLog("Failed to Destroy Window", OLD_CPU_SIMULATOR_ERR);
			result = false;
			goto error;
		}

		messageOnlyWindowHandle = NULL;
		*/
		if (!CloseHandle(timeEvent)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			result = false;
			goto error;
		}

		timeEvent = NULL;
	}
	/*
	if (!UnregisterClass(windowClassEx.lpszClassName, windowClassEx.hInstance)) {
		consoleLog("Failed to Unregister Window Class", OLD_CPU_SIMULATOR_ERR);
		result = false;
	}
	*/
	error:
	return result;
}