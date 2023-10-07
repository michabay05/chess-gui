CC=gcc
LIBS=-lraylib -lm
CFLAGS=-Wall -Wextra -pedantic -std=c17 

SRC=src
SRCS=$(wildcard $(SRC)/*.c)

INC=src vendor src/gui 
HDRS=$(wildcard $(INC)/*.h)

OBJ=obj
OBJS=$(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

BIN_DIR=bin
BIN=bin/chess_gui

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS) $(OBJ) $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBS)

$(OBJ)/%.o: $(SRC)/%.c $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(OBJ)
	$(RM) -r $(BIN_DIR)

run:
	$(MAKE) all
	$(BIN)
