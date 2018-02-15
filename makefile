CC=gcc
Cflags=-I -Wall -std=c99 -pedantic -ptheread

all: Daemon Tester

Daemon: ./source/LogDaemon.c ./source/LogTools.c ./source/LogTools.h ./source/shouter.c ./source/shouter.h
	@echo
	$(CC) -o Daemon ./source/LogDaemon.c ./source/LogTools.c ./source/shouter.c
	@echo $@ compiled
	@echo

Tester: ./source/tester.c ./source/DLogLib.c ./source/DLogLib.h
	$(CC) -o Tester ./source/tester.c ./source/DLogLib.c
	@echo $@ compiled
	@echo

clean:
	@echo
	rm Daemon Tester
	@echo executables removed
	@echo

#reset:
#	@echo
#	rm ./threaded_cleaner.config ./threaded_cleaner.log
#	rm -rfv ./cleaned/* ./logs/*
#	rm -r ./logs ./cleaned
