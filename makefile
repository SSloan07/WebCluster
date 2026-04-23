CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = main.c \
	HTTP/structs/requestLine.c HTTP/structs/response.c \
	HTTP/requestParser.c HTTP/processRequest.c \
	HTTP/utils/readFile.c HTTP/utils/getEnumRequestLine.c HTTP/utils/enumToString.c HTTP/utils/getDate.c\
	HTTP/methods/get.c HTTP/methods/head.c

OBJS = $(SRCS:.c=.o)
TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: clean all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)