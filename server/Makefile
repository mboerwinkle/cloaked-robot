CC=gcc
CFLAGS=-Wall -O2 -c -pthread -ffast-math $(DEBUG)
LFLAGS=-lm
SOURCES=main.c loadsector.c interpret.c unloadsector.c field.c entity.c module.c ais.c
OBJECTS=$(SOURCES:.c=.o)

.PHONY: all debug remake clean

all: run

run: $(OBJECTS)
	$(CC) $(OBJECTS) $(DEBUG) -o run $(LFLAGS)

main.o: main.c globals.h
	$(CC) $(CFLAGS) main.c

loadsector.o: loadsector.c globals.h
	$(CC) $(CFLAGS) loadsector.c

interpret.o: interpret.c globals.h
	$(CC) $(CFLAGS) interpret.c

unloadsector.o: unloadsector.c globals.h
	$(CC) $(CFLAGS) unloadsector.c

field.o: field.c entity.h field.h
	$(CC) $(CFLAGS) field.c

entity.o: entity.c entity.h field.h ais.h
	$(CC) $(CFLAGS) entity.c

module.o: module.c entity.h field.h
	$(CC) $(CFLAGS) module.c

ais.o: ais.c entity.h ais.h field.h
	$(CC) $(CFLAGS) ais.c

clean:
	rm -f *.o run

debug:
	$(MAKE) DEBUG="-g -O0"
	
remake:
	$(MAKE) clean
	$(MAKE)
