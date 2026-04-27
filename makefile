CC = gcc

CFLAGS = -Wall -Wextra -I./src/Network -I./src/Proxy -I./ManageClient -I./Configuration -I./src/HTTP -I./HTTP -pthread -g

OBJ_NET = src/Network/tcp.o
OBJ_PROXY = src/Proxy/Cluster.o src/Proxy/RoundRobin.o src/Proxy/Logger.o
OBJ_MANAGE = ManageClient/manage_client.o
OBJ_CONFIG = Configuration/config.o
OBJ_HTTP_PROXY = src/HTTP/HttpParser_lib.o

OBJ_HTTP_TEST = \
	HTTP/structs/requestLine.o HTTP/structs/response.o \
	HTTP/requestParser.o HTTP/processRequest.o \
	HTTP/utils/readFile.o HTTP/utils/getEnumRequestLine.o \
	HTTP/utils/enumToString.o HTTP/utils/getDate.o \
	HTTP/methods/get.o HTTP/methods/head.o

OBJ_SERV = $(OBJ_NET) $(OBJ_PROXY) $(OBJ_MANAGE) $(OBJ_CONFIG) $(OBJ_HTTP_PROXY)

all: servidor cliente backend http_test

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

servidor: $(OBJ_SERV) main.c
	$(CC) $(CFLAGS) main.c $(OBJ_SERV) -o servidor

cliente: $(OBJ_NET) client_main.c
	$(CC) $(CFLAGS) client_main.c $(OBJ_NET) -o cliente

backend: $(OBJ_NET) backend_main.c
	$(CC) $(CFLAGS) backend_main.c $(OBJ_NET) -o backend

http_test: $(OBJ_HTTP_TEST) main_http.c
	$(CC) $(CFLAGS) main_http.c $(OBJ_HTTP_TEST) -o http_test

run_http_test: http_test
	./http_test

clean:
	rm -f $(OBJ_NET) $(OBJ_PROXY) $(OBJ_MANAGE) $(OBJ_CONFIG) $(OBJ_HTTP_PROXY) $(OBJ_HTTP_TEST) servidor cliente backend http_test
	@echo "Limpieza completada."

.PHONY: all clean run_http_test