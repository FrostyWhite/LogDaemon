## README:

__Installation:__
Install via “make”, clean executables via “make clean”

__Usage:__
The program should be run via commandline

First run the Daemon program, which will setup the directory
structure and then proceeds to wait for input.

Then run the LogTester program. You must give the number of testers
to spawn as an argument.

When you are done you can close the daemon with a signal (SIGTERM or SIGINT
should do the trick). The program does also quit automatically after an idle period
_(which is set to be short for testing and debugging purposes and can be altered in
source/LogDaemon.c)_

Log output can be found in the _“logs”_ folder.

The _"links"_ folder is used for storing named pipes.

__Notes:__
The program still needs some work: The log text is not perfectly formatted and
there still is some issues with the daemon writing duplicates. Either Tester
program or the Daemon also may with large number of testers cause the programs
to get stuck which causes the Daemon to timeout and exit. The program doesn't do
a proper clean up at exit.

The program is tested to compile on MacOS and Linux but other Unix systems should
also work with minimal changes to the code.
