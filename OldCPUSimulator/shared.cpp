#include "shared.h"
#include <iostream>
#include <regex>
#include <ntstatus.h>

extern "C" {
	#include <PowrProf.h>
}

#define SHARED_OUT true, 2
#define SHARED_ERR true, 2, true, __FILE__, __LINE__

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

bool getMaxMhz(ULONG &maxMhz) {
	//consoleLog("Getting Max Rate", SHARED_OUT);

	maxMhz = 0;

	SYSTEM_INFO systemInfo = {};
	GetSystemInfo(&systemInfo);

	ULONG processorPowerInformationSize = sizeof(__PROCESSOR_POWER_INFORMATION) * systemInfo.dwNumberOfProcessors;

	std::unique_ptr<BYTE[]> outputBuffer = std::unique_ptr<BYTE[]>(new BYTE[processorPowerInformationSize]);

	if (!outputBuffer) {
		consoleLog("Failed to Allocate outputBuffer", SHARED_ERR);
		return false;
	}

	if (CallNtPowerInformation(ProcessorInformation, NULL, NULL, outputBuffer.get(), processorPowerInformationSize) != STATUS_SUCCESS) {
		consoleLog("Failed to Call Power Information", SHARED_ERR);
		return false;
	}

	__PPROCESSOR_POWER_INFORMATION processorPowerInformationPointer = (__PPROCESSOR_POWER_INFORMATION)outputBuffer.get();

	if (!processorPowerInformationPointer) {
		consoleLog("processorPowerInformationPointer must not be NULL", SHARED_ERR);
		return false;
	}

	for (DWORD i = 0; i < systemInfo.dwNumberOfProcessors; i++) {
		maxMhz = max(processorPowerInformationPointer->MaxMhz, maxMhz);
		processorPowerInformationPointer++;
	}
	return true;
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

	for (DWORD_PTR i = 1; i && processAffinity < affinity; i <<= 1) {
		if (systemAffinityMask & i) {
			processAffinityMask |= i;
			processAffinity++;
		}
	}

	if (processAffinity != affinity) {
		consoleLog("Invalid Affinity", SHARED_ERR);
		return false;
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
	__PROCESS_POWER_THROTTLING_STATE processPowerThrottlingState = {};
	processPowerThrottlingState.Version = _PROCESS_POWER_THROTTLING_CURRENT_VERSION;
	processPowerThrottlingState.ControlMask = _PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
	processPowerThrottlingState.StateMask = 0;

	const DWORD PROCESS_POWER_THROTTLING_STATE_SIZE = sizeof(processPowerThrottlingState);

	if (!setProcessInformation(process, _ProcessPowerThrottling, &processPowerThrottlingState, PROCESS_POWER_THROTTLING_STATE_SIZE)) {
		if (GetLastError() != ERROR_INVALID_PARAMETER) {
			consoleLog("Failed to Set Process Information", SHARED_ERR);
			return false;
		}
	}
	return true;
}

std::string getArgumentSliceFromCommandLine(std::string commandLine, int begin, int end) {
	std::vector<std::string> arguments = {};

	{
		const std::regex COMMAND_LINE_ARGUMENTS("^\\s*(?:\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"?|(?:[^\"\\\\\\s]+|\\\\\\S)+|\\\\|\\s+$)+\\s?");

		std::smatch matches = {};

		while (std::regex_search(commandLine, matches, COMMAND_LINE_ARGUMENTS)
			&& matches.length() > 0) {
			arguments.push_back(matches[0]);
			commandLine = matches.suffix();
		}
	}

	int argumentsSize = (int)(arguments.size() + 1);

	if (begin < 0) {
		begin += argumentsSize;
	}

	begin = max(0, begin);

	if (end < 0) {
		end += argumentsSize;
	}

	argumentsSize--;

	end = min(argumentsSize, end);

	std::string argumentSlice = "";

	for (int i = begin; i < end; i++) {
		argumentSlice += arguments[i];
	}
	return argumentSlice;
}