PROJECT = randplace
SRC = .
INC = -I. -I/usr/local/include

CC_FLAGS = $(CFLAG)

CC = gcc
CC_FLAGS += -O2 -fPIC $(INC) -Wall -Wextra -c

SRC_C   = $(foreach dir, $(SRC), $(wildcard $(dir)/*.c))
OBJ_C   = $(patsubst %.c, %.o, $(SRC_C))
OBJ     = $(OBJ_C)

.PHONY : all
all: $(PROJECT).so

$(PROJECT).so: $(OBJ)
	ld -shared $(OBJ) -o $(PROJECT).so

$(OBJ_C) : %.o : %.c
	$(CC) $(CC_FLAGS) -o $@ $<

test:
	lua test.lua

.PHONY : clean
clean:
	rm -f $(PROJECT).so $(OBJ)
