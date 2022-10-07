#pragma once
#include "shared.h"
#include <vector>
#include <map>
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <TlHelp32.h>

class OldCPUSimulator {
	private:
	void destroy();
	bool duplicate(const OldCPUSimulator &oldCPUSimulator);

	bool opened = false;

	bool setProcessPriorityHigh = false;
	bool syncedProcessMainThreadOnly = false;
	bool setSyncedProcessAffinityOne = false;
	bool refreshHzFloorFifteen = false;

	DWORD syncedProcessID = 0;
	DWORD syncedThreadID = 0;

	HANDLE syncedProcess = NULL;
	HANDLE syncedThread = NULL;

	HANDLE jobObject = NULL;

	bool suspended = false;

	// I tested it in profiler, unordered_map isn't faster
	typedef std::vector<DWORD> SUSPENDED_THREAD_IDS_VECTOR;
	typedef std::map<DWORD, HANDLE> SUSPENDED_THREAD_IDS_MAP;

	SUSPENDED_THREAD_IDS_VECTOR suspendedThreadIDsVector = {};
	SUSPENDED_THREAD_IDS_MAP suspendedThreadIDsMap = {};

	typedef std::vector<HANDLE> RESUMED_THREADS_VECTOR;
	RESUMED_THREADS_VECTOR resumedThreadsVector = {};

	SetProcessInformationProc setProcessInformation = NULL;

	SIZE_T systemInformationSize = 0;
	PVOID systemInformation = NULL;

	NtSuspendProcessProc ntSuspendProcess = NULL;
	NtResumeProcessProc ntResumeProcess = NULL;
	NtQuerySystemInformationProc ntQuerySystemInformation = NULL;
	
	inline bool OldCPUSimulator::wait(UINT waitMs, UINT refreshMs, HANDLE timeEvent) {
		UINT timerID = timeSetEvent(waitMs, 0, (LPTIMECALLBACK)timeEvent, 0, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET);
		
		if (!timerID) {
			return false;
		}

		if (WaitForSingleObject(timeEvent, refreshMs) != WAIT_OBJECT_0) {
			// if killing the time event succeeds,
			// we successfully prevent it from going off
			// so the event still isn't signaled
			if (timeKillEvent(timerID) != TIMERR_NOERROR) {
				// if killing the time event failed,
				// the time event went off after WaitForSingleObject returned,
				// so it won't have been auto reset
				// we have to reset the event now
				if (!ResetEvent(timeEvent)) {
					return false;
				}
			}
		}
		return true;
	}

	// returns true if the thread is nonsignaled
	// returns false if the thread has been terminated
	// suspended is set to true if the thread gets suspended, false otherwise
	inline bool OldCPUSimulator::suspendThread() {
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

	inline bool OldCPUSimulator::resumeThread() {
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

	inline bool OldCPUSimulator::suspendProcess() {
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

	inline bool OldCPUSimulator::resumeProcess() {
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

	inline void OldCPUSimulator::allocateSystemInformation() {
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

	inline bool OldCPUSimulator::querySystemInformation() {
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

		SUSPENDED_THREAD_IDS_VECTOR _suspendedThreadIDsVector = suspendedThreadIDsVector;
		suspendedThreadIDsVector = {};

		SUSPENDED_THREAD_IDS_MAP::iterator suspendedThreadIDsMapIterator = {};

		do {
			if ((DWORD)systemProcessInformationPointer->UniqueProcessId == syncedProcessID) {
				systemThreadInformationPointer = (PSYSTEM_THREAD_INFORMATION)(systemProcessInformationPointer + 1);

				if (!systemThreadInformationPointer) {
					throw "systemThreadInformationPointer must not be NULL";
				}

				for (ULONG i = 0; i < systemProcessInformationPointer->NumberOfThreads; i++) {
					threadID = (DWORD)systemThreadInformationPointer->ClientId.UniqueThread;

					suspendedThreadIDsMapIterator = suspendedThreadIDsMap.find(threadID);

					if (suspendedThreadIDsMapIterator == suspendedThreadIDsMap.end()) {
						thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadID);

						if (thread
							&& SuspendThread(thread) != -1) {
							suspendedThreadIDsVector.push_back(threadID);
							suspendedThreadIDsMap[threadID] = thread;
						}
					}
					systemThreadInformationPointer++;
				}

				suspendedThreadIDsVector.insert(suspendedThreadIDsVector.end(), _suspendedThreadIDsVector.begin(), _suspendedThreadIDsVector.end());

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

	inline bool OldCPUSimulator::_toolhelpSnapshot_snapshotHandle(HANDLE snapshot) {
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

		SUSPENDED_THREAD_IDS_VECTOR _suspendedThreadIDsVector = suspendedThreadIDsVector;
		suspendedThreadIDsVector = {};

		SUSPENDED_THREAD_IDS_MAP::iterator suspendedThreadIDsMapIterator = {};

		do {
			if (threadEntry.th32OwnerProcessID == syncedProcessID) {
				threadID = threadEntry.th32ThreadID;

				suspendedThreadIDsMapIterator = suspendedThreadIDsMap.find(threadID);

				if (suspendedThreadIDsMapIterator == suspendedThreadIDsMap.end()) {
					thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadID);

					if (thread
						&& SuspendThread(thread) != -1) {
						suspendedThreadIDsVector.push_back(threadID);
						suspendedThreadIDsMap[threadID] = thread;
					}
				}
			}
		} while (Thread32Next(snapshot, &threadEntry));

		suspendedThreadIDsVector.insert(suspendedThreadIDsVector.end(), _suspendedThreadIDsVector.begin(), _suspendedThreadIDsVector.end());

		lastError = GetLastError();

		if (lastError != ERROR_SUCCESS && lastError != ERROR_NO_MORE_FILES) {
			throw "Failed at Next Thread";
		}

		if (!threadID) {
			return false;
		}
		return true;
	}

	inline bool OldCPUSimulator::toolhelpSnapshot() {
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

	inline void OldCPUSimulator::closeResumedThreads() {
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

	inline void OldCPUSimulator::resumeThreads() {
		closeResumedThreads();

		DWORD threadID = 0;
		HANDLE thread = NULL;

		// we have to loop in the reverse order we suspended the threads
		// we can't use reverse_iterator because we need to erase elements mid-loop
		SUSPENDED_THREAD_IDS_VECTOR::iterator suspendedThreadIDsVectorIterator = suspendedThreadIDsVector.end();
		SUSPENDED_THREAD_IDS_MAP::iterator suspendedThreadIDsMapIterator = {};

		while (suspendedThreadIDsVectorIterator != suspendedThreadIDsVector.begin()) {
			threadID = *--suspendedThreadIDsVectorIterator;

			suspendedThreadIDsMapIterator = suspendedThreadIDsMap.find(threadID);

			if (suspendedThreadIDsMapIterator == suspendedThreadIDsMap.end()) {
				suspendedThreadIDsVectorIterator = suspendedThreadIDsVector.erase(suspendedThreadIDsVectorIterator);
			} else {
				thread = suspendedThreadIDsMapIterator->second;

				if (thread) {
					if (ResumeThread(thread) == -1
						&& WaitForSingleObject(thread, 0) != WAIT_OBJECT_0) {
						continue;
					}

					if (!CloseHandle(thread)) {
						resumedThreadsVector.push_back(thread);
					}
				}

				suspendedThreadIDsVectorIterator = suspendedThreadIDsVector.erase(suspendedThreadIDsVectorIterator);
				suspendedThreadIDsMap.erase(suspendedThreadIDsMapIterator);
			}
		}
	}

	public:
	OldCPUSimulator(bool setProcessPriorityHigh, bool syncedProcessMainThreadOnly, bool setSyncedProcessAffinityOne, bool refreshHzFloorFifteen);
	~OldCPUSimulator();
	OldCPUSimulator(const OldCPUSimulator &oldCPUSimulator);
	OldCPUSimulator &operator=(const OldCPUSimulator &oldCPUSimulator);
	bool open(std::string commandLine);
	bool close();
	bool run(SYNC_MODE syncMode, ULONG mhzLimit, ULONG targetMhz, UINT refreshHz);
};