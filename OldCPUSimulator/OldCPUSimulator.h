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
	std::shared_ptr<BYTE[]> systemInformation = NULL;

	NtSuspendProcessProc ntSuspendProcess = NULL;
	NtResumeProcessProc ntResumeProcess = NULL;
	NtQuerySystemInformationProc ntQuerySystemInformation = NULL;
	
	inline bool OldCPUSimulator::wait(UINT waitMs, UINT s2, HANDLE timeEvent) {
		// the documentation on the matter only alludes to this idea, but
		// TIME_KILL_SYNCHRONOUS doesn't do anything with TIME_CALLBACK_EVENT_SET
		// so removing it allows Windows 2000 compatibility
		UINT timerID = timeSetEvent(waitMs, 0, (LPTIMECALLBACK)timeEvent, 0, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET/* | TIME_KILL_SYNCHRONOUS*/);
		
		if (!timerID) {
			return false;
		}

		if (WaitForSingleObject(timeEvent, s2) != WAIT_OBJECT_0) {
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
			throw std::runtime_error("Failed to Get Page Size");
		}

		if (systemInformation) {
			if (systemInformationSize == pageSize) {
				return;
			}
		}

		systemInformationSize = pageSize;
		systemInformation = std::shared_ptr<BYTE[]>(new BYTE[systemInformationSize]);

		if (!systemInformation) {
			throw std::bad_alloc();
		}
	}

	inline bool OldCPUSimulator::querySystemInformation() {
		if (!systemInformation) {
			throw std::runtime_error("systemInformation must not be NULL");
		}

		if (!systemInformationSize) {
			throw std::runtime_error("systemInformationSize must not be zero");
		}

		ULONG returnSize = 0;
		NTSTATUS ntStatus = ntQuerySystemInformation(SystemProcessInformation, systemInformation.get(), systemInformationSize, &returnSize);

		while (ntStatus == STATUS_INFO_LENGTH_MISMATCH) {
			// if the buffer wasn't large enough, increase the size
			systemInformationSize += systemInformationSize;
			systemInformation = std::shared_ptr<BYTE[]>(new BYTE[systemInformationSize]);

			if (!systemInformation) {
				throw std::bad_alloc();
			}

			ntStatus = ntQuerySystemInformation(SystemProcessInformation, systemInformation.get(), systemInformationSize, &returnSize);
		}

		const size_t SYSTEM_PROCESS_INFORMATION_SIZE = sizeof(__SYSTEM_PROCESS_INFORMATION);

		if (ntStatus != STATUS_SUCCESS || returnSize < SYSTEM_PROCESS_INFORMATION_SIZE || returnSize > systemInformationSize) {
			throw std::runtime_error("Failed to Query System Information");
		}

		SIZE_T returnBeginIndex = (SIZE_T)systemInformation.get();
		SIZE_T returnEndIndex = returnBeginIndex + returnSize;

		ULONG nextEntryOffset = 0;

		__PSYSTEM_PROCESS_INFORMATION systemProcessInformationPointer = (__PSYSTEM_PROCESS_INFORMATION)systemInformation.get();
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
					throw std::runtime_error("systemThreadInformationPointer must not be NULL");
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
				return true;
			}

			nextEntryOffset = systemProcessInformationPointer->NextEntryOffset;

			if (!nextEntryOffset) {
				return false;
			}

			if ((SIZE_T)systemProcessInformationPointer + nextEntryOffset < (SIZE_T)systemProcessInformationPointer) {
				return false;
			}

			systemProcessInformationPointer = (__PSYSTEM_PROCESS_INFORMATION)((PBYTE)systemProcessInformationPointer + nextEntryOffset);
		} while ((SIZE_T)systemProcessInformationPointer >= returnBeginIndex && (SIZE_T)systemProcessInformationPointer + SYSTEM_PROCESS_INFORMATION_SIZE <= returnEndIndex);
		return false;
	}

	inline bool OldCPUSimulator::_toolhelpSnapshot_snapshotHandle(HANDLE snapshot) {
		THREADENTRY32 threadEntry = {};
		threadEntry.dwSize = sizeof(threadEntry);

		DWORD lastError = 0;

		if (!Thread32First(snapshot, &threadEntry)) {
			lastError = GetLastError();

			if (lastError != ERROR_SUCCESS && lastError != ERROR_NO_MORE_FILES) {
				throw std::runtime_error("Failed at First Thread");
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
			throw std::runtime_error("Failed at Next Thread");
		}

		if (!threadID) {
			return false;
		}
		return true;
	}

	inline bool OldCPUSimulator::toolhelpSnapshot() {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

		if (!snapshot || snapshot == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Failed to Create Toolhelp Snapshot");
		}

		__try {
			return _toolhelpSnapshot_snapshotHandle(snapshot);
		} __finally {
			if (!CloseHandle(snapshot)) {
				throw std::runtime_error("Failed to Close Handle");
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
	bool run(SYNC_MODE syncMode, ULONG maxMhz, ULONG targetMhz, UINT refreshHz);
};