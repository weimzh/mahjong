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

#include "main.h"

CGeneral *gpGeneral = NULL;

CGeneral::CGeneral():
m_iPrevGirl(0),
m_iPrevGirlColor(0),
m_iCurMusic(-1)
{
   LoadImages();
   LoadFonts();
   LoadMusic();
   LoadSound();
   InitCursor();
}

CGeneral::~CGeneral()
{
   DeleteImages();
   DeleteFonts();
   DeleteMusic();
   DeleteSound();
   DeleteCursor();
}

void CGeneral::DrawBG(int girlnum, int color)
{
   if (girlnum < 0 || girlnum > 4 || color < 0 || color > 2) {
      TerminateOnError("CGeneral::DrawBG(): invalid parameter");
   }

   SDL_Surface *bg = ((girlnum >= 4) ? m_imgElectron : m_imgGirls[girlnum][color]);
   assert(bg != NULL);

   // display the image
   SDL_BlitSurface(bg, NULL, gpScreen, NULL);

   m_iPrevGirl = girlnum;

   m_iPrevGirlColor = color;
}

void CGeneral::DrawBGFade(int girlnum, int color, int duration)
{
   if (girlnum < 0 || girlnum > 4 || color < 0 || color > 2) {
      TerminateOnError("CGeneral::DrawBGFade(): invalid parameter");
   }

   SDL_Surface *bg = ((girlnum >= 4) ? m_imgElectron : m_imgGirls[girlnum][color]);
   assert(bg != NULL);

   // display the image
   ScreenFade(duration, bg);

   m_iPrevGirl = girlnum;
   m_iPrevGirlColor = color;
}

void CGeneral::EraseArea(int x, int y, int w, int h, int girlnum, int color)
{
   if (girlnum < 0 || girlnum > 3) {
      girlnum = m_iPrevGirl;
      color = m_iPrevGirlColor;
   }

   SDL_Rect dstrect;

   dstrect.x = x;
   dstrect.y = y;
   dstrect.w = w;
   dstrect.h = h;

   SDL_Surface *bg = m_imgGirls[girlnum][color];
   assert(bg != NULL);

   // display the image
   SDL_BlitSurface(bg, &dstrect, gpScreen, &dstrect);
}

void CGeneral::ScreenFade(int duration, SDL_Surface *s)
{
   SDL_Surface *pNewFadeSurface = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      gpScreen->w, gpScreen->h, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   if (!pNewFadeSurface) {
      // cannot create surface, just blit the surface to the screen
      if (s != NULL) {
         SDL_BlitSurface(s, NULL, gpScreen, NULL);
         SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
      }
      return;
   }

   if (s == NULL) {
      // make black screen
      SDL_FillRect(pNewFadeSurface, NULL,
         SDL_MapRGB(pNewFadeSurface->format, 0, 0, 0));
   } else {
      SDL_BlitSurface(s, NULL, pNewFadeSurface, NULL);
   }

   if (SDL_MUSTLOCK(gpScreen)) {
      if (SDL_LockSurface(gpScreen) < 0) {
         // cannot lock screen, just blit the surface to the screen
         if (s != NULL) {
            SDL_BlitSurface(s, NULL, gpScreen, NULL);
            SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
         }
         return;
      }
   }

   const unsigned long size = gpScreen->pitch * gpScreen->h;
   unsigned char *fadeFromRGB = (unsigned char *)calloc(size, 1);
   unsigned char *fadeToRGB = (unsigned char *)calloc(size, 1);
   if (fadeFromRGB == NULL || fadeToRGB == NULL) {
      TerminateOnError("Memory allocation error !");
   }

   memcpy(fadeFromRGB, gpScreen->pixels, size);
   memcpy(fadeToRGB, pNewFadeSurface->pixels, size);

   int first = SDL_GetTicks(), now = first;

   do {
      // The +50 is to allow first frame to show some change
      float ratio = (now - first + 50) / (float)duration;
      const unsigned char amount = (unsigned char)(ratio * 255);
      const unsigned char oldamount = 255 - amount;
      unsigned char *pw = (unsigned char *)gpScreen->pixels;
      unsigned char *stop = pw + size;
      unsigned char *from = fadeFromRGB;
      unsigned char *to = fadeToRGB;

      do {
         //dividing by 256 instead of 255 provides huge optimization
         *pw = (oldamount * *(from++) + amount * *(to++)) / 256;
      } while (++pw != stop);

      now = SDL_GetTicks();
      SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
   } while (now - first + 50 < duration);

   free(fadeFromRGB);
   free(fadeToRGB);

   SDL_BlitSurface(pNewFadeSurface, NULL, gpScreen, NULL);
   SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);

   if (SDL_MUSTLOCK(gpScreen))
      SDL_UnlockSurface(gpScreen);

   SDL_FreeSurface(pNewFadeSurface);
}

void CGeneral::DrawGirl(int x, int y, int w, int h, int girlnum, int color)
{
   if (girlnum < 0 || girlnum > 3) {
      TerminateOnError("CGeneral::DrawGirl(): invalid parameter");
   }

   SDL_Rect dstrect, dstrect2;

   dstrect.x = dstrect.y = 0;
   dstrect.w = 640;
   dstrect.h = 480;

   dstrect2.x = x;
   dstrect2.y = y;
   dstrect2.w = w;
   dstrect2.h = h;

   SDL_Surface *bg = m_imgGirls[girlnum][color];
   assert(bg != NULL);

   UTIL_ScaleBlit(bg, &dstrect, gpScreen, &dstrect2);
}

void CGeneral::WaitForAnyKey()
{
   SDL_Event event;

   while (SDL_PollEvent(&event)) {
   }

   while (1) {
      if (SDL_WaitEvent(&event)) {
         if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
               // Quit the program immediately if user pressed ESC
               UserQuit();
            }
            break;
         } else if (event.type == SDL_QUIT) {
            UserQuit();
         } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            break;
         }
      }
   }
}

SDLKey CGeneral::ReadKey()
{
   SDL_Event event;

   while (SDL_PollEvent(&event)) {
   }

   while (1) {
      if (SDL_WaitEvent(&event)) {
         if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
               // Quit the program immediately if user pressed ESC
               UserQuit();
            }
            break;
         } else if (event.type == SDL_QUIT) {
            UserQuit();
         } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_RIGHT) {
               return SDLK_RIGHT;
            } else if (event.button.button == SDL_BUTTON_LEFT) {
               if (m_iCurMusic == 4) {
                  if (event.button.y < 240) {
                     if (event.button.x < 320) {
                        return SDLK_a;
                     } else {
                        return SDLK_b;
                     }
                  } else {
                     if (event.button.x < 320) {
                        return SDLK_c;
                     } else {
                        return SDLK_d;
                     }
                  }
               }

               if (event.button.y > (m_iCurMusic == 8 ? 260 : 380)) {
                  SDLKey r = (SDLKey)(SDLK_a + (event.button.x - 20) / TILE_WIDTH);
                  if (r > SDLK_n && r <= SDLK_z) {



                     return SDLK_n;
                  }
                  return r;
               }

               return SDLK_RETURN;
            }
         }
      }
   }

   return event.key.keysym.sym;
}

void CGeneral::GameOver()
{
   int iCurSel = 0;
   SDL_Event event;

   // Draw the background
   SDL_BlitSurface(m_imgGameOver, NULL, gpScreen, NULL);

   // Draw the text
   DrawText("GAME OVER", 22, 22, 1, 255, 255, 0);
   DrawUTF8Text(va("%s?", msg("Continue")), 400, 300, 1, 255, 255, 0);
   DrawUTF8Text(msg("Yes"), 450, 350, 1, 255, 255, 0);
   DrawUTF8Text(msg("No"), 450, 390, 1, 255, 255, 0);

   // Play the "Game Over" music
   PlayMusic(m_musGameOver, 0);

   while (1) {
      // Erase the original arrow
      SDL_Rect dstrect;
      dstrect.x = 400;
      dstrect.y = 350;
      dstrect.w = 32;
      dstrect.h = 80;

      SDL_BlitSurface(m_imgGameOver, &dstrect, gpScreen, &dstrect);

      // Draw the new arrow
      DrawText(">", 400, iCurSel ? 390 : 350, 1, 255, 255, 0);

      UpdateScreen();

      if (SDL_WaitEvent(&event)) {
         if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_n:
               // Quit the program immediately if user pressed ESC
               PlayMusic(NULL);
               UserQuit();
               break;

            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_LEFT:
            case SDLK_RIGHT:
               iCurSel ^= 1;
               break;

            case SDLK_y:
               PlayMusic(NULL);
               PlaySound(SND_SOUND1);
               UTIL_Delay(2000);
               ScreenFade(300);
               return;

            case SDLK_KP_ENTER:
            case SDLK_RETURN:
            case SDLK_SPACE:
               PlayMusic(NULL);
               if (iCurSel == 0) {
                  PlaySound(SND_SOUND1);
                  UTIL_Delay(2000);
                  ScreenFade(300);
                  return;
               } else {
                  UserQuit();
               }
               break;
            }
         } else if (event.type == SDL_QUIT) {
            PlayMusic(NULL);
            UserQuit();
         }
      }
   }

   PlayMusic(NULL);
   UserQuit();
}

void CGeneral::BonusGame()
{
   int i, j, k;
   char dat[] = {0xe9, 0x85, 0x8d, 0x00,
                 0xe7, 0x89, 0x8c, 0x00,
                 0xe5, 0x87, 0xba, 0x00,
                 0xe8, 0xa1, 0x80, 0x00,
                 0xe5, 0xa5, 0x96, 0xe5,
                 0x8a, 0xb1, 0xe6, 0xb8,
                 0xb8, 0xe6, 0x88, 0x8f, 0x00};

   SDL_BlitSurface(m_imgBonusGame, NULL, gpScreen, NULL);
   UpdateScreen();
   PlayMusic(m_musBGame, 0);
   UTIL_Delay(6000);

   bool locked = false;
   if (SDL_MUSTLOCK(gpScreen)) {
      SDL_LockSurface(gpScreen);
      locked = true;
   }

   for (i = 0; i < 640; i++) {
      for (j = 0; j < 480; j++) {
         unsigned char r, g, b;

         k = 0;
         UTIL_GetPixel(gpScreen, i, j, &r, &g, &b);
         k += r;
         k += g;
         k += b;
         k /= 3;
         UTIL_PutPixel(gpScreen, i, j, k, k, k);
      }
   }

   if (locked) {
      SDL_UnlockSurface(gpScreen);
   }

   for (i = 0; i < 4; i++) {
      DrawUTF8Text(&dat[i * 4], 150 + i * 100, 100, 2, 255, 255, 0);
      UpdateScreen();
      PlaySound(SND_SOUND1);
      UTIL_Delay(1300);
   }

   DrawUTF8Text(&dat[i * 4], 175, 200, 2, 0, 255, 255);

   PlaySound(SND_DISCARD2);
   UpdateScreen();
   UTIL_Delay(2500);
   ScreenFade();
}

void CGeneral::DrawText(const char *sz, int x, int y, int size, int r, int g, int b)
{
   SDL_Surface *text;
   SDL_Rect dstrect;
   SDL_Color textcolor = {r, g, b, 0};
   SDL_Color black = {0x10, 0x8, 0x8, 0};

   TTF_Font *f = (size == 0 ? m_fontBrush : (size == 1 ? m_fontBrushL : m_fontBrushH));

   text = TTF_RenderText_Blended(f, sz, black);
   dstrect.x = x + 2;
   dstrect.y = y + 2;

   SDL_BlitSurface(text, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(text);

   text = TTF_RenderText_Blended(f, sz, textcolor);
   dstrect.x = x;
   dstrect.y = y;

   SDL_BlitSurface(text, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(text);
}

void CGeneral::DrawUTF8Text(const char *sz, int x, int y, int size, int r, int g, int b)
{
   SDL_Surface *text;
   SDL_Rect dstrect;
   SDL_Color textcolor = {r, g, b, 0};
   SDL_Color black = {0x10, 0x8, 0x8, 0};

   TTF_Font *f = (size == 0 ? m_fontKai : (size == 1 ? m_fontKaiL : m_fontKaiH));

   text = TTF_RenderUTF8_Blended(f, sz, black);
   dstrect.x = x + 2;
   dstrect.y = y + 2;

   SDL_BlitSurface(text, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(text);

   text = TTF_RenderUTF8_Blended(f, sz, textcolor);
   dstrect.x = x;
   dstrect.y = y;

   SDL_BlitSurface(text, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(text);
}

void CGeneral::DrawMessage(const char *sz)
{
   SDL_Rect dstrect, dstrect2;

   dstrect.x = 150;
   dstrect.y = 300;
   dstrect.w = 340;
   dstrect.h = 60;

   static SDL_Surface *saved = NULL;

   if (sz == NULL) {
      // Restore the screen
      if (saved != NULL) {
         SDL_BlitSurface(saved, NULL, gpScreen, &dstrect);
         SDL_FreeSurface(saved);
         saved = NULL;

         SDL_UpdateRect(gpScreen, 150, 300, 340, 60);
      }
      return;
   }

   // Save the current screen if no screen is saved
   if (saved == NULL) {
      saved = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
         340, 60, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
         gpScreen->format->Gmask, gpScreen->format->Bmask,
         gpScreen->format->Amask);

      SDL_SetAlpha(saved, 0, 0);
      SDL_BlitSurface(gpScreen, &dstrect, saved, NULL);
   }

   // Draw a box to the screen
   SDL_Surface *temp = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      340, 60, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   SDL_SetAlpha(temp, 0, 0);

   SDL_FillRect(temp, NULL, SDL_MapRGB(temp->format, 100, 100, 100));

   dstrect2.x = 5;
   dstrect2.y = 5;
   dstrect2.w = 330;
   dstrect2.h = 50;

   SDL_FillRect(temp, &dstrect2, SDL_MapRGB(temp->format, 20, 100, 190));

   SDL_BlitSurface(temp, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(temp);

   int y = 305, t;

   char *line = UTIL_StrGetLine(sz, 26, t);
   if (strlen(line) >= strlen(sz)) {
      y += 12;
   } else {
      sz += t;
      DrawUTF8Text(line, 160, y);
      y += 26;
      line = UTIL_StrGetLine(sz, 26, t);
   }

   DrawUTF8Text(line, 160, y);

   SDL_UpdateRect(gpScreen, 150, 300, 340, 60);

   PlaySound(SND_DING); // play a hint sound
}

void CGeneral::DrawTile(const CTile &t, int x, int y, int dir, int size)
{
   SDL_Rect dstrect, dstrect2;

   dstrect.x = x;
   dstrect.y = y;

   if (dir <= PLAYER_CONCEALED) {
      // shown tile
      dstrect.w = dstrect2.w = TILE_WIDTH;
      dstrect.h = dstrect2.h = ((dir < PLAYER_CONCEALED) ?
         TILE_HEIGHT_SHOWN : TILE_HEIGHT_CONCEALED);

      if (t.GetSuit() == TILESUIT_DRAGON) {
         dstrect2.x = (t.index2() + 4) * TILE_WIDTH;
         dstrect2.y = 3 * dstrect.h;
      } else {
         dstrect2.x = t.index2() * TILE_WIDTH;
         dstrect2.y = t.index1() * dstrect.h;
      }

      if (dir < PLAYER_CONCEALED) {
         dstrect2.y += TILE_HEIGHT_CONCEALED * 4;
      }
   } else if (dir == COMPUTER_CONCEALED) {
      // concealed tile
      dstrect2.x = 0;
      dstrect2.y = TILE_HEIGHT_SHOWN * 4 + TILE_HEIGHT_CONCEALED * 4;
      dstrect2.w = dstrect.w = TILE_WIDTH;
      dstrect2.h = dstrect.h = TILE_HEIGHT_COMPUTER;
   } else if (dir == WALL_CONCEALED) {
      dstrect2.x = TILE_WIDTH;
      dstrect2.y = TILE_HEIGHT_SHOWN * 4 + TILE_HEIGHT_CONCEALED * 4;
      dstrect2.w = dstrect.w = TILE_WIDTH;
      dstrect2.h = dstrect.h = TILE_HEIGHT_WALL;
   }

   // Draw the tile to the screen
   if (size >= 1) {
      SDL_BlitSurface(m_imgTiles, &dstrect2, gpScreen, &dstrect);
   } else {
      dstrect.w = dstrect.w * 7 / 10;
      dstrect.h = dstrect.h * 7 / 10;
      UTIL_ScaleBlit(m_imgTiles, &dstrect2, gpScreen, &dstrect);
   }
}

void CGeneral::DrawTiles(const CTile t[], int num, int x, int y, int dir, int size)
{
   int i;

   if (dir == COMPUTER_SHOWN) {
      // Draw computer's tiles in reverse order
      for (i = num - 1; i >= 0; i--) {
         DrawTile(t[i], (int)(x + (num - i - 1) * TILE_WIDTH * (size ? 1 : 0.7)),
            y, dir, size);
      }
   } else {
      // Otherwise draw in normal order
      for (i = 0; i < num; i++) {
         DrawTile(t[i], (int)(x + i * TILE_WIDTH * (size ? 1 : 0.7)), y, dir, size);
      }
   }
}

void CGeneral::DrawDotBar(int x, int y, bool anim)
{
   SDL_Rect dstrect, dstrect2;

   dstrect.x = x;
   dstrect2.x = 359;
   dstrect.w = dstrect2.w = 20;

   if (anim) {
      int i;
      for (i = 0; i < 20; i++) {
         dstrect.y = y + (20 - i) * 85 / 40;
         dstrect2.y = 531 + (20 - i) * 85 / 40;
         dstrect.h = dstrect2.h = i * 85 / 20;
         SDL_BlitSurface(m_imgTiles, &dstrect2, gpScreen, &dstrect);
         UpdateScreen(dstrect.x, dstrect.y, dstrect.w, dstrect.h);
         UTIL_Delay(10);
      }
   }

   dstrect.h = dstrect2.h = 85;

   dstrect.y = y;
   dstrect2.y = 531;
   SDL_BlitSurface(m_imgTiles, &dstrect2, gpScreen, &dstrect);
}

void CGeneral::DrawTurn(int turn, int x, int y)
{
   SDL_Rect dstrect, dstrect2;

   dstrect.x = x;
   dstrect.y = y;
   dstrect.w = dstrect2.w = 26;
   dstrect.h = dstrect2.h = 53;

   dstrect2.x = TILE_WIDTH * 2;
   dstrect2.x += ((turn == 0) ? 0 : TILE_WIDTH);
   dstrect2.y = TILE_HEIGHT_SHOWN * 4 + TILE_HEIGHT_CONCEALED * 4;

   SDL_BlitSurface(m_imgTiles, &dstrect2, gpScreen, &dstrect);
}

void CGeneral::UpdateScreen(int x, int y, int w, int h)
{
   SDL_UpdateRect(gpScreen, x, y, w, h);
}

void CGeneral::PlayMusic(Mix_Music *m, int loop, int volume)
{
   if (g_fNoMusic) {
      return;
   }

   if (m == NULL) {
      Mix_HaltMusic(); // stop playing any music
      return;
   }

   if (volume == -1) {
      float f = atof(cfg.Get("OPTIONS", "MusicVolume", "1"));
      if (f < 0) {
         f = 0;
      } else if (f > 1) {
         f = 1;
      }
      volume = (int)(MIX_MAX_VOLUME * f);
   }

   Mix_VolumeMusic(volume);
   Mix_PlayMusic(m, loop);
}

void CGeneral::PlaySound(Mix_Chunk *s, int volume)
{
   if (g_fNoSound) {
      return;
   }

   if (volume == -1) {
      float f = atof(cfg.Get("OPTIONS", "SoundVolume", "1"));
      if (f < 0) {
         f = 0;
      } else if (f > 1) {
         f = 1;
      }
      volume = (int)(MIX_MAX_VOLUME * f);
   }

   Mix_VolumeChunk(s, volume);
   Mix_PlayChannel(-1, s, 0);
}

void CGeneral::PlaySound(int num, int volume)
{
   if (g_fNoSound) {
      return;
   }

   if (volume == -1) {
      float f = atof(cfg.Get("OPTIONS", "SoundVolume", "1"));
      if (f < 0) {
         f = 0;
      } else if (f > 1) {
         f = 1;
      }
      volume = (int)(MIX_MAX_VOLUME * f);
   }

   PlaySound(m_snd[num], volume);
}

void CGeneral::PlayBGMusic(int num, int volume)
{
   m_iCurMusic = num;

   if (g_fNoMusic) {
      return;
   }

   if (num < 0 || num > 5) {
      PlayMusic(NULL); // stop playing any music
   }

   PlayMusic(m_musBG[num], -1, volume);
}

void CGeneral::PlayWinMusic(int volume)
{
   if (g_fNoMusic) {
      return;
   }

   PlayMusic(m_musWin, 1, volume);
}

void CGeneral::PlayEndRoundMusic(int volume)
{
   m_iCurMusic = 8;

   if (g_fNoMusic) {
      return;
   }

   PlayMusic(m_musEndRound, -1, volume);
}

void CGeneral::Fire()
{
   int i;
   SDL_Surface *save = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      575, 130, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask, gpScreen->format->Amask);

   SDL_Rect dstrect;
   dstrect.x = 5;
   dstrect.y = 470 - 130;
   dstrect.w = 575;
   dstrect.h = 130;

   SDL_BlitSurface(gpScreen, &dstrect, save, NULL);

   UTIL_Delay(400);
   PlaySound(SND_FIRE);

   for (i = 0; i < 6; i++) {
      dstrect.y = 470 - 125;
      dstrect.h = 125;

      SDL_BlitSurface(m_imgFire1, NULL, gpScreen, &dstrect);
      UpdateScreen(0, 470 - 130, 575, 130);
      UTIL_Delay(100);

      dstrect.y = 470 - 130;
      dstrect.h = 130;

      SDL_BlitSurface(save, NULL, gpScreen, &dstrect);
      SDL_BlitSurface(m_imgFire2, NULL, gpScreen, &dstrect);
      UpdateScreen(0, 470 - 130, 575, 130);
      UTIL_Delay(100);

      SDL_BlitSurface(save, NULL, gpScreen, &dstrect);
   }
   UpdateScreen(0, 470 - 130, 575, 130);
   SDL_FreeSurface(save);
   UTIL_Delay(100);
}

void CGeneral::AnimSelfDrawn(const CTile &t)
{
   EraseArea(20, 265, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 265);
   UpdateScreen(20, 265, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 265);
   int i;
   SDL_Rect dstrect;

   PlaySound(SND_FLASH);
   for (i = 0; i < 12; i++) {
      EraseArea(20, 265, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 265);
      EraseArea(dstrect.x, dstrect.y, dstrect.w, 30);

      dstrect.x = 245;
      dstrect.y = 235;
      dstrect.w = m_imgFlash1->w;
      dstrect.h = m_imgFlash1->h;

      SDL_BlitSurface(m_imgFlash1, NULL, gpScreen, &dstrect);
      DrawTile(t, 280, 255);
      UpdateScreen(20, 235, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 235);

      UTIL_Delay(50);

      EraseArea(20, 265, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 265);
      EraseArea(dstrect.x, dstrect.y, dstrect.w, 30);

      dstrect.w = m_imgFlash2->w;
      dstrect.h = m_imgFlash2->h;

      SDL_BlitSurface(m_imgFlash2, NULL, gpScreen, &dstrect);
      DrawTile(t, 280, 255);
      UpdateScreen(20, 235, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 235);
      UTIL_Delay(50);
   }

   UTIL_Delay(20);
   EraseArea(20, 265, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 265);
   EraseArea(dstrect.x, dstrect.y, dstrect.w, 30);

   dstrect.w = m_imgFlash1->w;
   dstrect.h = m_imgFlash1->h;

   SDL_BlitSurface(m_imgFlash1, NULL, gpScreen, &dstrect);
   DrawTile(t, 280, 255);
   DrawUTF8Text(msg("out_selfdrawn"), 215, 265, 2, 255, 255, 255);
   UpdateScreen(20, 205, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 205);
   UTIL_Delay(1000);
   EraseArea(20, 265, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 265);
   EraseArea(dstrect.x, dstrect.y, dstrect.w, 30);
   UpdateScreen(20, 205, TILE_WIDTH * 10, 303 + TILE_HEIGHT_SHOWN - 205);
}

void CGeneral::AnimOut(const CTile &t)
{
   EraseArea(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 94);
   UpdateScreen(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 64);
   int i;
   SDL_Rect dstrect, dhand;

   SDL_Surface *save = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      m_imgHand->w, m_imgHand->h, gpScreen->format->BitsPerPixel,
      gpScreen->format->Rmask, gpScreen->format->Gmask,
      gpScreen->format->Bmask, gpScreen->format->Amask);

   dhand.x = 218;
   dhand.y = 125;
   dhand.w = m_imgHand->w;
   dhand.h = m_imgHand->h;

   SDL_BlitSurface(gpScreen, &dhand, save, NULL);
   SDL_BlitSurface(m_imgHand, NULL, gpScreen, &dhand);
   UpdateScreen(dhand.x, dhand.y, dhand.w, dhand.h);

   PlaySound(SND_FLASH);
   dhand.h = m_imgFlash2->h - (125 - 94);
   for (i = 0; i < 10; i++) {
      EraseArea(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 94);
      EraseArea(245, 94, m_imgFlash2->w, m_imgFlash2->h);

      dstrect.x = 245;
      dstrect.y = 94;
      dstrect.w = m_imgFlash1->w;
      dstrect.h = m_imgFlash1->h;

      SDL_BlitSurface(m_imgFlash1, NULL, gpScreen, &dstrect);
      DrawTile(t, 280, 104);
      SDL_BlitSurface(m_imgHand, NULL, gpScreen, &dhand);
      UpdateScreen(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 64);

      UTIL_Delay(50);

      EraseArea(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 94);
      EraseArea(dstrect.x, dstrect.y, dstrect.w, dstrect.h);

      dstrect.w = m_imgFlash2->w;
      dstrect.h = m_imgFlash2->h;
      SDL_BlitSurface(m_imgFlash2, NULL, gpScreen, &dstrect);
      DrawTile(t, 280, 104);
      SDL_BlitSurface(m_imgHand, NULL, gpScreen, &dhand);
      UpdateScreen(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 64);
      UTIL_Delay(50);
   }

   dhand.h = m_imgHand->h;
   UTIL_Delay(120);
   DrawUTF8Text(msg("out_discard"), 215, 114, 2, 255, 255, 255);
   UpdateScreen(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 64);
   UTIL_Delay(1000);
   EraseArea(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 94);
   EraseArea(dstrect.x, dstrect.y, dstrect.w, dstrect.h);
   SDL_BlitSurface(save, NULL, gpScreen, &dhand);
   SDL_FreeSurface(save);
   UpdateScreen(620 - 10 * TILE_WIDTH, 94, TILE_WIDTH * 10, 132 + TILE_HEIGHT_SHOWN - 64);
   UpdateScreen(dhand.x, dhand.y, dhand.w, dhand.h);
}

void CGeneral::LoadImages()
{
   m_imgGirls[0][GIRL_COLOR] = LoadImgFile(IMAGES_DIR "mjgirl1b.jpg");
   m_imgGirls[0][GIRL_GRAYSTYLE] = UTIL_GrayStyle(m_imgGirls[0][GIRL_COLOR]);
   m_imgGirls[0][GIRL_FULL] = LoadImgFile(IMAGES_DIR "mjgirl1c.jpg");
   m_imgGirls[1][GIRL_COLOR] = LoadImgFile(IMAGES_DIR "mjgirl2b.jpg");
   m_imgGirls[1][GIRL_GRAYSTYLE] = UTIL_GrayStyle(m_imgGirls[1][GIRL_COLOR]);
   m_imgGirls[1][GIRL_FULL] = LoadImgFile(IMAGES_DIR "mjgirl2c.jpg");
   m_imgGirls[2][GIRL_COLOR] = LoadImgFile(IMAGES_DIR "mjgirl3b.jpg");
   m_imgGirls[2][GIRL_GRAYSTYLE] = UTIL_GrayStyle(m_imgGirls[2][GIRL_COLOR]);
   m_imgGirls[2][GIRL_FULL] = LoadImgFile(IMAGES_DIR "mjgirl3c.jpg");
   m_imgGirls[3][GIRL_COLOR] = LoadImgFile(IMAGES_DIR "mjgirl4b.jpg");
   m_imgGirls[3][GIRL_GRAYSTYLE] = UTIL_GrayStyle(m_imgGirls[3][GIRL_COLOR]);
   m_imgGirls[3][GIRL_FULL] = LoadImgFile(IMAGES_DIR "mjgirl4c.jpg");

   m_imgElectron = LoadImgFile(IMAGES_DIR "electron.jpg");
   m_imgGameOver = LoadImgFile(IMAGES_DIR "gameover.jpg");
   m_imgTiles = LoadImgFile(IMAGES_DIR "tiles.jpg");

   m_imgBonusGame = LoadImgFile(IMAGES_DIR "bgame.jpg");

   m_imgFire1 = LoadImgFile(IMAGES_DIR "fire1.bmp");
   m_imgFire2 = LoadImgFile(IMAGES_DIR "fire2.bmp");
   m_imgFlash1 = LoadImgFile(IMAGES_DIR "flash1.bmp");
   m_imgFlash2 = LoadImgFile(IMAGES_DIR "flash2.bmp");
   m_imgHand = LoadImgFile(IMAGES_DIR "hand.bmp");

   SDL_SetColorKey(m_imgFire1, SDL_SRCCOLORKEY, SDL_MapRGBA(m_imgFire1->format, 98, 101, 98, 0));
   SDL_SetColorKey(m_imgFire2, SDL_SRCCOLORKEY, SDL_MapRGBA(m_imgFire2->format, 98, 101, 98, 0));
   SDL_SetColorKey(m_imgFlash1, SDL_SRCCOLORKEY, SDL_MapRGBA(m_imgFlash1->format, 98, 101, 98, 0));
   SDL_SetColorKey(m_imgFlash2, SDL_SRCCOLORKEY, SDL_MapRGBA(m_imgFlash2->format, 98, 101, 98, 0));
   SDL_SetColorKey(m_imgHand, SDL_SRCCOLORKEY, SDL_MapRGBA(m_imgHand->format, 98, 101, 98, 0));
}

void CGeneral::DeleteImages()
{
   SDL_FreeSurface(m_imgGirls[0][GIRL_GRAYSTYLE]);
   SDL_FreeSurface(m_imgGirls[0][GIRL_COLOR]);
   SDL_FreeSurface(m_imgGirls[0][GIRL_FULL]);
   SDL_FreeSurface(m_imgGirls[1][GIRL_GRAYSTYLE]);
   SDL_FreeSurface(m_imgGirls[1][GIRL_COLOR]);
   SDL_FreeSurface(m_imgGirls[1][GIRL_FULL]);
   SDL_FreeSurface(m_imgGirls[2][GIRL_GRAYSTYLE]);
   SDL_FreeSurface(m_imgGirls[2][GIRL_COLOR]);
   SDL_FreeSurface(m_imgGirls[2][GIRL_FULL]);
   SDL_FreeSurface(m_imgGirls[3][GIRL_GRAYSTYLE]);
   SDL_FreeSurface(m_imgGirls[3][GIRL_COLOR]);
   SDL_FreeSurface(m_imgGirls[3][GIRL_FULL]);

   SDL_FreeSurface(m_imgGameOver);
   SDL_FreeSurface(m_imgTiles);

   SDL_FreeSurface(m_imgBonusGame);
   SDL_FreeSurface(m_imgFire1);
   SDL_FreeSurface(m_imgFire2);
}

void CGeneral::LoadFonts()
{
   // Small fonts
   m_fontKai = LoadFontFile(FONTS_DIR "gkai00mp.ttf", 24); // Chinese fonts (Kai Ti)
   m_fontBrush = LoadFontFile(FONTS_DIR "brush.ttf", 24); // English/Number fonts

   // Large fonts
   m_fontKaiL = LoadFontFile(FONTS_DIR "gkai00mp.ttf", 36); // Chinese fonts (Kai Ti)
   m_fontBrushL = LoadFontFile(FONTS_DIR "brush.ttf", 36); // English/Number fonts

   // Huge fonts
   m_fontKaiH = LoadFontFile(FONTS_DIR "gkai00mp.ttf", 72); // Chinese fonts (Kai Ti)
   m_fontBrushH = LoadFontFile(FONTS_DIR "brush.ttf", 72); // English/Number fonts
}

void CGeneral::DeleteFonts()
{
   TTF_CloseFont(m_fontKai);
   TTF_CloseFont(m_fontBrush);
   TTF_CloseFont(m_fontKaiL);
   TTF_CloseFont(m_fontBrushL);
   TTF_CloseFont(m_fontKaiH);
   TTF_CloseFont(m_fontBrushH);
}

void CGeneral::LoadMusic()
{
   if (g_fNoMusic) {
      return;
   }

   m_musBG[0] = LoadMusicFile(MUSIC_DIR "music.ogg");
   m_musBG[1] = LoadMusicFile(MUSIC_DIR "music1.ogg");
   m_musBG[2] = LoadMusicFile(MUSIC_DIR "musicc.ogg");
   m_musBG[3] = LoadMusicFile(MUSIC_DIR "musicp.ogg");
   m_musBG[4] = LoadMusicFile(MUSIC_DIR "bet.ogg");
   m_musBG[5] = LoadMusicFile(MUSIC_DIR "bonus.ogg");
   m_musBG[6] = LoadMusicFile(MUSIC_DIR "musicb.ogg");

   m_musGameOver = LoadMusicFile(MUSIC_DIR "gameover.ogg");
   m_musWin = LoadMusicFile(MUSIC_DIR "win.ogg");
   m_musEndRound = LoadMusicFile(MUSIC_DIR "musice.ogg");
   m_musBGame = LoadMusicFile(MUSIC_DIR "bgame.ogg");

}

void CGeneral::DeleteMusic()
{
   if (g_fNoMusic) {
      return;
   }

   Mix_FreeMusic(m_musBG[0]);
   Mix_FreeMusic(m_musBG[1]);
   Mix_FreeMusic(m_musBG[2]);
   Mix_FreeMusic(m_musBG[3]);
   Mix_FreeMusic(m_musBG[4]);
   Mix_FreeMusic(m_musBG[5]);
   Mix_FreeMusic(m_musBG[6]);

   Mix_FreeMusic(m_musGameOver);
   Mix_FreeMusic(m_musWin);
   Mix_FreeMusic(m_musEndRound);
   Mix_FreeMusic(m_musBGame);
}

void CGeneral::LoadSound()
{
   if (g_fNoSound) {
      return;
   }

   m_snd[SND_DISCARD1] = LoadSoundFile(SOUND_DIR "discard.wav");
   m_snd[SND_DISCARD2] = LoadSoundFile(SOUND_DIR "discard2.wav");
   m_snd[SND_DING] = LoadSoundFile(SOUND_DIR "ding.wav");
   m_snd[SND_SOUND1] = LoadSoundFile(SOUND_DIR "snd1.wav");
   m_snd[SND_SOUND2] = LoadSoundFile(SOUND_DIR "snd2.wav");
   m_snd[SND_SOUND3] = LoadSoundFile(SOUND_DIR "snd3.wav");
   m_snd[SND_SOUND4] = LoadSoundFile(SOUND_DIR "snd4.wav");
   m_snd[SND_REACH] = LoadSoundFile(SOUND_DIR "reach.wav");

   m_snd[SND_BOOM] = LoadSoundFile(SOUND_DIR "boom.wav");
   m_snd[SND_FLASH] = LoadSoundFile(SOUND_DIR "flash.wav");
   m_snd[SND_FIRE] = LoadSoundFile(SOUND_DIR "fire.wav");
}

void CGeneral::DeleteSound()
{
   if (g_fNoSound) {
      return;
   }

   Mix_FreeChunk(m_snd[SND_DISCARD1]);
   Mix_FreeChunk(m_snd[SND_DISCARD2]);
   Mix_FreeChunk(m_snd[SND_DING]);
   Mix_FreeChunk(m_snd[SND_SOUND1]);
   Mix_FreeChunk(m_snd[SND_SOUND2]);
   Mix_FreeChunk(m_snd[SND_SOUND3]);
   Mix_FreeChunk(m_snd[SND_SOUND4]);
   Mix_FreeChunk(m_snd[SND_REACH]);
   Mix_FreeChunk(m_snd[SND_BOOM]);
   Mix_FreeChunk(m_snd[SND_FLASH]);
   Mix_FreeChunk(m_snd[SND_FIRE]);
}

SDL_Surface *CGeneral::LoadImgFile(const char *filename)
{
   SDL_Surface *pic = IMG_Load(filename);

   if (pic == NULL) {
      TerminateOnError("Cannot load Image file %s: %s", filename, SDL_GetError());
   }

   return pic;
}

TTF_Font *CGeneral::LoadFontFile(const char *filename, int size)
{
   TTF_Font *f = TTF_OpenFont(filename, size);

   if (f == NULL) {
      TerminateOnError("Cannot load font file %s (%dpt): %s",
         filename, size, SDL_GetError());
   }

   return f;
}

Mix_Music *CGeneral::LoadMusicFile(const char *filename)
{
   if (g_fNoMusic) {
      return NULL;
   }

   Mix_Music *m = Mix_LoadMUS(filename);

   if (m == NULL) {
      TerminateOnError("Cannot load music file %s: %s",
         filename, SDL_GetError());
   }

   return m;
}

Mix_Chunk *CGeneral::LoadSoundFile(const char *filename)
{
   if (g_fNoSound) {
      return NULL;
   }

   Mix_Chunk *s = Mix_LoadWAV(filename);

   if (s == NULL) {
      TerminateOnError("Cannot load sound file %s: %s",
         filename, SDL_GetError());
   }

   return s;
}

void CGeneral::InitCursor()
{
   char cursor_data[22 + 1][18 + 1] = {
      "*****  ***********",
      "**** )) **********",
      "**** )) **********",
      "**** )) **********",
      "**** )) **********",
      "**** ))   ********",
      "**** )) ))   *****",
      "**** )) )) ))  ***",
      "**** )) )) )) ) **",
      "   ) )) )) )) )) *",
      " ))  )))))))) )) *",
      " ))) ))))))))))) *",
      "* )) ))))))))))) *",
      "** ) ))))))))))) *",
      "** ))))))))))))) *",
      "*** )))))))))))) *",
      "*** ))))))))))) **",
      "**** )))))))))) **",
      "**** )))))))))) **",
      "***** )))))))) ***",
      "***** )))))))) ***",
      "*****          ***"
   };

   unsigned char data[24 * 3], mask[24 * 3];

   int i, j, index = -1;

   for (i = 0; i < 24 * 3; i++) {
      data[i] = mask[i] = 0;
   }

   for (i = 0; i < 22; i++) {
      for (j = 0; j < 24; j++) {
         if (j % 8 != 0) {
            data[index] <<= 1;
            mask[index] <<= 1;
         } else {
            index++;
         }

         if (j >= 18) {
            continue;
         }

         switch (cursor_data[i][j]) {
            case ')':
               mask[index] |= 1;
               break;

            case ' ':
               data[index] |= 1;
               mask[index] |= 1;
               break;
         }
      }
   }

   m_HandCursor = SDL_CreateCursor(data, mask, 24, 24, 0, 0);
   SDL_SetCursor(m_HandCursor);
}

void CGeneral::DeleteCursor()
{
   SDL_FreeCursor(m_HandCursor);
}

