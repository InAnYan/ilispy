EXEC_NAME=ilispy

CC=clang
LD=clang

BIN_DIR=bin
INC_DIR=include
SRC_DIR=src
OBJ_DIR=obj
SUBDIRS=. ../lib/mpc

INCLUDES_DIRS=lib
LIBS_DIRS=/usr/local/lib

LIBS=edit
DEFINES= LISPY_COMPILE_LINUX

CFLAGS=-g -Wall -std=c99 -gdwarf-4
LDFLAGS=

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.d

FULL_EXEC=$(BIN_DIR)/$(EXEC_NAME)
FULL_CFLAGS=$(CFLAGS) -I$(INC_DIR) $(addprefix -I, $(INCLUDES_DIRS)) $(addprefix -D, $(DEFINES)) $(DEPFLAGS)
FULL_LDFLAGS=$(LDFLAGS) $(addprefix -L, $(LIBS_DIRS)) $(addprefix -l, $(LIBS))

SRC_SUBDIRS=$(addprefix $(SRC_DIR)/, $(SUBDIRS))
INC_SUBDIRS=$(addprefix $(INC_DIR)/, $(SUBDIRS))

INCS=$(wildcard *.h $(foreach fd, $(INC_SUBDIRS), $(fd)/*.h))
SRCS=$(wildcard *.c $(foreach fd, $(SRC_SUBDIRS), $(fd)/*.c))

OBJS=$(subst $(SRC_DIR), $(OBJ_DIR), $(SRCS:.c=.o))
DEPFILES := $(OBJS:.o=.d)

all: $(FULL_EXEC)

$(FULL_EXEC): $(OBJS)
	$(CC) $(OBJS) $(FULL_LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(FULL_CFLAGS) -c $< -o $@

clean:
	rm -rf lispy $(BIN_DIR)/*.exe $(BIN_DIR)/*.out $(BIN_DIR)/*.bin $(OBJ_DIR)/*

run:
	./bin/ilispy

$(DEPFILES):

include $(wildcard $(DEPFILES))
