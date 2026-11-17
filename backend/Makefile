CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I. -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION
LDFLAGS = -lws2_32

SRCS = main.c \
       player.c \
       avl.c \
       hashtable.c \
       maxheap.c \
       stack.c \
       queue.c \
       linkedlist.c \
       database.c \
       api.c \
       http_server.c \
       sqlite3.c

OBJS = $(SRCS:.c=.o)
TARGET = leaderboard.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o $(TARGET) 2>nul || rm -f *.o $(TARGET)

.PHONY: all clean