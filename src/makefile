# This is the src makefile for sodoku2.
#
# Started by: E. van der Woude, September 2020.
#

SHELL = /bin/bash

CC = gcc


objects = sokoban2.o setup.o error.o

~/bin/soko2 : $(objects)
	gcc -o ~/bin/soko2 -Wall -Wextra  $(objects)

sokoban2.o: sokoban2.h setup.h
setup.o: sokoban2.h setup.h
error.o: error.h
