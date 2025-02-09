# Makefile para compilar o servidor HTTP em C

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -pthread

SRCS = src/main.c src/server.c src/socket_utils.c src/http_parser.c src/config.c
OBJS = $(SRCS:.c=.o)
TARGET = http_server

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
