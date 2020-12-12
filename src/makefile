# This is the src makefile for sodoku2.
#
# Started by: E. van der Woude, September 2020.
#

SHELL = /bin/bash

CC = gcc -Ofast -Wall -Wextra -Werror -pedantic

objects = sokoban2.o error.o setup.o resolve.o move.o position.o utility.o solution.o

~/bin/soko2 : $(objects)
	gcc -o ~/bin/soko2 -Wall -Wextra -Werror $(objects)

sokoban2.o: sokoban2.h error.h setup.h resolve.h utility.h
error.o: error.h
setup.o: sokoban2.h error.h setup.h
resolve.o: sokoban2.h error.h resolve_int.h  move.h position.h utility.h solution.h resolve.h
move.o: sokoban2.h error.h resolve_int.h utility.h move.h
position.o: sokoban2.h error.h resolve_int.h utility.h position.h
utility.o: sokoban2.h error.h resolve_int.h utility.h
solution.o: sokoban2.h error.h resolve_int.h move.h utility.h solution.h
