CC = gcc

CFLAGS = -Wall -Wextra -I./src/Network -I./src/Proxy -I./ManageClient -I./Configuration -I./src/HTTP -I./HTTP -pthread -g

OBJ_NET = src/Network/tcp.o
OBJ_CONNECT = src/Network/connect_tunnel.o
OBJ_PROXY = src/Proxy/Cluster.o src/Proxy/RoundRobin.o src/Proxy/Logger.o
OBJ_MANAGE = ManageClient/manage_client.o ManageClient/manage_client_utils.o
OBJ_CONFIG = Configuration/config.o
OBJ_CACHE = src/cache/Manage_cache.o src/cache/utils/cache_utils.o
OBJ_ENTRY = proxy_main.o backend_main.o
OBJ_HTTP_PROXY = \
	src/HTTP/HttpParser.o \
	src/HTTP/HttpParser_peer.o \
	src/HTTP/http_peer/processRequest.o \
	src/HTTP/http_peer/methods/get.o \
	src/HTTP/http_peer/methods/head.o \
	src/HTTP/http_peer/methods/post.o \
	src/HTTP/http_peer/methods/put.o \
	src/HTTP/http_peer/methods/delete.o \
	src/HTTP/http_peer/methods/trace.o \
	src/HTTP/http_peer/methods/options.o \
	src/HTTP/http_peer/requestParser.o \
	src/HTTP/http_peer/utils/readFile.o \
	src/HTTP/http_peer/utils/getDate.o \
	src/HTTP/http_peer/utils/stringToEnum.o \
	src/HTTP/http_peer/utils/enumToString.o \
	src/HTTP/structs/request.o \
	src/HTTP/structs/response.o

OBJ_SERV = $(OBJ_NET) $(OBJ_CONNECT) $(OBJ_PROXY) $(OBJ_MANAGE) $(OBJ_CONFIG) $(OBJ_CACHE) $(OBJ_HTTP_PROXY) $(OBJ_ENTRY)

all: servidor

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

servidor: $(OBJ_SERV) main.c
	$(CC) $(CFLAGS) main.c $(OBJ_SERV) -o servidor

clean:
	rm -f $(OBJ_NET) $(OBJ_CONNECT) $(OBJ_PROXY) $(OBJ_MANAGE) $(OBJ_CONFIG) $(OBJ_CACHE) $(OBJ_HTTP_PROXY) $(OBJ_ENTRY) servidor
	@echo "Limpieza completada."

.PHONY: all clean
