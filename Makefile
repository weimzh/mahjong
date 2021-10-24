#
# Copyright (c) 2005, Wei Mingzhi. All rights reserved.
#
# Use, redistributions and modifications of this file is
# unrestricted provided the above copyright notice is
# preserved.
#

OBJ = \
	bot.o config.o game.o general.o hand.o ini.o main.o \
	player.o text.o tile.o util.o

HEADERS = \
	bot.h game.h general.h hand.h ini.h main.h player.h \
	tile.h

CC = gcc
CXX = g++

TARGET = mj

#BASEFLAGS = -g3 -D_DEBUG=1
BASEFLAGS = -s -O3 -std=c++0x -Wno-narrowing

CFLAGS = ${BASEFLAGS} `sdl-config --cflags`
LDFLAGS = ${BASEFLAGS} `sdl-config --libs` -lSDL_image -lSDL_mixer -lSDL_ttf

all: ${TARGET}

${TARGET}: ${OBJ}
	${CXX} -o ${TARGET} ${OBJ} ${LDFLAGS}

clean:
	rm -f *.o ${TARGET}

distclean:
	rm -f *.o ${TARGET}

%.o:    %.cpp ${HEADERS}
	${CXX} ${CFLAGS} -c $< -o $@

%.o:    %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

