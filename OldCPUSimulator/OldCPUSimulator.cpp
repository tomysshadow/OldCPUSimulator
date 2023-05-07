#include "OldCPUSimulator.h"
#include <vector>
#include <math.h>
#include <windows.h>

#define OLD_CPU_SIMULATOR_OUT true, 1
#define OLD_CPU_SIMULATOR_ERR true, 1, true, __FILE__, __LINE__

void OldCPUSimulator::destroy() {
	//consoleLog("Destroying Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (!closeProcess(syncedProcess)) {
		consoleLog("Failed to Close Process", OLD_CPU_SIMULATOR_ERR);
	}

	if (!closeThread(syncedThread)) {
		consoleLog("Failed to Close Thread", OLD_CPU_SIMULATOR_ERR);
	}

	if (!closeHandle(jobObject)) {
		consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
	}
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
		// this is a unique_ptr because the data in the buffer doesn't matter when copying
		// (only the size)
		systemInformation = std::unique_ptr<BYTE[]>(new BYTE[systemInformationSize]);

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
	HMODULE kernel32ModuleHandle = LoadLibrary(TEXT("KERNEL32.DLL"));

	if (kernel32ModuleHandle) {
		setProcessInformation = (SetProcessInformationProc)GetProcAddress(kernel32ModuleHandle, "SetProcessInformation");
	}

	HMODULE ntdllModuleHandle = LoadLibrary(TEXT("NTDLL.DLL"));

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
		throw std::runtime_error("Failed to Duplicate Old CPU Simulator");
	}
}

OldCPUSimulator &OldCPUSimulator::operator=(const OldCPUSimulator &oldCPUSimulator) {
	//consoleLog("Setting Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (this == &oldCPUSimulator) {
		return *this;
	}

	destroy();

	if (!duplicate(oldCPUSimulator)) {
		throw std::runtime_error("Failed to Duplicate Old CPU Simulator");
	}
	return *this;
}

bool OldCPUSimulator::open(std::string commandLine) {
	consoleLog("Opening Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (opened) {
		return true;
	}

	size_t _commandLineSize = commandLine.size() + 1;
	std::unique_ptr<CHAR[]> _commandLine = std::unique_ptr<CHAR[]>(new CHAR[_commandLineSize]);

	if (!_commandLine) {
		consoleLog("Failed to Allocate commandLine", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	if (strncpy_s(_commandLine.get(), _commandLineSize, commandLine.c_str(), _commandLineSize)) {
		consoleLog("Failed to Copy String", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	STARTUPINFO startupInfo = {};
	startupInfo.cb = sizeof(startupInfo);

	PROCESS_INFORMATION processInformation = {};

	// create the processHandle, fail if we can't
	opened = CreateProcess(NULL, _commandLine.get(), NULL, NULL, FALSE, CREATE_BREAKAWAY_FROM_JOB | CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInformation)
		&& processInformation.hProcess
		&& processInformation.hThread;

	if (!opened) {
		consoleLog("Failed to Create Process", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	syncedProcessID = processInformation.dwProcessId;
	syncedThreadID = processInformation.dwThreadId;
	syncedProcess = processInformation.hProcess;
	syncedThread = processInformation.hThread;

	// we create a job so that if either the process or the synced process ends
	// for whatever reason, we don't sync the process anymore
	jobObject = CreateJobObject(NULL, NULL);

	if (!jobObject || jobObject == INVALID_HANDLE_VALUE) {
		consoleLog("Failed to Create Job Object", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	bool result = true;

	MAKE_SCOPE_EXIT(jobObjectScopeExit) {
		if (!closeHandle(jobObject)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			result = false;
		}
	};

	// this is how we kill both processes if either ends
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobObjectInformation = {};
	jobObjectInformation.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;

	const size_t JOB_OBJECT_INFORMATION_SIZE = sizeof(jobObjectInformation);

	if (!SetInformationJobObject(jobObject, JobObjectExtendedLimitInformation, &jobObjectInformation, JOB_OBJECT_INFORMATION_SIZE)) {
		consoleLog("Failed to Set Job Object Information", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	if (!AssignProcessToJobObject(jobObject, syncedProcess)) {
		consoleLog("Failed to Assign Process to Job Object", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	jobObjectScopeExit.dismiss();
	return result;
}

bool OldCPUSimulator::close() {
	consoleLog("Closing Old CPU Simulator", OLD_CPU_SIMULATOR_OUT);

	if (!opened) {
		return true;
	}

	bool result = true;

	SCOPE_EXIT {
		if (!closeHandle(jobObject)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			result = false;
		}
	};

	SCOPE_EXIT {
		if (!closeThread(syncedThread)) {
			consoleLog("Failed to Close Thread", OLD_CPU_SIMULATOR_ERR);
			result = false;
		}
	};

	SCOPE_EXIT {
		if (!terminateProcess(syncedProcess)) {
			consoleLog("Failed to Terminate Process", OLD_CPU_SIMULATOR_ERR);
			result = false;
		}
	};
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

	// the PostMessage/GetMessage here has been replaced with a SetEvent/WaitForSingleObject so that
	// we are not a window-holding process (to fulfill Windows 11 timer throttling requirement)
	HANDLE timeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!timeEvent || timeEvent == INVALID_HANDLE_VALUE) {
		consoleLog("Failed to Create Event", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	bool result = true;

	SCOPE_EXIT {
		if (!closeHandle(timeEvent)) {
			consoleLog("Failed to Close Handle", OLD_CPU_SIMULATOR_ERR);
			result = false;
		}
	};

	TIMECAPS timeDevCaps = {};
	const size_t TIME_DEV_CAPS_SIZE = sizeof(timeDevCaps);

	if (timeGetDevCaps(&timeDevCaps, TIME_DEV_CAPS_SIZE) != MMSYSERR_NOERROR) {
		consoleLog("Failed to Get Time Dev Caps", OLD_CPU_SIMULATOR_ERR);
		return false;
	}

	// one millisecond (approximately)
	UINT ms = clamp(1, timeDevCaps.wPeriodMin, timeDevCaps.wPeriodMax);
	// one second (approximately)
	UINT s = clamp(1000, timeDevCaps.wPeriodMin, timeDevCaps.wPeriodMax);
	// two seconds (we should never hit this)
	UINT s2 = max(2000, s + s);

	if (!ms || !s || !s2 || s < ms || s2 < s) {
		consoleLog("Invalid Time Dev Caps", OLD_CPU_SIMULATOR_ERR);
		return false;
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
		return false;
	}

	SCOPE_EXIT {
		if (timeEndPeriod(gcdMs) != TIMERR_NOERROR) {
			consoleLog("Failed to End Time Period", OLD_CPU_SIMULATOR_ERR);
			result = false;
		}
	};

	if (syncedProcessMainThreadOnly) {
		consoleLog("Syncing Main Thread Only", OLD_CPU_SIMULATOR_OUT);

		// we start with the synced thread suspended
		suspended = true;

		for (;;) {
			if (!wait(suspendMs, s2, timeEvent)) {
				consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
				return false;
			}

			if (!resumeThread()) {
				// main thread terminated
				// try suspending the process
				break;
			}

			if (!wait(resumeMs, s2, timeEvent)) {
				consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
				return false;
			}

			if (!suspendThread()) {
				// main thread terminated
				// try suspending the process
				break;
			}
		}
	} else {
		if (ResumeThread(syncedThread) == -1) {
			consoleLog("Failed to Resume Thread", OLD_CPU_SIMULATOR_ERR);
			return false;
		}
	}

	suspended = false;

	if (syncMode == SYNC_MODE_SUSPEND_PROCESS) {
		consoleLog("Testing Sync Mode: Suspend Process", OLD_CPU_SIMULATOR_OUT);

		if (ntSuspendProcess && ntResumeProcess) {
			// if the process terminated then return
			if (suspendProcess()) {
				if (suspended) {
					for (;;) {
						if (!wait(suspendMs, s2, timeEvent)) {
							consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
							return false;
						}

						if (!resumeProcess()) {
							// process terminated
							break;
						}

						if (!wait(resumeMs, s2, timeEvent)) {
							consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
							return false;
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

	SCOPE_EXIT {
		closeResumedThreads();

		if (!resumedThreadsVector.empty()) {
			consoleLog("Failed to Close Resumed Threads", OLD_CPU_SIMULATOR_ERR);
			result = false;
		}
	};

	if (syncMode == SYNC_MODE_QUERY_SYSTEM_INFORMATION) {
		consoleLog("Testing Sync Mode: Query System Information", OLD_CPU_SIMULATOR_OUT);

		OSVERSIONINFO osVersionInfo = {};
		osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

		// only for Windows XP and greater
		// this would be possible, but difficult to adapt for Windows 2000/NT 4.0
		// http://www.informit.com/articles/article.aspx?p=22442&seqNum=6
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
								return false;
							}

							resumeThreads();

							if (!wait(resumeMs, s2, timeEvent)) {
								consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
								return false;
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
				return false;
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
					return false;
				} else {
					for (;;) {
						if (!wait(suspendMs, s2, timeEvent)) {
							consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
							return false;
						}

						resumeThreads();

						if (!wait(resumeMs, s2, timeEvent)) {
							consoleLog("Failed to Wait Old CPU Simulator", OLD_CPU_SIMULATOR_ERR);
							return false;
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
			return false;
		}
	}
	return result;
}