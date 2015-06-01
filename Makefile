# Name: Makefile
# Project: steamingpot 
# Author: Stingworks

PROGRAM	= steamingpot
INCLUDE_PATH = .
OBJ	= $(PROGRAM).o 
USBFLAGS = 
USBLIBS	= 
SDLFLAGS = `sdl-config --cflags`
SDLLIBS	= `sdl-config --libs` -lSDL_ttf -lSDL_gfx -lSDL_net
CC = gcc -Wall -I$(INCLUDE_PATH)
CFLAGS = -O $(USBFLAGS) $(SDLFLAGS) -g
LIBS = $(USBLIBS) $(SDLLIBS) -lm -g

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o

