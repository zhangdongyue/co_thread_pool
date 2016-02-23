#version：0.1
#author: dongyue.z
#date:2016.01.29

CC = gcc -w -O2 -fPIC

TARGET = threadpool
LIBSO_NAME = libco_threadpool.so
LIBA_NAME = libco_threadpool.a
SRC_PATH = .
SRCS = $(wildcard $(SRC_PATH)/*.c)
SRC_TMP = $(notdir $(SRCS))
#OBJS = $(SRC_TMP:%.c = %.o)
OBJS=co_thread_pool.o \
	 co_tool_func.o
ARGV=-g
LIBSO =   -lpthread
AR=ar -rs


all:$(OBJS) test.c
	$(CC) $(ARGV) $? $(LIBSO) -o $(TARGET)

lib:${LIBSO_NAME} ${LIBA_NAME}

${LIBSO_NAME}:$(OBJS)
	$(CC) -fPIC -shared  $(OBJS) $(LIBSO) -o $@

${LIBA_NAME}:$(OBJS)
	${AR} $@ $?
	
%.o:%.c
	$(CC) -c -o $@ $<

.PHONY: clean
clean:
	-rm ${TARGET}
	-rm ${LIBSO_NAME}
	-rm ${LIBA_NAME}
	-rm *.o -f
