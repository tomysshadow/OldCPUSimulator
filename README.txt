Old CPU Simulator 2.2.3
By Anthony Kleine

	This command line tool simulates running a process on a CPU with a
	slower clock speed in order to make old games run at the correct speed
	or underclock CPU intensive processes like video encoding.


Usage: OldCPUSimulator -t targetRate [options] -sw software.exe

-t targetRate (or --target-rate targetRate)
	The Target Rate (in MHz, from 1 to your CPU's current clock speed)
	to simulate.
	This argument is required.

	Go to http://intel.com/pressroom/kits/quickrefyr.htm
	for a quick reference of year to clock speed.
	Note that many of the measurements in said reference
	are in GHz. This tool uses MHz.

-r refreshRate (or --refresh-rate refreshRate)
	The Refresh Rate (in Hz) at which to refresh.
	This argument is not required.
	
	Effectively an accuracy meter.
	Lower numbers are more accurate but result in choppier playback.
	Higher numbers are less accurate but result in smoother playback.
	If not specified, Old CPU Simulator will default to the
	smoothest possible playback setting.
	
	Try 60, 30 or 15 for gaming, 1 to 6 for video encoding.

-ph (or --set-process-priority-high)
	Set the process priority of Old CPU Simulator to High,
	in order to potentially improve the accuracy of the simulation.

-a1 (or --set-synced-process-affinity-one)
	Set the process affinity of the synced process
	to one, which may make the speed more consistent
	and prevent crashes.
	May not work with newer games.

-mt (or --synced-process-main-thread-only)
	This is an optimization which improves the accuracy of the
	simulation, but may not work well with multithreaded software.

-rf (or --refresh-rate-floor-fifteen)
	Rounds Refresh Rate to the nearest multiple of 15 if applicable.

-sw software.exe (or --software software.exe)
	The software that will be created as the synced process.
	This argument is required.

	It must be given last, after all the options.
	Command line arguments may also be specified,
	which will be passed to the software.