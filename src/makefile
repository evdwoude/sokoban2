# This is the src makefile for sodoku2.
#
# Started by: E. van der Woude, September 2020.
#

SHELL = /bin/bash

CC = gcc

objects = sokoban2.o error.o setup.o resolve.o transposition.o utility.o

~/bin/soko2 : $(objects)
	gcc -o ~/bin/soko2 -Wall -Wextra $(objects)

sokoban2.o: sokoban2.h error.h setup.h utility.h
error.o: error.h
setup.o: sokoban2.h error.h setup.h utility.h
resolve.o: sokoban2.h error.h resolve_int.h transposition.h utility.h resolve.h
transposition.o: sokoban2.h error.h resolve_int.h utility.h transposition.h
utility.o: sokoban2.h error.h resolve_int.h utility.h
