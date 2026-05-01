CC = gcc

CFLAGS = -Wall -Wextra -I./src/Network -I./src/Proxy -I./ManageClient -I./Configuration -I./src/HTTP -I./HTTP -pthread -g

OBJ_NET = src/Network/tcp.o
OBJ_PROXY = src/Proxy/Cluster.o src/Proxy/RoundRobin.o src/Proxy/Logger.o
OBJ_MANAGE = ManageClient/manage_client.o
OBJ_CONFIG = Configuration/config.o
OBJ_HTTP_PROXY = \
	src/HTTP/HttpParser.o \
	src/HTTP/HttpParser_peer.o \
	src/HTTP/http_peer/processRequest.o \
	src/HTTP/http_peer/methods/get.o \
	src/HTTP/http_peer/methods/head.o \
	src/HTTP/http_peer/methods/post.o \
	src/HTTP/http_peer/requestParser.o \
	src/HTTP/http_peer/utils/readFile.o \
	src/HTTP/http_peer/utils/getDate.o \
	src/HTTP/http_peer/utils/stringToEnum.o \
	src/HTTP/http_peer/utils/enumToString.o \
	src/HTTP/structs/request.o \
	src/HTTP/structs/response.o


OBJ_SERV = $(OBJ_NET) $(OBJ_PROXY) $(OBJ_MANAGE) $(OBJ_CONFIG) $(OBJ_HTTP_PROXY)

all: servidor cliente backend http_test

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

servidor: $(OBJ_SERV) main.c
	$(CC) $(CFLAGS) main.c $(OBJ_SERV) -o servidor

cliente: $(OBJ_NET) client_main.c
	$(CC) $(CFLAGS) client_main.c $(OBJ_NET) -o cliente

backend: $(OBJ_NET) $(OBJ_HTTP_PROXY) backend_main.c
	$(CC) $(CFLAGS) backend_main.c $(OBJ_NET) $(OBJ_HTTP_PROXY) -o backend


clean:
	rm -f $(OBJ_NET) $(OBJ_PROXY) $(OBJ_MANAGE) $(OBJ_CONFIG) $(OBJ_HTTP_PROXY)servidor cliente backend 
	@echo "Limpieza completada."

.PHONY: all clean run_http_test
