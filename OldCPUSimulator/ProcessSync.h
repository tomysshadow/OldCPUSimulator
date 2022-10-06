#pragma once
#include "shared.h"
#include <vector>
#include <map>
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <TlHelp32.h>

class ProcessSync {
	private:
	void destroy();
	bool duplicate(const ProcessSync &processSync);

	bool opened = false;

	bool suspended = false;

	bool setProcessPriorityHigh = false;
	bool syncedProcessMainThreadOnly = false;
	bool setSyncedProcessAffinityOne = false;
	bool refreshHzFloorFifteen = false;

	DWORD syncedProcessID = 0;
	DWORD syncedThreadID = 0;

	HANDLE syncedProcess = NULL;
	HANDLE syncedThread = NULL;

	HANDLE jobObject = NULL;

	SetProcessInformationProc setProcessInformation = NULL;

	HANDLE timeEvent = NULL;

	//double extra = 1.0;

	UINT ms = 1;
	UINT s = 1000;
	//UINT suspendExtraMs = 0;
	//UINT resumeExtraMs = 0;
	UINT suspendWaitMs = 0;
	UINT resumeWaitMs = 0;

	SIZE_T systemInformationSize = 0;
	PVOID systemInformation = NULL;

	// I tested it in profiler, unordered_map isn't faster
	typedef std::vector<DWORD> SUSPENDED_THREADS_VECTOR;
	typedef std::map<DWORD, HANDLE> SUSPENDED_THREADS_MAP;

	SUSPENDED_THREADS_VECTOR suspendedThreadsVector = {};
	SUSPENDED_THREADS_MAP suspendedThreadsMap = {};

	typedef std::vector<HANDLE> RESUMED_THREADS_VECTOR;
	RESUMED_THREADS_VECTOR resumedThreadsVector = {};

	NtSuspendProcessProc ntSuspendProcess = NULL;
	NtResumeProcessProc ntResumeProcess = NULL;
	NtQuerySystemInformationProc ntQuerySystemInformation = NULL;
	
	inline bool ProcessSync::wait(UINT timerID/*, UINT &extraMs, UINT waitMs*/) {
		/*
		MSG message = {};

		if (GetMessage(&message, messageOnlyWindowHandle, UWM_OLD_CPU_SIMULATOR_PROCESS_SYNC_WAIT, UWM_OLD_CPU_SIMULATOR_PROCESS_SYNC_WAIT) == -1) {
			return false;
		}
		*/
		if (!timerID) {
			return false;
		}

		if (WaitForSingleObject(timeEvent, s) != WAIT_OBJECT_0) {
			if (timeKillEvent(timerID) != TIMERR_NOERROR) {
				if (!ResetEvent(timeEvent)) {
					return false;
				}
			}
		}
		return true;
		//extra = (extraMs > waitMs) ? (double)extraMs / waitMs : 1.0;
		//extraMs = timeGetTime();
		//return true;
	}

	inline bool ProcessSync::suspendWait() {
		//resumeExtraMs = timeGetTime() - resumeExtraMs;
		return wait(timeSetEvent(suspendWaitMs/*clamp(round(extra * suspendWaitMs), ms, s)*/, 0, (LPTIMECALLBACK)timeEvent, 0, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET)/*, suspendExtraMs, suspendWaitMs*/);
	}

	inline bool ProcessSync::resumeWait() {
		//suspendExtraMs = timeGetTime() - suspendExtraMs;
		return wait(timeSetEvent(resumeWaitMs/*clamp(round(extra * resumeWaitMs), ms, s)*/, 0, (LPTIMECALLBACK)timeEvent, 0, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET)/*, resumeExtraMs, resumeWaitMs*/);
	}

	// returns true if the thread is nonsignaled
	// returns false if the thread has been terminated
	// suspended is set to true if the thread gets suspended, false otherwise
	inline bool ProcessSync::suspendThread() {
		if (suspended) {
			// the thread is already suspended, it needs to be resumed instead
			return true;
		}

		if (SuspendThread(syncedThread) != -1) {
			// the thread is nonsignaled and was successfully suspended
			suspended = true;
			return true;
		}

		if (WaitForSingleObject(syncedThread, 0) != WAIT_OBJECT_0) {
			// the thread is nonsignaled but failed to be suspended
			return true;
		}
		// the thread is signaled
		return false;
	}

	inline bool ProcessSync::resumeThread() {
		if (!suspended) {
			// the thread is already resumed, it needs to be suspended instead
			return true;
		}

		if (ResumeThread(syncedThread) != -1) {
			// the thread is nonsignaled and was successfully resumed
			suspended = false;
			return true;
		}

		if (WaitForSingleObject(syncedThread, 0) != WAIT_OBJECT_0) {
			// the thread is nonsignaled but failed to be resumed
			return true;
		}
		// the thread is signaled
		return false;
	}

	inline bool ProcessSync::suspendProcess() {
		if (suspended) {
			// the process is already suspended, it needs to be resumed instead
			return true;
		}

		if (ntSuspendProcess(syncedProcess) == STATUS_SUCCESS) {
			// the process is nonsignaled and was successfully suspended
			suspended = true;
			return true;
		}

		if (WaitForSingleObject(syncedProcess, 0) != WAIT_OBJECT_0) {
			// the process is nonsignaled but failed to be suspended
			return true;
		}
		// the process is signaled
		return false;
	}

	inline bool ProcessSync::resumeProcess() {
		if (!suspended) {
			// the process is already resumed, it needs to be suspended instead
			return true;
		}

		if (ntResumeProcess(syncedProcess) == STATUS_SUCCESS) {
			// the process is nonsignaled and was successfully suspended
			suspended = false;
			return true;
		}

		if (WaitForSingleObject(syncedProcess, 0) != WAIT_OBJECT_0) {
			// the process is nonsignaled but failed to be resumed
			return true;
		}
		// the process is signaled
		return false;
	}

	inline void ProcessSync::allocateSystemInformation() {
		SYSTEM_INFO systemInfo = {};
		GetSystemInfo(&systemInfo);

		SIZE_T pageSize = systemInfo.dwPageSize;

		if (!pageSize) {
			throw "Failed to Get Page Size";
		}

		if (systemInformation) {
			if (systemInformationSize == pageSize) {
				return;
			}

			delete[] systemInformation;
			systemInformation = NULL;
			systemInformationSize = 0;
		}

		systemInformationSize = pageSize;
		systemInformation = new BYTE[systemInformationSize];

		if (!systemInformation) {
			throw "Failed to Allocate systemInformation";
		}
	}

	inline bool ProcessSync::querySystemInformation() {
		ULONG returnSize = 0;
		NTSTATUS ntStatus = ntQuerySystemInformation(SystemProcessInformation, systemInformation, systemInformationSize, &returnSize);

		while (ntStatus == STATUS_INFO_LENGTH_MISMATCH) {
			// if the buffer wasn't large enough, increase the size
			delete[] systemInformation;
			systemInformation = NULL;

			systemInformationSize += systemInformationSize;
			systemInformation = new BYTE[systemInformationSize];

			if (!systemInformation) {
				throw "Failed to Allocate systemInformation";
			}

			ntStatus = ntQuerySystemInformation(SystemProcessInformation, systemInformation, systemInformationSize, &returnSize);
		}

		const size_t SYSTEM_PROCESS_INFORMATION_SIZE = sizeof(__SYSTEM_PROCESS_INFORMATION);

		if (ntStatus != STATUS_SUCCESS || returnSize < SYSTEM_PROCESS_INFORMATION_SIZE) {
			delete[] systemInformation;
			systemInformation = NULL;
			systemInformationSize = 0;
			throw "Failed to Query System Information";
		}

		SIZE_T systemInformationEndIndex = (SIZE_T)systemInformation + systemInformationSize;

		ULONG nextEntryOffset = 0;

		__PSYSTEM_PROCESS_INFORMATION systemProcessInformationPointer = (__PSYSTEM_PROCESS_INFORMATION)systemInformation;
		PSYSTEM_THREAD_INFORMATION systemThreadInformationPointer = NULL;

		DWORD threadID = 0;
		HANDLE thread = NULL;

		SUSPENDED_THREADS_VECTOR _suspendedThreadsVector = suspendedThreadsVector;
		suspendedThreadsVector = {};

		SUSPENDED_THREADS_MAP::iterator suspendedThreadsMapIterator = {};

		do {
			if ((DWORD)systemProcessInformationPointer->UniqueProcessId == syncedProcessID) {
				systemThreadInformationPointer = (PSYSTEM_THREAD_INFORMATION)(systemProcessInformationPointer + 1);

				if (!systemThreadInformationPointer) {
					throw "systemThreadInformationPointer must not be NULL";
				}

				for (ULONG i = 0; i < systemProcessInformationPointer->NumberOfThreads; i++) {
					threadID = (DWORD)systemThreadInformationPointer->ClientId.UniqueThread;

					suspendedThreadsMapIterator = suspendedThreadsMap.find(threadID);

					if (suspendedThreadsMapIterator == suspendedThreadsMap.end()) {
						thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadID);

						if (thread
							&& SuspendThread(thread) != -1) {
							suspendedThreadsVector.push_back(threadID);
							suspendedThreadsMap[threadID] = thread;
						}
					}
					systemThreadInformationPointer++;
				}

				suspendedThreadsVector.insert(suspendedThreadsVector.end(), _suspendedThreadsVector.begin(), _suspendedThreadsVector.end());

				systemProcessInformationPointer = NULL;
				systemThreadInformationPointer = NULL;
				return true;
			}

			nextEntryOffset = systemProcessInformationPointer->NextEntryOffset;

			if (!nextEntryOffset) {
				break;
			}

			systemProcessInformationPointer = (__PSYSTEM_PROCESS_INFORMATION)((PBYTE)systemProcessInformationPointer + nextEntryOffset);
		} while ((SIZE_T)systemProcessInformationPointer < systemInformationEndIndex);

		systemProcessInformationPointer = NULL;
		systemThreadInformationPointer = NULL;
		delete[] systemInformation;
		systemInformation = NULL;
		systemInformationSize = 0;
		return false;
	}

	inline bool ProcessSync::_toolhelpSnapshot_snapshotHandle(HANDLE snapshot) {
		THREADENTRY32 threadEntry = {};
		threadEntry.dwSize = sizeof(threadEntry);

		DWORD lastError = 0;

		if (!Thread32First(snapshot, &threadEntry)) {
			lastError = GetLastError();

			if (lastError != ERROR_SUCCESS && lastError != ERROR_NO_MORE_FILES) {
				throw "Failed at First Thread";
			}
			return false;
		}

		DWORD threadID = 0;
		HANDLE thread = NULL;

		SUSPENDED_THREADS_VECTOR _suspendedThreadsVector = suspendedThreadsVector;
		suspendedThreadsVector = {};

		SUSPENDED_THREADS_MAP::iterator suspendedThreadsMapIterator = {};

		do {
			if (threadEntry.th32OwnerProcessID == syncedProcessID) {
				threadID = threadEntry.th32ThreadID;

				suspendedThreadsMapIterator = suspendedThreadsMap.find(threadID);

				if (suspendedThreadsMapIterator == suspendedThreadsMap.end()) {
					thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadID);

					if (thread
						&& SuspendThread(thread) != -1) {
						suspendedThreadsVector.push_back(threadID);
						suspendedThreadsMap[threadID] = thread;
					}
				}
			}
		} while (Thread32Next(snapshot, &threadEntry));

		suspendedThreadsVector.insert(suspendedThreadsVector.end(), _suspendedThreadsVector.begin(), _suspendedThreadsVector.end());

		lastError = GetLastError();

		if (lastError != ERROR_SUCCESS && lastError != ERROR_NO_MORE_FILES) {
			throw "Failed at Next Thread";
		}

		if (!threadID) {
			return false;
		}
		return true;
	}

	inline bool ProcessSync::toolhelpSnapshot() {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

		if (!snapshot || snapshot == INVALID_HANDLE_VALUE) {
			throw "Failed to Create Toolhelp Snapshot";
		}

		__try {
			return _toolhelpSnapshot_snapshotHandle(snapshot);
		} __finally {
			if (!CloseHandle(snapshot)) {
				throw "Failed to Close Handle";
			}

			snapshot = NULL;
		}
		return false;
	}

	inline void ProcessSync::closeResumedThreads() {
		HANDLE thread = NULL;

		RESUMED_THREADS_VECTOR::iterator resumedThreadsVectorIterator = resumedThreadsVector.begin();

		while (resumedThreadsVectorIterator != resumedThreadsVector.end()) {
			thread = *resumedThreadsVectorIterator;

			if (!thread
				|| CloseHandle(thread)) {
				resumedThreadsVectorIterator = resumedThreadsVector.erase(resumedThreadsVectorIterator);
			} else {
				resumedThreadsVectorIterator++;
			}
		}
	}

	inline void ProcessSync::resumeThreads() {
		closeResumedThreads();

		DWORD threadID = 0;
		HANDLE thread = NULL;

		// we have to loop in the reverse order we suspended the threads
		// we can't use reverse_iterator because we need to erase elements mid-loop
		SUSPENDED_THREADS_VECTOR::iterator suspendedThreadsVectorIterator = suspendedThreadsVector.end();
		SUSPENDED_THREADS_MAP::iterator suspendedThreadsMapIterator = {};

		while (suspendedThreadsVectorIterator != suspendedThreadsVector.begin()) {
			threadID = *--suspendedThreadsVectorIterator;

			suspendedThreadsMapIterator = suspendedThreadsMap.find(threadID);

			if (suspendedThreadsMapIterator == suspendedThreadsMap.end()) {
				suspendedThreadsVectorIterator = suspendedThreadsVector.erase(suspendedThreadsVectorIterator);
			} else {
				thread = suspendedThreadsMapIterator->second;

				if (thread) {
					if (ResumeThread(thread) == -1
						&& WaitForSingleObject(thread, 0) != WAIT_OBJECT_0) {
						continue;
					}

					if (!CloseHandle(thread)) {
						resumedThreadsVector.push_back(thread);
					}
				}

				suspendedThreadsVectorIterator = suspendedThreadsVector.erase(suspendedThreadsVectorIterator);
				suspendedThreadsMap.erase(suspendedThreadsMapIterator);
			}
		}
	}
	public:
	ProcessSync(bool setProcessPriorityHigh, bool syncedProcessMainThreadOnly, bool setSyncedProcessAffinityOne, bool refreshHzFloorFifteen);
	~ProcessSync();
	ProcessSync(const ProcessSync &processSync);
	ProcessSync &operator=(const ProcessSync &processSync);
	bool open(std::string commandLine);
	bool close();
	bool run(SYNC_MODE syncMode, ULONG mhzLimit, ULONG targetMhz, UINT refreshHz);
};