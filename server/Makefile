CC=gcc
CFLAGS=-Wall -Wno-strict-aliasing -O2 -c -g -ffast-math $(DEBUG)
LFLAGS=-lm -pthread -lrt -lSDL2
SOURCES=main.c loadsector.c unloadsector.c field.c entity.c module.c ais.c mkship.c loadship.c networking.c move.c searchforsector.c positionFuncs.c spawnstroids.c collisions.c physics.c
OBJECTS=$(SOURCES:.c=.o)

.PHONY: debug remake clean

run: $(OBJECTS)
	$(CC) $(OBJECTS) $(DEBUG) -o run $(LFLAGS)

main.o: main.c globals.h networking.h
	$(CC) $(CFLAGS) main.c

mkship.o: mkship.c globals.h
	$(CC) $(CFLAGS) mkship.c

loadsector.o: loadsector.c globals.h
	$(CC) $(CFLAGS) loadsector.c

loadship.o: loadship.c globals.h
	$(CC) $(CFLAGS) loadship.c

unloadsector.o: unloadsector.c globals.h
	$(CC) $(CFLAGS) unloadsector.c

field.o: field.c globals.h networking.h
	$(CC) $(CFLAGS) field.c

entity.o: entity.c globals.h
	$(CC) $(CFLAGS) entity.c

module.o: module.c globals.h
	$(CC) $(CFLAGS) module.c

ais.o: ais.c globals.h
	$(CC) $(CFLAGS) ais.c

networking.o: networking.c globals.h networking.h
	$(CC) $(CFLAGS) networking.c

move.o: move.c globals.h
	$(CC) $(CFLAGS) move.c

searchforsector.o: searchforsector.c globals.h
	$(CC) $(CFLAGS) searchforsector.c

positionFuncs.o: positionFuncs.c globals.h
	$(CC) $(CFLAGS) positionFuncs.c

spawnstroids.o: spawnstroids.c globals.h
	$(CC) $(CFLAGS) spawnstroids.c

collisions.o: collisions.c globals.h
	$(CC) $(CFLAGS) collisions.c

physics.o: physics.c globals.h
	$(CC) $(CFLAGS) physics.c

clean:
	rm -f *.o run

debug:
	$(MAKE) DEBUG="-g -O0"
	
remake:
	$(MAKE) clean
	$(MAKE)
