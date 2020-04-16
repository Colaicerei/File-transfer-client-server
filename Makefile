CFLAGS= -Wall -fpic -coverage -lm -std=c99

default:
	gcc -o ftserver ftserver.c
clean:
	rm -f ftserver