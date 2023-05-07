#include "shared.h"
#include "OldCPUSimulator.h"
#include <string>
#include <windows.h>

#define MAIN_OUT 2
#define MAIN_ERR true, false, true, __FILE__, __LINE__

void help() {
	consoleLog("This command line tool simulates running a process on a CPU with a", true, true);
	consoleLog("slower clock speed in order to make old games run at the correct speed", true, true);
	consoleLog("or underclock CPU intensive processes like video encoding.", 3, true);


	consoleLog("Usage: OldCPUSimulator -t targetRate [options] -sw software.exe", 2);

	consoleLog("-t targetRate (or --target-rate targetRate)");
	consoleLog("The Target Rate (in MHz, from 1 to your CPU's current clock speed)", true, true);
	consoleLog("to simulate.", true, true);
	consoleLog("This argument is required.", 2, true);

	consoleLog("Try 233 to simulate an Intel Pentium 233 MHz from the late 1990s.", true, true);
	consoleLog("Try 350 to simulate an Intel Pentium II 350 MHz from the early 2000s.", true, true);
	consoleLog("Try 933 to simulate an Intel Pentium III 933 MHz from the mid 2000s.", 2, true);

	consoleLog("Go to http://intel.com/pressroom/kits/quickrefyr.htm", true, true);
	consoleLog("for a quick reference of year to clock speed.", true, true);
	consoleLog("Note that many of the measurements in said reference", true, true);
	consoleLog("are in GHz. This tool uses MHz.", 2, true);

	consoleLog("-r refreshRate (or --refresh-rate refreshRate)");
	consoleLog("The Refresh Rate (in Hz) at which to refresh.", true, true);
	consoleLog("This argument is not required.", 2, true);

	consoleLog("Effectively an accuracy meter.", true, true);
	consoleLog("Lower numbers are more accurate but result in choppier playback.", true, true);
	consoleLog("Higher numbers are less accurate but result in smoother playback.", true, true);
	consoleLog("If not specified, Old CPU Simulator will default to the", true, true);
	consoleLog("smoothest possible playback setting.", 2, true);

	consoleLog("Try 60, 30 or 15 for gaming, 1 to 6 for video encoding.", 2, true);

	consoleLog("-ph (or --set-process-priority-high)");
	consoleLog("Set the process priority of Old CPU Simulator to High,", true, true);
	consoleLog("in order to potentially improve the accuracy of the simulation.", 2, true);

	consoleLog("-a1 (or --set-synced-process-affinity-one)");
	consoleLog("Set the process affinity of the synced process", true, true);
	consoleLog("to one, which may make the speed more consistent", true, true);
	consoleLog("and prevent crashes.", true, true);
	consoleLog("May not work with newer games.", 2, true);

	consoleLog("-mt (or --synced-process-main-thread-only)");
	consoleLog("This is an optimization which improves the accuracy of the", true, true);
	consoleLog("simulation, but may not work well with multithreaded software.", 2, true);

	consoleLog("-rf (or --refresh-rate-floor-fifteen)");
	consoleLog("Rounds Refresh Rate to the nearest multiple of 15 if applicable.", 2, true);

	consoleLog("-sw software.exe (or --software software.exe)");
	consoleLog("The software that will be created as the synced process.", true, true);
	consoleLog("This argument is required.", 2, true);

	consoleLog("It must be given last, after all the options.", true, true);
	consoleLog("Command line arguments may also be specified,", true, true);
	consoleLog("which will be passed to the software.", 2, true);
}

int main(int argc, char** argv) {
	HANDLE applicationMutex = CreateMutex(NULL, TRUE, TEXT("Old CPU Simulator"));

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		if (!closeMutex(applicationMutex)) {
			consoleLog("Failed to Close Mutex", MAIN_ERR);
		}
		return -2;
	}

	int result = -1;

	SCOPE_EXIT {
		if (!releaseMutex(applicationMutex)) {
			consoleLog("Failed to Release Mutex", MAIN_ERR);
			result = -2;
		}
	};

	consoleLog("Old CPU Simulator 2.2.0");
	consoleLog("By Anthony Kleine", 2);

	// the number of required arguments
	// including the executable name, even if we don't use it
	const int MIN_ARGC = 1;

	if (argc < MIN_ARGC) {
		help();
		return result;
	}

	const int MIN_ARGS_REQUIRED = 2;

	std::string arg = "";
	int argc2 = argc - 1;
	int argsRequired = 0;

	bool setProcessPriorityHigh = false;
	bool setSyncedProcessAffinityOne = false;
	bool syncedProcessMainThreadOnly = false;
	bool refreshHzFloorFifteen = false;

	std::string software = "";

	SYNC_MODE syncMode = SYNC_MODE_SUSPEND_PROCESS;
	ULONG maxMhz = 0;
	ULONG targetMhz = 233;
	UINT refreshHz = 1000;

	for (int i = MIN_ARGC; i < argc; i++) {
		arg = std::string(argv[i]);

		if (arg == "-h" || arg == "--help") {
			help();
			result = 0;
			return result;
		} else if (arg == "-ph" || arg == "--set-process-priority-high") {
			setProcessPriorityHigh = true;
		} else if (arg == "-a1" || arg == "--set-synced-process-affinity-one") {
			setSyncedProcessAffinityOne = true;
		} else if (arg == "-mt" || arg == "--synced-process-main-thread-only") {
			syncedProcessMainThreadOnly = true;
		} else if (arg == "-rf" || arg == "--refresh-rate-floor-fifteen") {
			refreshHzFloorFifteen = true;
		} else if (arg == "-sw" || arg == "--software") {
			software = getArgumentSliceFromCommandLine(GetCommandLine(), i + 1);
			argsRequired++;
			break;
		} else if (arg == "--dev-force-sync-mode-suspend-process") {
			syncMode = SYNC_MODE_SUSPEND_PROCESS;
		} else if (arg == "--dev-force-sync-mode-query-system-information") {
			syncMode = SYNC_MODE_QUERY_SYSTEM_INFORMATION;
		} else if (arg == "--dev-force-sync-mode-toolhelp-snapshot") {
			syncMode = SYNC_MODE_TOOLHELP_SNAPSHOT;
		} else if (arg == "--dev-get-max-mhz") {
			if (!getMaxMhz(maxMhz)
				|| !maxMhz) {
				consoleLog("Failed to Get Max Rate", MAIN_ERR);
				result = -3;
				return result;
			}

			consoleLog(std::to_string(maxMhz).c_str(), false);
			result = 0;
			return result;
		} else {
			if (i < argc2) {
				if (arg == "-t" || arg == "--target-rate") {
					if (!getMaxMhz(maxMhz)
						|| !maxMhz) {
						consoleLog("Failed to Get Max Rate", MAIN_ERR);
						result = -3;
						return result;
					}

					targetMhz = strtol(argv[++i], NULL, 10);

					if (!targetMhz) {
						consoleLog("Target Rate must be a valid non-zero number", 2);
						help();
						return result;
					}

					/*
					if (maxMhz <= targetMhz) {
						std::ostringstream oStringStream;
						oStringStream << "Target Rate must not exceed or equal the Max Rate of " << maxMhz;

						consoleLog(oStringStream.str().c_str(), 2);
						help();
						return result;
					}
					*/

					argsRequired++;
				} else if (arg == "-r" || arg == "--refresh-rate") {
					refreshHz = strtol(argv[++i], NULL, 10);

					if (!refreshHz) {
						consoleLog("Refresh Rate must be a valid non-zero number", 2);
						help();
						return result;
					}

					//argsRequired++;
				}
			}
		}
	}

	if (argsRequired < MIN_ARGS_REQUIRED) {
		help();
		return result;
	}

	OldCPUSimulator oldCPUSimulator(setProcessPriorityHigh, syncedProcessMainThreadOnly, setSyncedProcessAffinityOne, refreshHzFloorFifteen);

	if (!oldCPUSimulator.open(software)) {
		consoleLog("Failed to Open Old CPU Simulator", MAIN_ERR);
		return result;
	}

	SCOPE_EXIT {
		if (!oldCPUSimulator.close()) {
			consoleLog("Failed to Close Old CPU Simulator", MAIN_ERR);
			result = -1;
		}
	};

	if (!oldCPUSimulator.run(syncMode, maxMhz, targetMhz, refreshHz)) {
		consoleLog("Failed to Run Old CPU Simulator", MAIN_ERR);
		return result;
	}

	result = 0;
	return result;
}