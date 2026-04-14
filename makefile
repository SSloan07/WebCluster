CC = gcc

CFLAGS = -Wall -Wextra -I./src/Network -I./src/Proxy -pthread -g

OBJ_NET = src/Network/tcp.o
OBJ_PROXY = src/Proxy/Cluster.o src/Proxy/RoundRobin.o
OBJ_SERV = $(OBJ_NET) $(OBJ_PROXY)

all: servidor cliente backend

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

servidor: $(OBJ_SERV) main.c
	$(CC) $(CFLAGS) main.c $(OBJ_SERV) -o servidor

cliente: $(OBJ_NET) client_main.c
	$(CC) $(CFLAGS) client_main.c $(OBJ_NET) -o cliente

backend: $(OBJ_NET) backend_main.c
	$(CC) $(CFLAGS) backend_main.c $(OBJ_NET) -o backend

clean:
	rm -f $(OBJ_SERV) $(OBJ_NET) servidor cliente backend
	@echo "Limpieza completada. Siuuu"

.PHONY: all clean