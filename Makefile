#version：0.1
#author: dongyue.z
#date:2016.01.29

CC = gcc -w -O2 -fPIC

TARGET = threadpool
LIBSO_NAME = libco_threadpool.so
SRC_PATH = .
SRCS = $(wildcard $(SRC_PATH)/*.c)
SRC_TMP = $(notdir $(SRCS))
OBJS = $(SRC_TMP:%.c = %.o)
ARGV = -g
LIBSO =   -lpthread

all:$(OBJS)
	$(CC) $(ARGV) $(OBJS) $(LIBSO) -o $(TARGET)

lib:$(OBJS)
	$(CC) -fPIC -shared  $(OBJS) $(LIBSO) -o $(LIBSO_NAME) 
	
%.o:%.c
	$(CC) -c -o $@ $<

.PHONY: clean
clean:
	-rm $(TARGET)
	-rm $(LIBSO_NAME)
	-rm *.o -f
