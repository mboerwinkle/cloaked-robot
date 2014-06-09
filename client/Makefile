CC=gcc
CFLAGS=-Wall -O2 -c -pthread $(DEBUG)
LFLAGS=-lSDL2 -pthread -lm
SOURCES=gui.c
OBJECTS=$(SOURCES:.c=.o)

.PHONY: debug remake clean

run: $(OBJECTS)
	$(CC) $(OBJECTS) $(DEBUG) -o run $(LFLAGS)

gui.o: gui.c gui.h
	$(CC) $(CFLAGS) gui.c

clean:
	rm -f *.o run

debug:
	$(MAKE) DEBUG="-g -O0"
	
remake:
	$(MAKE) clean
	$(MAKE)
