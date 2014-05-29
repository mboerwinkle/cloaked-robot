CC=gcc
CFLAGS=-c -Wall
SOURCES=main.c loadsector.c interpret.c unloadsector.c
OBJECTS=$(SOURCES:.c=.o)
all: final

final: $(OBJECTS)
	$(CC) $(OBJECTS) -o run

main.o: main.c globals.h
	$(CC) $(CFLAGS) main.c

loadsector.o: loadsector.c globals.h
	$(CC) $(CFLAGS) loadsector.c

interpret.o: interpret.c globals.h
	$(CC) $(CFLAGS) interpret.c

unloadsector.o: unloadsector.c globals.h
	$(CC) $(CFLAGS) unloadsector.c
