CC = gcc
CFLAGS = -Wall -Wextra -I./src/Network -g
OBJ = src/Network/tcp.o

all: servidor cliente


$(OBJ): src/Network/tcp.c
	$(CC) $(CFLAGS) -c $< -o $@


servidor: $(OBJ) main.c
	$(CC) $(CFLAGS) main.c $(OBJ) -o servidor


cliente: $(OBJ) client_main.c
	$(CC) $(CFLAGS) client_main.c $(OBJ) -o cliente

clean:
	rm -f $(OBJ) servidor cliente
	@echo "Limpieza completada. Siuuu"

.PHONY: all clean
