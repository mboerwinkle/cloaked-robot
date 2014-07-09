CC=gcc
CFLAGS=-Wall -Wno-strict-aliasing -O2 -c -pthread $(DEBUG)
LFLAGS=-lSDL2 -pthread -lm -lImlib2
SOURCES=gui.c images.c
OBJECTS=$(SOURCES:.c=.o)

.PHONY: debug remake clean

run: $(OBJECTS)
	$(CC) $(OBJECTS) $(DEBUG) -o run $(LFLAGS)

gui.o: gui.c gui.h images.h
	$(CC) $(CFLAGS) gui.c

images.o: images.c gui.h images.h
	$(CC) $(CFLAGS) images.c

clean:
	rm -f *.o run

debug:
	$(MAKE) DEBUG="-g -O0"
	
remake:
	$(MAKE) clean
	$(MAKE)
