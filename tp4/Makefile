CC=gcc
CFLAGS=-W -Wall -g -Wextra
LDFLAGS=-lm
EXEC=scindeur lirefichier

all: $(EXEC)

scindeur: scindeur.o
	$(CC) -o $@ $< $(LDFLAGS)

lirefichier: lirefichier.o
	$(CC) -o $@ $< $(LDFLAGS)

scindeur.o: scindeur.c
	$(CC) -o $@ -c $< $(CFLAGS)

lirefichier.o: lirefichier.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
