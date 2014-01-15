//
// Copyright (c) 2005, Wei Mingzhi <whistler@openoffice.org>
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA
//

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define vsnprintf _vsnprintf
#else
#include <unistd.h>
#endif

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

#ifdef WITH_CONFIG_H
#include "config.h"
#endif

#ifndef PREFIX
#define PREFIX "./"
#endif

#ifndef DATA_DIR
#define DATA_DIR PREFIX "data/"
#endif

#ifndef MUSIC_DIR
#define MUSIC_DIR PREFIX "music/"
#endif

#ifndef SOUND_DIR
#define SOUND_DIR PREFIX "sound/"
#endif

#ifndef IMAGES_DIR
#define IMAGES_DIR PREFIX "images/"
#endif

#ifndef FONTS_DIR
#define FONTS_DIR PREFIX "fonts/"
#endif

extern SDL_Surface *gpScreen;
extern bool g_fNoSound;
extern bool g_fNoMusic;

// main.cpp functions...
void UserQuit();

// util.cpp functions...
void trim(char *str);
char *va(const char *format, ...);
int RandomLong(int from, int to);
float RandomFloat(float from, float to);
int log2(int val);
void TerminateOnError(const char *fmt, ...);

char *UTIL_StrGetLine(const char *buf, int width, int &length);
unsigned int UTIL_GetPixel(SDL_Surface *surface, int x, int y);
void UTIL_PutPixel(SDL_Surface *surface, int x, int y, unsigned int pixel);
int UTIL_GetPixel(SDL_Surface *f, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b);
int UTIL_PutPixel(SDL_Surface *f, int x, int y, unsigned char r, unsigned char g, unsigned char b);
void UTIL_RevertSurfaceX(SDL_Surface *s);
void UTIL_RevertSurfaceY(SDL_Surface *s);
void UTIL_RevertSurfaceXY(SDL_Surface *s);
SDL_Surface *UTIL_ScaleSurface(SDL_Surface *s, int w, int h);
int UTIL_ScaleBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr);
SDL_Surface *UTIL_GrayStyle(SDL_Surface *s);
void UTIL_Delay(int duration);

// text.cpp functions...
void InitTextMessage();
void FreeTextMessage();
const char *msg(const char *name);

// config.cpp functions...
void LoadCfg();
void SaveCfg();

#include "ini.h"
#include "general.h"
#include "tile.h"
#include "hand.h"
#include "player.h"
#include "bot.h"
#include "game.h"

extern CIniFile cfg;


#endif

