PROJECT = randplace
SRC = .
INC = -I. -I/usr/local/include

CC_FLAGS = $(CFLAG)

CC = gcc
CC_FLAGS += -O2 -std=c99 -fPIC $(INC) -Wall -Wextra -c

SRC_C   = $(foreach dir, $(SRC), $(wildcard $(dir)/*.c))
OBJ_C   = $(patsubst %.c, %.o, $(SRC_C))
OBJ     = $(OBJ_C)

.PHONY : all gprof testc
all: $(PROJECT).so gprof testc

$(PROJECT).so: $(OBJ)
	ld -shared $(OBJ) -o $(PROJECT).so

$(OBJ_C) : %.o : %.c
	$(CC) $(CC_FLAGS) -o $@ $<

test:
	lua test.lua

gprof:
	$(CC) -pg -I. -o gprof/gprof.exe gprof/main.c rand_place.c intlist.c
	gprof/gprof.exe
	gprof gprof/gprof.exe | gprof/gprof2dot.py | dot -Tpng -o output.png

.PHONY : clean
clean:
	rm -f $(PROJECT).so $(OBJ) testc
	rm -f gprof/gprof.exe gmon.out output.png

testc:
	gcc -lm -fsanitize=address -ggdb rand_place.c intlist.c test/testc.c -o testc