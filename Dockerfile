# Multi-stage build for C backend
FROM gcc:12 AS builder

WORKDIR /app

# Install sqlite3 dev headers
RUN apt-get update && apt-get install -y libsqlite3-dev

# Copy source files
COPY backend/*.c backend/*.h backend/Makefile ./
COPY backend/sqlite3.c backend/sqlite3.h ./

# Build
RUN gcc -std=c11 -Wall -Wextra -O2 -I. -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION \
    -c main.c player.c avl.c hashtable.c maxheap.c stack.c queue.c \
    linkedlist.c database.c api.c http_server.c sqlite3.c && \
    gcc -o leaderboard main.o player.o avl.o hashtable.o maxheap.o stack.o \
    queue.o linkedlist.o database.o api.o http_server.o sqlite3.o -lpthread -ldl

# Runtime stage
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    libsqlite3-0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary and public folder
COPY --from=builder /app/leaderboard .
COPY backend/public ./public

# Database directory
RUN mkdir -p /data

# Expose port
EXPOSE 8080

# Run with database in /data volume
ENV DATABASE_PATH=/data/leaderboard.db
CMD ["./leaderboard", "8080"]