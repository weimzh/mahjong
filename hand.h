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

#ifndef HAND_H
#define HAND_H

enum handtile_flags
{
   HT_NULL            = 0,

   HT_OPENKONG        = (1 << 0),
   HT_CLOSEDKONG      = (1 << 1),
   HT_OPENPUNG        = (1 << 2),
   HT_OPENCHOW        = (1 << 3),

   HT_OPEN            = (HT_OPENKONG | HT_CLOSEDKONG | HT_OPENPUNG | HT_OPENCHOW),

   HT_CLOSEDPUNG      = (1 << 4),
   HT_CLOSEDCHOW      = (1 << 5),
   HT_PAIR            = (1 << 6),

   HT_THIRTEENWONDERS = (1 << 7),

   HT_LOCKED          = (HT_OPEN | HT_CLOSEDPUNG | HT_CLOSEDCHOW | HT_PAIR),

   HT_JUSTGOT         = (1 << 8),
   HT_FROMOPPONENT    = (1 << 9),
   HT_TUMO            = (1 << 10),
   HT_RON             = (1 << 11),

   HT_TOCHOWUPPER     = (1 << 12),
   HT_TOCHOWMIDDLE    = (1 << 13),
   HT_TOCHOWLOWER     = (1 << 14),

   HT_TOCHOW          = (HT_TOCHOWUPPER | HT_TOCHOWMIDDLE | HT_TOCHOWLOWER),
};

enum chow_location
{
   CHOW_LOWER        = 0,
   CHOW_MIDDLE,
   CHOW_UPPER,
};

typedef struct handtile_s
{
   CTile           tile;
   unsigned int    flags;
   int             tileset;
} handtile_t;

typedef struct tileset_s
{
   CTile           first;
   int             type;
} tileset_t;

#define  MAX_HANDTILE    18  /* 14 tiles plus 4 possible kongs */
#define  MAX_TILESET     7
#define  MAX_CONCEALED   14

class CHand
{
public:
   CHand();
   virtual ~CHand();

   void             Sort(handtile_t *begin, handtile_t *end);
   void             Sort();

   void             AddRandomTile(int flag = 0);
   void             AddTile(const CTile &t, int flag = HT_JUSTGOT, int tileset = -1);
   bool             RemoveTile(const CTile &t, int filter = HT_OPEN);
   bool             RemoveTile(int index, int filter = HT_OPEN);
   bool             HasTile(const CTile &t, int filter = HT_OPEN);

   bool             Pung(const CTile &t, bool open = true, int flags = 0);
   bool             Chow(const CTile &t, int location, bool open = true, int flags = 0);
   bool             Kong(const CTile &t, bool open = true, int flags = 0);

   bool             CanPung(const CTile &t);
   bool             CanChow(const CTile &t, int location);
   bool             CanKong(const CTile &t, bool open = true);

   bool             GoMahjong();
   bool             GoMahjong(const CTile &t);

   bool             CanMahjong(const CTile &t);

   bool             IsReady();

   int              GetWaitingTile(CTile tiles[], int max = 13);

   bool             IsConcealed();

   void             Restore();

   inline void      NewHand() { m_iNumTiles = m_iNumTileSets = 0; }

   handtile_t       m_Tiles[MAX_HANDTILE];
   int              m_iNumTiles;

   tileset_t        m_TileSets[MAX_TILESET];
   int              m_iNumTileSets;

private:
   bool             ThirteenWonders();
   bool             SevenPairs();

   bool             IsThirteenWondersReady();
   bool             IsSevenPairsReady();

   int              GetWaitingTile(CTile tiles[], int max, int &current);
   int              GetThirteenWondersWaitingTile(CTile tiles[], int max, int &current);
   int              GetSevenPairsWaitingTile(CTile tiles[], int max, int &current);

   bool             GoMahjong_real();
};

#endif

