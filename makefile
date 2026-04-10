CC = gcc

CFLAGS = -Wall -Wextra -I./src/Network -I./src/Proxy -pthread -g

OBJ_NET = src/Network/tcp.o


OBJ_PROXY = src/Proxy/Cluster.o src/Proxy/RoundRobin.o


OBJ_SERV = $(OBJ_NET) $(OBJ_PROXY)

all: servidor cliente


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

servidor: $(OBJ_SERV) main.c
	$(CC) $(CFLAGS) main.c $(OBJ_SERV) -o servidor

cliente: $(OBJ_NET) client_main.c
	$(CC) $(CFLAGS) client_main.c $(OBJ_NET) -o cliente

clean:
	rm -f $(OBJ_SERV) servidor cliente
	@echo "Limpieza completada. Siuuu"

.PHONY: all clean
