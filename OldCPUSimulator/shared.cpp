#include "shared.h"
#include <iostream>
#include <string>
#include <regex>
#define WIN32_NO_STATUS
#include <windows.h>
#include <winternl.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>

extern "C" {
	#include <PowrProf.h>
}

#define SHARED_OUT true, 2
#define SHARED_ERR true, 2, true, __FILE__, __LINE__

/*
void CALLBACK TimeProc(UINT id, UINT msg, DWORD user, DWORD one, DWORD two) {
	if (user) {
		PostMessage((HWND)user, UWM_OLD_CPU_SIMULATOR_PROCESS_SYNC_WAIT, NULL, NULL);
	}
}
*/

void consoleLog(const char* str, short newline, short tab, bool err, const char* file, unsigned int line) {
	if (!str) {
		str = "";
	}

	if (err) {
		for (short i = 0; i < tab; i++) {
			std::cerr << "\t";
		}

		std::cerr << str;

		if (line || !stringNullOrEmpty(file)) {
			if (!file) {
				file = "";
			}

			std::cerr << " [" << file << ":" << line << "]";
		}

		std::cerr << " (" << GetLastError() << ")";

		for (short i = 0; i < newline; i++) {
			std::cerr << std::endl;
		}
		return;
	}

	for (short i = 0; i < tab; i++) {
		std::cout << "\t";
	}

	std::cout << str;

	if (line || !stringNullOrEmpty(file)) {
		if (!file) {
			file = "";
		}

		std::cout << " [" << file << ":" << line << "]";
	}

	for (short i = 0; i < newline; i++) {
		std::cout << std::endl;
	}
}

bool getMhzLimit(ULONG &mhzLimit) {
	//consoleLog("Getting Rate Limit", SHARED_OUT);

	mhzLimit = 0;

	SYSTEM_INFO systemInfo = {};
	GetSystemInfo(&systemInfo);

	size_t PROCESSOR_POWER_INFORMATION_SIZE = sizeof(PROCESSOR_POWER_INFORMATION) * systemInfo.dwNumberOfProcessors;
	PVOID outputBuffer = new BYTE[PROCESSOR_POWER_INFORMATION_SIZE];

	if (!outputBuffer) {
		consoleLog("Failed to Allocate outputBuffer", SHARED_ERR);
		return false;
	}

	bool result = false;

	if (CallNtPowerInformation(ProcessorInformation, NULL, NULL, outputBuffer, PROCESSOR_POWER_INFORMATION_SIZE) != STATUS_SUCCESS) {
		consoleLog("Failed to Call Power Information", SHARED_ERR);
		goto error;
	}

	PPROCESSOR_POWER_INFORMATION processorPowerInformationPointer = (PPROCESSOR_POWER_INFORMATION)outputBuffer;

	for (DWORD i = 0; i < systemInfo.dwNumberOfProcessors; i++) {
		mhzLimit = max(processorPowerInformationPointer->MhzLimit ? processorPowerInformationPointer->MhzLimit : processorPowerInformationPointer->MaxMhz, mhzLimit);
		processorPowerInformationPointer++;
	}

	result = true;
	processorPowerInformationPointer = NULL;
	error:
	delete[] outputBuffer;
	outputBuffer = NULL;
	return result;
}

bool setProcessAffinity(HANDLE process, DWORD affinity) {
	consoleLog("Setting Process Affinity", SHARED_OUT);

	if (!process) {
		consoleLog("process must not be NULL", SHARED_ERR);
		return false;
	}

	DWORD_PTR processAffinityMask = 0;
	DWORD_PTR systemAffinityMask = 0;

	if (!GetProcessAffinityMask(process, &processAffinityMask, &systemAffinityMask)) {
		consoleLog("Failed to Get Process Affinity Mask", SHARED_ERR);
		return false;
	}

	processAffinityMask = 0;
	DWORD_PTR processAffinity = 0;

	for (DWORD_PTR i = 1; i; i <<= 1) {
		if (systemAffinityMask & i) {
			processAffinityMask |= i;
			processAffinity++;

			if (processAffinity == affinity) {
				break;
			}
		}
	}

	if (!SetProcessAffinityMask(process, processAffinityMask)) {
		consoleLog("Failed to Set Process Affinity Mask", SHARED_ERR);
		return false;
	}
	return true;
}

bool honorTimerResolutionRequests(HANDLE process, SetProcessInformationProc setProcessInformation) {
	consoleLog("Honoring Timer Resolution Requests", SHARED_OUT);

	if (!process) {
		consoleLog("process must not be NULL", SHARED_ERR);
		return false;
	}

	if (!setProcessInformation) {
		consoleLog("setProcessInformation must not be NULL", SHARED_ERR);
		return false;
	}

	// Windows 11 timer throttling behaviour
	PROCESS_POWER_THROTTLING_STATE processPowerThrottlingState = {};
	processPowerThrottlingState.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
	processPowerThrottlingState.ControlMask = PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
	processPowerThrottlingState.StateMask = 0;

	const DWORD PROCESS_POWER_THROTTLING_STATE_SIZE = sizeof(processPowerThrottlingState);

	if (!setProcessInformation(process, ProcessPowerThrottling, &processPowerThrottlingState, PROCESS_POWER_THROTTLING_STATE_SIZE)) {
		if (GetLastError() != ERROR_INVALID_PARAMETER) {
			consoleLog("Failed to Set Process Information", SHARED_ERR);
			return false;
		}
	}
	return true;
}

bool getArgumentFromCommandLine(std::string commandLine, std::string &argument) {
	std::regex commandLineQuotes("^\\s*\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"?\\s*");
	std::regex commandLineWords("^\\s*\\S+\\s*");
	std::smatch matchResults = {};
	bool match = std::regex_search(commandLine, matchResults, commandLineQuotes);

	if (match && matchResults.length() > 0) {
		argument = matchResults[0];
		return true;
	} else {
		matchResults = {};
		match = std::regex_search(commandLine, matchResults, commandLineWords);

		if (match && matchResults.length() > 0) {
			argument = matchResults[0];
			return true;
		}
	}
	return false;
}

std::string getArgumentRangeFromCommandLine(std::string commandLine, int begin, int end) {
	std::vector<std::string>::iterator argumentsIterator;
	std::vector<std::string> arguments;
	std::string argument = "";
	std::string argumentRange = "";

	while (getArgumentFromCommandLine(commandLine, argument)) {
		arguments.push_back(argument);
		commandLine = commandLine.substr(argument.length());
	}

	int i = 0;

	if (end < 0) {
		end += arguments.size() + 1;
	}

	for (argumentsIterator = arguments.begin(); argumentsIterator != arguments.end(); argumentsIterator++) {
		if (i >= end) {
			break;
		}

		if (i >= begin) {
			argumentRange += *argumentsIterator;
		}

		i++;
	}
	return argumentRange;
}