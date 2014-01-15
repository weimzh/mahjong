//
// Copyright (c) 2005, Wei Mingzhi <whistler@openoffice.org>
// Portions Copyright (c) 2001, Gregory Velichansky <hmaon@bumba.net>
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

#include "main.h"

void trim(char *str)
{
   int pos = 0;
   char *dest = str;

   // skip leading blanks
   while (str[pos] <= ' ' && str[pos] > 0)
      pos++;

   while (str[pos]) {
      *(dest++) = str[pos];
      pos++;
   }

   *(dest--) = '\0'; // store the null

   // remove trailing blanks
   while (dest >= str && *dest <= ' ' && *dest > 0)
      *(dest--) = '\0';
}

// Does a varargs printf into a temp buffer, so we don't need to have
// varargs versions of all text functions.
char *va(const char *format, ...)
{
   static char string[256];
   va_list     argptr;

   va_start(argptr, format);
   vsnprintf(string, 256, format, argptr);
   va_end(argptr);

   return string;
}

static int glSeed = 0, glGen2 = 0, glGen1 = 0; // our random number generator's seed

// This function initializes the random seed based on the initial seed value passed in the
// initial_seed parameter.
static void lsrand(unsigned int initial_seed)
{
   // Pick two large integers such that one is double the other
   glGen2 = 3719;
   glGen1 = glGen2 / 2;

   // fill in the initial seed of the random number generator
   glSeed = (glGen1 * initial_seed) + glGen2;
}

// This function is the equivalent of the rand() standard C library function, except that
// whereas rand() works only with short integers (i.e. not above 32767), this function is
// able to generate 32-bit random numbers. Isn't that nice?
static int lrand(void)
{
   if (glSeed == 0) // if the random seed isn't initialized...
      lsrand(time(NULL)); // initialize it first
   glSeed = (glGen1 * glSeed) + glGen2; // do some twisted math
   return (glSeed > 0) ? glSeed : -glSeed; // and return absolute value of the result
}

// This function returns a random integer number between (and including) the starting and
// ending values passed by parameters from and to.
int RandomLong(int from, int to)
{
   if (to <= from)
      return from;

   return from + lrand() / (INT_MAX / (to - from + 1));
}

// This function returns a random floating-point number between (and including) the starting
// and ending values passed by parameters from and to.
float RandomFloat(float from, float to)
{
   if (to <= from)
      return from;

   return from + (float)lrand() / (INT_MAX / (to - from));
}

int log2(int val)
{
   int answer = 0;

   while ((val >>= 1) != 0) {
      answer++;
   }

   return answer;
}

// This function terminates the game because of an error and
// prints the message string pointed to by fmt both in the
// console and in a messagebox.
void TerminateOnError(const char *fmt, ...)
{
   va_list argptr;
   char string[256];

   // concatenate all the arguments in one string
   va_start(argptr, fmt);
   vsnprintf(string, sizeof(string), fmt, argptr);
   va_end(argptr);

   fprintf(stderr, "\nFATAL ERROR: %s\n", string);

#ifdef _WIN32
   MessageBox(0, string, "ERROR", MB_ICONERROR);
#endif

   assert(!"TerminateOnError()"); // allows jumping to debugger
   exit(1);
}

char *UTIL_StrGetLine(const char *buf, int width, int &length)
{
   int w = 0, i, index = 0;
   static char str[256];

   str[0] = '\0';
   length = 0;

   while (buf != NULL && *buf != '\0' && (*buf == '\r' || *buf == '\n')) {
      buf++;
      length++;
   }

   while (buf != NULL && *buf != '\0') {
      if (w >= width) {
         break;
      }
      if ((unsigned char)*buf >= 0x7f) {
         if (strlen(buf) < 3 || w + 2 > width) {
            break;
         }
         for (i = 0; i < 3 && *buf; i++) {
            str[index++] = *buf++;
            length++;


         }
         w += 2;
      } else if (*buf == '\r' || *buf == '\n') {
         break;
      } else {
         str[index++] = *buf++;
         length++;
         w++;
      }
   }

   str[index] = '\0';
   return str;
}

//-------------------------------------------------------------

// Return the pixel value at (x, y)
// NOTE: The surface must be locked before calling this!
unsigned int UTIL_GetPixel(SDL_Surface *surface, int x, int y)
{
   int bpp = surface->format->BytesPerPixel;

   // Here p is the address to the pixel we want to retrieve
   unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * bpp;

   switch (bpp) {
      case 1:
         return *p;

      case 2:
         return *(unsigned short *)p;

      case 3:
         if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
         else
            return p[0] | p[1] << 8 | p[2] << 16;

      case 4:
         return *(unsigned int *)p;
   }

   return 0; // shouldn't happen, but avoids warnings
}

// Set the pixel at (x, y) to the given value
// NOTE: The surface must be locked before calling this!
void UTIL_PutPixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
   int bpp = surface->format->BytesPerPixel;

   // Here p is the address to the pixel we want to set
   unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * bpp;

   switch (bpp) {
      case 1:
         *p = pixel;
         break;


      case 2:
         *(unsigned short *)p = pixel;
         break;

      case 3:
         if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
             p[0] = (pixel >> 16) & 0xff;
             p[1] = (pixel >> 8) & 0xff;
             p[2] = pixel & 0xff;
         } else {
             p[0] = pixel & 0xff;
             p[1] = (pixel >> 8) & 0xff;
             p[2] = (pixel >> 16) & 0xff;
         }
         break;

     case 4:
        *(unsigned int *)p = pixel;
        break;
   }

}

void UTIL_RevertSurfaceX(SDL_Surface *s)
{
   unsigned int temp;
   bool locked = false;

   if (SDL_MUSTLOCK(s)) {
      SDL_LockSurface(s);
      locked = true;
   }

   for (int i = 0; i < s->h; i++) {
      for (int j = 0; j < s->w / 2; j++) {
         temp = UTIL_GetPixel(s, j, i);
         UTIL_PutPixel(s, j, i, UTIL_GetPixel(s, s->w - j - 1, i));
         UTIL_PutPixel(s, s->w - j - 1, i, temp);
      }
   }

   if (locked)
      SDL_UnlockSurface(s);
}

void UTIL_RevertSurfaceY(SDL_Surface *s)
{
   unsigned int temp;
   bool locked = false;

   if (SDL_MUSTLOCK(s)) {
      SDL_LockSurface(s);
      locked = true;
   }

   for (int i = 0; i < s->h / 2; i++) {
      for (int j = 0; j < s->w; j++) {
         temp = UTIL_GetPixel(s, j, i);
         UTIL_PutPixel(s, j, i, UTIL_GetPixel(s, j, s->h - i - 1));
         UTIL_PutPixel(s, j, s->h - i - 1, temp);
      }
   }

   if (locked)
      SDL_UnlockSurface(s);
}

void UTIL_RevertSurfaceXY(SDL_Surface *s)
{
   UTIL_RevertSurfaceX(s);
   UTIL_RevertSurfaceY(s);
}

int UTIL_GetPixel(SDL_Surface *f, int x, int y,
   unsigned char *r, unsigned char *g, unsigned char *b)
{
   unsigned int pixel;
   unsigned char *pp;

   int n; /* general purpose 'n'. */

   if (f == NULL)
      return -1;

   pp = (unsigned char *)f->pixels;

   if (x >= f->w || y >= f->h)
      return -1;

   pp += (f->pitch * y);
   pp += (x * f->format->BytesPerPixel);

   // we do not lock the surface here, it would be inefficient XXX
   // this reads the pixel as though it was a big-endian integer XXX
   // I'm trying to avoid reading part the end of the pixel data by
   // using a data-type that's larger than the pixels
   for (n = 0, pixel = 0; n < f->format->BytesPerPixel; ++n, ++pp) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
      pixel >>= 8;
      pixel |= *pp << (f->format->BitsPerPixel - 8);
#else
      pixel |= *pp;
      pixel <<= 8;
#endif
   }

   SDL_GetRGB(pixel, f->format, r, g, b);
   return 0;
}

int UTIL_PutPixel(SDL_Surface *f, int x, int y,
   unsigned char r, unsigned char g, unsigned char b)
{
   unsigned int pixel;
   unsigned char *pp;

   int n;

   if (f == NULL)
      return -1;

   pp = (unsigned char *)f->pixels;

   if (x >= f->w || y >= f->h)
      return -1;

   pp += (f->pitch * y);
   pp += (x * f->format->BytesPerPixel);

   pixel = SDL_MapRGB(f->format, r, g, b);

   for (n = 0; n < f->format->BytesPerPixel; ++n, ++pp) {
      *pp = (unsigned char)(pixel & 0xFF);
      pixel >>= 8;
   }

   return 0;
}

inline unsigned int ifloor(unsigned int i)
{
   return (i & 0xFFFF0000);
}

inline unsigned int iceil(unsigned int i)
{
   return (i & 0xFFFF) ? i : ifloor(i) + (1<<16);
}

int UTIL_ScaleBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr)
{
   // This function is based on work done by Gregory Velichansky.

   unsigned char r, g, b;
   unsigned int rs, gs, bs; // sums.

   // temp storage for large int multiplies. Uint64 doen't exist anywhere
   double farea;
   unsigned int area;

   unsigned int sx, sy;
   unsigned int dsx, dsy;

   unsigned int wsx, wsy;

   unsigned int x, y; // x and y, for sub-area

   unsigned int tx, ty; // normal integers
   unsigned int lx, ly; // normal integers

   unsigned int w, e, n, s; // temp variables, named after compass directions

   bool locked = false;

   if (src == NULL || sr == NULL || dst == NULL || dr == NULL)
      return -1;

   if (!dr->w || !dr->h)
      return -1;

   if (SDL_MUSTLOCK(dst)) {
      SDL_LockSurface(dst);
      locked = true;
   }

   wsx = dsx = (sr->w << 16) / dr->w;
   if (!(wsx & 0xFFFF0000))
      wsx = 1 << 16;
   wsy = dsy = (sr->h << 16) / dr->h;
   if (!(wsy & 0xFFFF0000))
      wsy = 1 << 16;

   lx = dr->x + dr->w;
   ly = dr->y + dr->h;

   // lazy multiplication. Hey, it's only once per blit.
   farea = ((double)wsx) * ((double)wsy);
   farea /= (double)(1 << 16);
   area = (unsigned int)farea;

   for (ty = dr->y, sy = sr->y << 16; ty < ly; ++ty, sy += dsy) {
      for (tx = dr->x, sx = sr->x << 16; tx < lx; ++tx, sx += dsx) {
         rs = gs = bs = 0;
         for (y = ifloor(sy); iceil(sy + wsy) > y; y += (1<<16)) {
            for (x = ifloor(sx); iceil(sx + wsx) > x; x += (1<<16)) {
               w = (x > sx) ? 0 : sx - x;
               n = (y > sy) ? 0 : sy - y;

               e = (sx + wsx >= x + (1 << 16)) ? (1 << 16) : sx + wsx - x;
               s = (sy + wsy >= y + (1 << 16)) ? (1 << 16) : sy + wsy - y;

               if (w > e || s < n)
                  continue;

#define gsx ((int)(x >> 16) >= sr->x+sr->w ? sr->x+sr->w-1 : x >> 16)
#define gsy ((int)(y >> 16) >= sr->y+sr->h ? sr->y+sr->h-1 : y >> 16)

               UTIL_GetPixel(src, gsx, gsy, &r, &g, &b);

               rs += ((e - w) >> 8) * ((s - n)>>8) * r;
               gs += ((e - w) >> 8) * ((s - n)>>8) * g;
               bs += ((e - w) >> 8) * ((s - n)>>8) * b;
            }
         }


         rs /= area;
         gs /= area;
         bs /= area;

         if (rs >= 256 || gs >= 256 || bs >= 256) {
            fprintf(stderr, "fixed point weighted average overflow!");
            fprintf(stderr, "Values: %d, %d, %d\n", rs, gs, bs);
         }

         r = (unsigned char)rs;
         g = (unsigned char)gs;
         b = (unsigned char)bs;

         UTIL_PutPixel(dst, tx, ty, r, g, b);
      }
   }

   if (locked) {
      SDL_UnlockSurface(dst);
   }

   return 0;
#undef gsx
#undef gsy
}

SDL_Surface *UTIL_ScaleSurface(SDL_Surface *s, int w, int h)
{
   SDL_Surface *r = SDL_CreateRGBSurface(s->flags,
      w, h, s->format->BitsPerPixel, s->format->Rmask,
      s->format->Gmask, s->format->Bmask,
      s->format->Amask);

   SDL_Rect dstrect, dstrect2;

   dstrect.x = 0;
   dstrect.y = 0;
   dstrect.w = w;
   dstrect.h = h;

   dstrect2.x = 0;
   dstrect2.y = 0;
   dstrect2.w = s->w;
   dstrect2.h = s->h;

   UTIL_ScaleBlit(s, &dstrect2, r, &dstrect);
   return r;
}

SDL_Surface *UTIL_GrayStyle(SDL_Surface *s)
{
   SDL_Surface *t = SDL_CreateRGBSurface(s->flags,
      s->w, s->h, s->format->BitsPerPixel, s->format->Rmask,
      s->format->Gmask, s->format->Bmask,
      s->format->Amask);

   for (int i = 0; i < s->w; i++) {
      for (int j = 0; j < s->h; j++) {
         unsigned char r, g, b, n;
         int d = UTIL_GetPixel(s, i, j);
         SDL_GetRGB(d, s->format, &r, &g, &b);
         n = ((int)r + (int)g + (int)b) / 4;
         UTIL_PutPixel(t, i, j, n, n, n);
      }
   }

   return t;
}

void UTIL_Delay(int duration)
{
   unsigned int begin = SDL_GetTicks();

   while (SDL_GetTicks() - begin < (unsigned int)duration) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
               // Quit the program immediately if user pressed ESC
               UserQuit();
            }
            break;
         } else if (event.type == SDL_QUIT) {
            UserQuit();
         }
      }
      SDL_Delay(5);
   }
}

