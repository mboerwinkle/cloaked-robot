CC=gcc
CFLAGS=-Wall -O2 -c -pthread -ffast-math $(DEBUG)
LFLAGS=-lSDL2 -lm -lrt -lGLEW -lGL -lGLU -pthread
SOURCES=main.c loadsector.c interpret.c unloadsector.c field.c font.c gui.c gfx.c entity.c module.c ais.c
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

field.o: field.c gui.h entity.h field.h
	$(CC) $(CFLAGS) field.c

font.o: font.c fontData.h gui.h
	$(CC) $(CFLAGS) font.c

gui.o: gui.c gui.h font.h field.h entity.h ais.h module.h
	$(CC) $(CFLAGS) gui.c

gfx.o: gfx.c gfx.h gui.h
	$(CC) $(CFLAGS) gfx.c

entity.o: entity.c entity.h gfx.h field.h gui.h ais.h
	$(CC) $(CFLAGS) entity.c

module.o: module.c entity.h field.h
	$(CC) $(CFLAGS) module.c

ais.o: ais.c entity.h gui.h ais.h field.h
	$(CC) $(CFLAGS) ais.c

clean:
	rm -f *.o run

debug:
	$(MAKE) DEBUG="-g -O0"
	
remake:
	$(MAKE) clean
	$(MAKE)
