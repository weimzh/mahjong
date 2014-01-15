//
// Copyright (c) 2005, Wei Mingzhi <whistler_wmz@users.sf.net>
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

#ifndef GENERAL_H_
#define GENERAL_H_

#define GIRL_GRAYSTYLE    0
#define GIRL_COLOR        1
#define GIRL_FULL         2

class CTile;

// Draw which kind of tile?
enum
{
   PLAYER_SHOWN = 0,
   COMPUTER_SHOWN,
   WALL_SHOWN,
   PLAYER_CONCEALED = 100,
   COMPUTER_CONCEALED,
   WALL_CONCEALED,
};

// Sound ID
enum
{
   SND_DISCARD1 = 0,
   SND_DISCARD2,
   SND_DING,
   SND_BOOM,
   SND_FLASH,
   SND_SOUND1,
   SND_SOUND2,
   SND_SOUND3,
   SND_SOUND4,
   SND_REACH,
   SND_FIRE,
};

// Mouse cursor
enum
{
   CURSOR_HAND = 0,
   CURSOR_POINTER,
   CURSOR_HIDDEN,
};

#define TILE_WIDTH                42
#define TILE_HEIGHT_CONCEALED     76
#define TILE_HEIGHT_SHOWN         60
#define TILE_HEIGHT_WALL          60
#define TILE_HEIGHT_COMPUTER      70

class CGeneral
{
public:
   CGeneral();
   virtual ~CGeneral();

   // color: 0 - graystyle, 1 - color, 2 - full girl
   void               DrawBG(int girlnum = 0, int color = 0);
   void               DrawBGFade(int girlnum = 0, int color = 0, int duration = 200);
   void               EraseArea(int x, int y, int w, int h, int girlnum = -1, int color = -1);
   void               ScreenFade(int duration = 500, SDL_Surface *s = NULL);
   void               DrawGirl(int x, int y, int w, int h, int girlnum = 0, int color = 0);

   void               WaitForAnyKey();
   SDLKey             ReadKey();

   void               GameOver();
   void               BonusGame();

   void               DrawText(const char *sz, int x, int y, int size = 0, int r = 255, int g = 255, int b = 255);
   void               DrawUTF8Text(const char *sz, int x, int y, int size = 0, int r = 255, int g = 255, int b = 255);

   void               DrawMessage(const char *sz);

   void               DrawTile(const CTile &t, int x, int y, int dir = PLAYER_SHOWN, int size = 1);
   void               DrawTiles(const CTile t[], int num, int x, int y, int dir = PLAYER_SHOWN, int size = 1);
   void               DrawDotBar(int x, int y, bool anim = false);
   void               DrawTurn(int turn, int x, int y);

   void               UpdateScreen(int x = 0, int y = 0, int w = 0, int h = 0);

   void               PlayMusic(Mix_Music *m, int loop = -1, int volume = -1);
   void               PlaySound(Mix_Chunk *s, int volume = -1);

   void               PlaySound(int num, int volume = -1);
   void               PlayBGMusic(int num = 0, int volume = -1);
   void               PlayWinMusic(int volume = -1);
   void               PlayEndRoundMusic(int volume = -1);

   void               Fire();
   void               AnimSelfDrawn(const CTile &t);
   void               AnimOut(const CTile &t);

private:
   SDL_Surface       *m_imgGirls[4][3], *m_imgGameOver, *m_imgElectron;
   SDL_Surface       *m_imgTiles, *m_imgBonusGame, *m_imgHand;
   SDL_Surface       *m_imgFire1, *m_imgFire2, *m_imgFlash1, *m_imgFlash2;

   TTF_Font          *m_fontKai, *m_fontBrush;     // small fonts (24pt)
   TTF_Font          *m_fontKaiL, *m_fontBrushL;   // large fonts (32pt)
   TTF_Font          *m_fontKaiH, *m_fontBrushH;   // large fonts (72pt)

   Mix_Music         *m_musBG[7], *m_musGameOver, *m_musWin, *m_musEndRound, *m_musBGame;
   Mix_Chunk         *m_snd[11];

   SDL_Cursor        *m_HandCursor;

   int                m_iPrevGirl, m_iPrevGirlColor;
   int                m_iCurMusic;

   void               LoadImages();
   void               DeleteImages();
   void               LoadFonts();
   void               DeleteFonts();
   void               LoadMusic();
   void               DeleteMusic();

   void               LoadSound();
   void               DeleteSound();

   void               InitCursor();
   void               DeleteCursor();

   SDL_Surface       *LoadImgFile(const char *filename);
   TTF_Font          *LoadFontFile(const char *filename, int size);
   Mix_Music         *LoadMusicFile(const char *filename);
   Mix_Chunk         *LoadSoundFile(const char *filename);
};

extern CGeneral *gpGeneral;

#endif

