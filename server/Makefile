CC=gcc
CFLAGS=-Wall -O2 -c -ffast-math $(DEBUG)
LFLAGS=-lm
SOURCES=main.c loadsector.c interpret.c unloadsector.c field.c entity.c module.c ais.c mkship.c loadship.c
OBJECTS=$(SOURCES:.c=.o)

.PHONY: all debug remake clean

all: run

run: $(OBJECTS)
	$(CC) $(OBJECTS) $(DEBUG) -o run $(LFLAGS)

main.o: main.c globals.h
	$(CC) $(CFLAGS) main.c

mkship.o: mkship.c globals.h
	$(CC) $(CFLAGS) mkship.c

loadsector.o: loadsector.c globals.h
	$(CC) $(CFLAGS) loadsector.c

loadship.o: loadship.c globals.h
	$(CC) $(CFLAGS) loadship.c

interpret.o: interpret.c globals.h
	$(CC) $(CFLAGS) interpret.c

unloadsector.o: unloadsector.c globals.h
	$(CC) $(CFLAGS) unloadsector.c

field.o: field.c globals.h
	$(CC) $(CFLAGS) field.c

entity.o: entity.c globals.h
	$(CC) $(CFLAGS) entity.c

module.o: module.c globals.h
	$(CC) $(CFLAGS) module.c

ais.o: ais.c globals.h
	$(CC) $(CFLAGS) ais.c

clean:
	rm -f *.o run

debug:
	$(MAKE) DEBUG="-g -O0"
	
remake:
	$(MAKE) clean
	$(MAKE)
