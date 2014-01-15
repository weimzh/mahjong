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

CHand::CHand():
m_iNumTiles(0),
m_iNumTileSets(0)
{
}

CHand::~CHand()
{
}

void CHand::Sort(handtile_t *begin, handtile_t *end)
{
   if (end <= begin)
      return;

   handtile_t pivot = *begin, *a = begin, *b = end;

   while (a < b) {
      while (a < b && b->tile >= pivot.tile)
         b--;
      *a = *b;
      while (a < b && a->tile <= pivot.tile)
         a++;
      *b = *a;
   }

   *a = pivot;

   Sort(begin, a - 1);
   Sort(a + 1, end);
}

void CHand::Sort()
{
   Sort(&m_Tiles[0], &m_Tiles[m_iNumTiles - 1]);
}

void CHand::AddRandomTile(int flag)
{
   if (m_iNumTiles >= MAX_HANDTILE) {
      TerminateOnError("CHand::GetRandomTile(): Max tile reached");
   }

   m_Tiles[m_iNumTiles].tile = CTile::RandomTile();
   m_Tiles[m_iNumTiles].flags = flag;
   m_Tiles[m_iNumTiles].tileset = -1;

   m_iNumTiles++;
}

void CHand::AddTile(const CTile &t, int flag, int tileset)
{
   if (m_iNumTiles >= MAX_HANDTILE) {
      TerminateOnError("CHand::GetRandomTile(): Max tile reached");
   }

   m_Tiles[m_iNumTiles].tile = t;
   m_Tiles[m_iNumTiles].flags = flag;
   m_Tiles[m_iNumTiles].tileset = tileset;

   m_iNumTiles++;
}

bool CHand::RemoveTile(const CTile &t, int filter)
{
   int i, j;

   for (i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & filter)
         continue; // skip open tiles
      if (m_Tiles[i].tile == t) {
         for (j = i; j < m_iNumTiles - 1; j++) {
            m_Tiles[j] = m_Tiles[j + 1];
         }
         m_iNumTiles--;
         return true;
      }
   }

   return false; // cannot find this tile
}

bool CHand::RemoveTile(int index, int filter)
{
   if (index < 0 || index >= m_iNumTiles) {
      return false;
   }

   if (m_Tiles[index].flags & filter) {
      return false;
   }

   for (int j = index; j < m_iNumTiles - 1; j++) {
      m_Tiles[j] = m_Tiles[j + 1];
   }
   m_iNumTiles--;
   return true;
}

bool CHand::HasTile(const CTile &t, int filter)
{
   for (int i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & filter)
         continue; // skip unwanted tiles
      if (m_Tiles[i].tile == t)
         return true;
   }
   return false; // cannot find this tile
}

bool CHand::Pung(const CTile &t, bool open, int flags)
{
   handtile_t *p[3] = {NULL, NULL, NULL};
   int found = 0, i;

   for (i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue; // skip locked tiles

      if (m_Tiles[i].tile == t) {
         p[found++] = &m_Tiles[i];
         if (found >= (open ? 2 : 3))
            break;
      }
   }

   if (found < (open ? 2 : 3))
      return false; // cannot pung with this tile

   if (open) {
      // Add this tile into the hand
      m_Tiles[m_iNumTiles].tile = t;
      m_Tiles[m_iNumTiles].flags = (HT_OPENPUNG | HT_FROMOPPONENT | flags);
      m_Tiles[m_iNumTiles].tileset = m_iNumTileSets;
      m_iNumTiles++;
   }

   // mark the tiles as open pungs
   p[0]->flags |= ((open ? HT_OPENPUNG : HT_CLOSEDPUNG) | flags);
   p[1]->flags |= ((open ? HT_OPENPUNG : HT_CLOSEDPUNG) | flags);
   p[0]->tileset = p[1]->tileset = m_iNumTileSets;

   if (!open) {
      p[2]->flags |= (HT_CLOSEDPUNG | flags);
      p[2]->tileset = m_iNumTileSets;
   }

   // Add a tileset
   m_TileSets[m_iNumTileSets].first = t;
   m_TileSets[m_iNumTileSets].type = ((open ? HT_OPENPUNG : HT_CLOSEDPUNG) | flags);
   m_iNumTileSets++;

   return true;
}

bool CHand::Chow(const CTile &t, int location, bool open, int flags)
{
   handtile_t *p[3] = {NULL, NULL, NULL};
   CTile to_find[2];

   if (t.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON))
      return false; // cannot chow wind or dragon tiles

   int i, j, found = 0;

   switch (location) {
   case CHOW_LOWER:
      if (t.GetValue() > 7)
         return false;
      to_find[0] = t() + 1;
      to_find[1] = t() + 2;
      break;

   case CHOW_MIDDLE:
      if (t.GetValue() == 1 || t.GetValue() == 9)
         return false;
      to_find[0] = t() - 1;
      to_find[1] = t() + 1;
      break;

   case CHOW_UPPER:
      if (t.GetValue() < 3)
         return false;
      to_find[0] = t() - 1;
      to_find[1] = t() - 2;
      break;

   default:
      TerminateOnError("CHand::Chow(): Invalid location");
      return false;
   }

   for (i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue; // skip open or closed kong tiles

      for (j = 0; j < 2; j++) {
         if (m_Tiles[i].tile == to_find[j]) {
            if (found < 2)
               p[found++] = &m_Tiles[i];
            to_find[j] = 0; // already found this one
         }
      }

      if (!open && m_Tiles[i].tile == t) {
         p[2] = &m_Tiles[i];
      }


   }

   if (found < 2 || (!open && p[2] == NULL))
      return false; // cannot chow with this tile

   // Add this tile into the hand
   if (open) {
      m_Tiles[m_iNumTiles].tile = t;
      m_Tiles[m_iNumTiles].tileset = m_iNumTileSets;
      m_Tiles[m_iNumTiles].flags = (HT_OPENCHOW | HT_FROMOPPONENT | flags);
      m_iNumTiles++;
   }

   // mark the tiles as open pungs
   p[0]->flags |= ((open ? HT_OPENCHOW : HT_CLOSEDCHOW) | flags);
   p[1]->flags |= ((open ? HT_OPENCHOW : HT_CLOSEDCHOW) | flags);
   p[0]->tileset = p[1]->tileset = m_iNumTileSets;
   if (!open) {
      p[2]->flags |= (HT_CLOSEDCHOW | flags);
      p[2]->tileset = m_iNumTileSets;
   }

   // Add a tileset
   switch (location) {
   case CHOW_LOWER:
      m_TileSets[m_iNumTileSets].first = t;
      break;

   case CHOW_MIDDLE:
      m_TileSets[m_iNumTileSets].first = t() - 1;
      break;

   case CHOW_UPPER:
      m_TileSets[m_iNumTileSets].first = t() - 2;
      break;
   }

   m_TileSets[m_iNumTileSets].type = ((open ? HT_OPENCHOW : HT_CLOSEDCHOW) | flags);
   m_iNumTileSets++;

   return true;
}

bool CHand::Kong(const CTile &t, bool open, int flags)
{
   handtile_t *p[4] = {NULL, NULL, NULL, NULL};
   int found = 0, i;

   for (i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue; // skip open or closed kong tiles

      if (m_Tiles[i].tile == t) {
         p[found++] = &m_Tiles[i];
         if (found >= (open ? 3 : 4))
            break;
      }
   }

   if (found < (open ? 3 : 4)) {
      if (!open && found > 0) {
         // if not open kong, also see if we have a pung
         for (i = 0; i < m_iNumTileSets; i++) {
            if (m_TileSets[i].first == t &&
               (m_TileSets[i].type & HT_OPENPUNG))
            {
               m_TileSets[i].type = (HT_OPENKONG | flags);
               p[0]->flags |= (HT_OPENKONG | flags);

               for (i = 0; i < m_iNumTiles; i++) {
                  if (!(m_Tiles[i].flags & HT_OPENPUNG))
                     continue;
                  if (m_Tiles[i].tile == t) {
                     m_Tiles[i].flags |= (HT_OPENKONG | flags);
                     m_Tiles[i].flags &= ~HT_OPENPUNG;
                  }
               }
               return true;
            }
         }
      }
      return false; // cannot kong with this tile
   }

   if (open) {
      // Add this tile into the hand
      m_Tiles[m_iNumTiles].tile = t;
      m_Tiles[m_iNumTiles].flags = ((open ? HT_OPENKONG : HT_CLOSEDKONG) | HT_FROMOPPONENT | flags);
      m_Tiles[m_iNumTiles].tileset = m_iNumTileSets;
      m_iNumTiles++;
   }

   // mark the tiles as kongs
   p[0]->flags |= ((open ? HT_OPENKONG : HT_CLOSEDKONG) | flags);
   p[1]->flags |= ((open ? HT_OPENKONG : HT_CLOSEDKONG) | flags);
   p[2]->flags |= ((open ? HT_OPENKONG : HT_CLOSEDKONG) | flags);
   p[0]->tileset = p[1]->tileset = p[2]->tileset = m_iNumTileSets;
   if (!open) {
      p[3]->flags |= (HT_CLOSEDKONG | flags);
      p[3]->tileset = m_iNumTileSets;
   }

   // Add a tileset
   m_TileSets[m_iNumTileSets].first = t;
   m_TileSets[m_iNumTileSets].type = ((open ? HT_OPENKONG : HT_CLOSEDKONG) | flags);
   m_iNumTileSets++;

   return true;
}

bool CHand::CanPung(const CTile &t)
{
   int count = 0;
   for (int i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue;
      if (m_Tiles[i].tile == t) {
         count++;
      }
   }
   return (count >= 2);
}

bool CHand::CanChow(const CTile &t, int location)
{
   CTile to_find[2];

   if (t.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON))
      return false; // cannot chow wind or dragon tiles

   switch (location) {
   case CHOW_LOWER:
      if (t.GetValue() > 7)
         return false;
      to_find[0] = t() + 1;
      to_find[1] = t() + 2;
      break;

   case CHOW_MIDDLE:
      if (t.GetValue() == 1 || t.GetValue() == 9)
         return false;
      to_find[0] = t() - 1;
      to_find[1] = t() + 1;
      break;

   case CHOW_UPPER:
      if (t.GetValue() < 3)
         return false;
      to_find[0] = t() - 1;
      to_find[1] = t() - 2;
      break;

   default:
      TerminateOnError("CHand::Chow(): Invalid location");
      return false;
   }

   return (HasTile(to_find[0]) && HasTile(to_find[1]));
}

bool CHand::CanKong(const CTile &t, bool open)
{
   int count = 0, i;

   for (i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue;
      if (m_Tiles[i].tile == t) {
         count++;
      }
   }

   if (count >= (open ? 3 : 4))
      return true;

   if (!open) {
      for (i = 0; i < m_iNumTileSets; i++) {
         if ((m_TileSets[i].type & HT_OPENPUNG) &&
            m_TileSets[i].first == t)
            return true;
      }
   }

   return false;
}

// Check if we can go mah-jong with t.
bool CHand::CanMahjong(const CTile &t)
{
   CHand tmp(*this);
   tmp.AddTile(t);
   return tmp.GoMahjong_real();
}

// Return true if the hand in its current state of
// organization is a complete mah-jong hand.
// (That is, the hand has been organized into four sets
// and a pair, etc.)
bool CHand::GoMahjong()
{
   int i;

   for (i = m_iNumTiles - 1; i >= 0; i--) {
      if ((m_Tiles[i].flags & HT_JUSTGOT) == 0) {
         continue;
      }

      CHand tmp;

      tmp = *this;
      if (tmp.Pung(m_Tiles[i].tile, false, HT_TUMO)) {
         if (tmp.GoMahjong_real()) {
            *this = tmp;
            return true;
         }
      }

      tmp = *this;
      if (tmp.Chow(m_Tiles[i].tile, CHOW_LOWER, false, HT_TUMO)) {
         if (tmp.GoMahjong_real()) {
            *this = tmp;
            return true;
         }
      }

      tmp = *this;
      if (tmp.Chow(m_Tiles[i].tile, CHOW_UPPER, false, HT_TUMO)) {
         if (tmp.GoMahjong_real()) {
            *this = tmp;
            return true;
         }
      }

      tmp = *this;
      if (tmp.Chow(m_Tiles[i].tile, CHOW_MIDDLE, false, HT_TUMO)) {
         if (tmp.GoMahjong_real()) {
            *this = tmp;
            return true;
         }
      }

      tmp = *this;
      for (int j = 0; j < m_iNumTiles; j++) {
         if (j == i) {
            continue;
         }
         if (m_Tiles[j].tile == m_Tiles[i].tile) {
            tmp.m_Tiles[i].flags |= HT_PAIR;
            tmp.m_Tiles[j].flags |= HT_PAIR;
            tmp.m_Tiles[i].tileset = tmp.m_Tiles[j].tileset = tmp.m_iNumTileSets;

            // Add this pair to the tileset
            tmp.m_TileSets[tmp.m_iNumTileSets].first = tmp.m_Tiles[i].tile;
            tmp.m_TileSets[tmp.m_iNumTileSets].type = (HT_PAIR | HT_TUMO);
            tmp.m_iNumTileSets++;

            if (tmp.GoMahjong_real()) {
               *this = tmp;
               return true;
            }

            break;
         }
      }

      break;
   }

   if (i >= m_iNumTiles) {
      return GoMahjong_real();
   }

   // If all above fail, check if we have Thirteen Unique
   // Wonders or Seven Pairs...
   if (ThirteenWonders())
      return true;

   if (SevenPairs())
      return true;

   return false; // does NOT have a Mahjong
}

bool CHand::GoMahjong_real()
{
   int i, concealed = 0;
   handtile_t *t[2] = {NULL, NULL};

   for (i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue; // skip locked tiles

      CHand tmp;

      tmp = *this;
      if (tmp.Pung(m_Tiles[i].tile, false)) {
         if (tmp.GoMahjong_real()) {
            *this = tmp;
            return true;
         }
      }

      tmp = *this;
      if (tmp.Chow(m_Tiles[i].tile, CHOW_LOWER, false)) {
         if (tmp.GoMahjong_real()) {
            *this = tmp;
            return true;
         }
      }

      if (concealed < 2)
         t[concealed] = &m_Tiles[i];

      concealed++;
   }

   if (concealed == 2 && t[0]->tile == t[1]->tile) {
      // only one pair is remaining; this is a mah-jong hand
      t[0]->flags |= HT_PAIR;
      t[1]->flags |= HT_PAIR;
      t[0]->tileset = t[1]->tileset = m_iNumTileSets;

      // Add this pair to the tileset
      m_TileSets[m_iNumTileSets].first = t[0]->tile;
      m_TileSets[m_iNumTileSets].type = HT_PAIR;
      m_iNumTileSets++;

      return true;
   } else if (concealed == 0) {
      return true;
   }

   // If all above fail, check if we have Thirteen Unique
   // Wonders or Seven Pairs...
   if (ThirteenWonders())
      return true;

   if (SevenPairs())
      return true;

   return false; // does NOT have a Mahjong
}

bool CHand::GoMahjong(const CTile &t)
{
   CHand tmp;

   tmp = *this;
   if (tmp.Pung(t, true, HT_RON)) {
      if (tmp.GoMahjong_real()) {
         *this = tmp;
         return true;
      }
   }

   tmp = *this;
   if (tmp.Chow(t, CHOW_LOWER, true, HT_RON)) {
      if (tmp.GoMahjong_real()) {
         *this = tmp;
         return true;
      }
   }

   tmp = *this;
   if (tmp.Chow(t, CHOW_UPPER, true, HT_RON)) {
      if (tmp.GoMahjong_real()) {
         *this = tmp;
         return true;
      }
   }

   tmp = *this;
   if (tmp.Chow(t, CHOW_MIDDLE, true, HT_RON)) {
      if (tmp.GoMahjong_real()) {
         *this = tmp;
         return true;
      }
   }

   tmp = *this;
   for (int j = 0; j < m_iNumTiles; j++) {
      if (m_Tiles[j].flags & (HT_LOCKED | HT_TOCHOW))
         continue;
      if (m_Tiles[j].tile == t) {
         tmp.AddTile(t, HT_PAIR | HT_RON | HT_FROMOPPONENT, tmp.m_iNumTileSets);
         tmp.m_Tiles[j].flags |= (HT_PAIR | HT_RON);
         tmp.m_Tiles[j].tileset = tmp.m_iNumTileSets;

         // Add this pair to the tileset
         tmp.m_TileSets[tmp.m_iNumTileSets].first = tmp.m_Tiles[j].tile;
         tmp.m_TileSets[tmp.m_iNumTileSets].type = (HT_PAIR | HT_RON);
         tmp.m_iNumTileSets++;

         if (tmp.GoMahjong_real()) {
            *this = tmp;
            return true;
         }

         break;
      }
   }

   // If all above fail, check if we have Thirteen Unique
   // Wonders or Seven Pairs...
   tmp = *this;
   tmp.AddTile(t, HT_FROMOPPONENT);

   if (tmp.ThirteenWonders()) {
      *this = tmp;
      return true;
   }

   if (tmp.SevenPairs()) {
      *this = tmp;
      return true;
   }

   return false; // does NOT have a Mahjong
}

// Check if we have "Thirteen Unique Wonders" special hand.
bool CHand::ThirteenWonders()
{
   const CTile ttw[] = {"1C", "9C", "1D", "9D", "1B", "9B",
      "EW", "SW", "WW", "NW", "RD", "GD", "WD"};

   bool havetwo = false;

   int i, j;

   // first check if all the tiles are majors...
   for (j = 0; j < m_iNumTiles; j++) {
      if (!m_Tiles[j].tile.IsMajor())
         return false; // we have minors; fail
   }

   for (i = 0; i < 13; i++) {
      int count = 0;
      for (j = 0; j < m_iNumTiles; j++) {
         if (m_Tiles[j].tile == ttw[i]) {
            count++;
         }
      }

      if (count <= 0 || count > 2) {
         return false; // we have 3 or more same tiles; fail
      }

      if (count == 2) {
         if (havetwo) {
            return false;
         }
         havetwo = true;
      }
   }

   // MUST have one pair
   if (havetwo) {
      // Success. Set the "Thirteen Wonders" flag to all tiles
      for (i = 0; i < m_iNumTiles; i++) {
         m_Tiles[i].flags |= HT_THIRTEENWONDERS;
      }
      return true;
   }

   return false; // fail!
}

// Check if we have "Seven Pairs" special hand.
bool CHand::SevenPairs()
{
   if (m_iNumTiles <= 0)
      return true;

   if (m_iNumTileSets > 0)
      return false; // non-concealed hand is not possible

   CHand tmp = *this;
   CTile t = m_Tiles[0].tile;

   // three or more same tiles is not allowed!
   if (tmp.Pung(t, false))
      return false;

   tmp.RemoveTile(t);
   if (tmp.RemoveTile(t)) {
      if (tmp.SevenPairs()) {
         // FIXME: add the seven pairs to the tileset?
         for (int i = 0; i < m_iNumTiles; i++) {
            m_Tiles[i].flags |= HT_PAIR;
         }
         return true;
      }
   }

   return false;
}

// Return true if the hand in its current state of
// organization is a ready hand.
// (That is, we only need one more tile to go mah-jong.)
bool CHand::IsReady()
{
   int i, concealed = 0;
   CTile t[MAX_HANDTILE];

   for (i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue; // skip locked tiles

      CHand tmp;

      tmp = *this;
      if (tmp.Pung(m_Tiles[i].tile, false)) {
         if (tmp.IsReady()) {
            return true;
         }
      }

      tmp = *this;
      if (tmp.Chow(m_Tiles[i].tile, CHOW_LOWER, false)) {
         if (tmp.IsReady()) {
            return true;
         }
      }

      t[concealed++] = m_Tiles[i].tile;
   }

   if (concealed == 1) {
      // only 1 tile remaining; can form a pair with another tile
      return true;
   } else if (concealed == 4) {
      // check if we have 1 pair and another 2 tiles which can
      // form a chew or pung with another tile.

      // search for a pair
      int m, n, other[2], count = 0;

      for (m = 0; m < 4; m++) {
         for (n = m + 1; n < 4; n++) {
            if (m == n)
               continue;
            if (t[m] == t[n]) {
               break; // we've found the pair
            }
         }
         if (n < 4)
            break; // break out if we've already found the pair
      }

      if (m >= 4 || n >= 4)
         return false; // no pair found; fail

      for (i = 0; i < 4; i++) {
         if (i != m && i != n) {
            assert(count < 2);
            other[count++] = i;
         }
      }
      assert(count == 2);

      // If the other 2 tiles are not of the same suit...
      if (t[other[0]].GetSuit() != t[other[1]].GetSuit())
         return false; // fail!

      int d = abs(t[other[0]].GetValue() - t[other[1]].GetValue());
      return (d >= 0 && d < 3);
   }

   if (IsThirteenWondersReady())
      return true;

   if (IsSevenPairsReady())
      return true;

   return false; // does NOT have a Ready Hand
}

bool CHand::IsThirteenWondersReady()
{
   const CTile ttw[] = {"1C", "9C", "1D", "9D", "1B", "9B",
      "EW", "SW", "WW", "NW", "RD", "GD", "WD"};

   bool havetwo = false;

   int i, j;

   // first check if all the tiles are majors...
   for (j = 0; j < m_iNumTiles; j++) {
      if (!m_Tiles[j].tile.IsMajor())
         return false; // we have minors; fail
   }

   for (i = 0; i < 13; i++) {
      int count = 0;
      for (j = 0; j < m_iNumTiles; j++) {
         if (m_Tiles[j].tile == ttw[i]) {
            count++;
         }
      }

      if (count > 2) {
         return false; // we have 3 or more same tiles; fail
      } else if (count == 2) {
         if (havetwo) {
            return false;
         }
         havetwo = true;
      }
   }

   return true; // Success.
}

bool CHand::IsSevenPairsReady()
{
   if (m_iNumTiles <= 1)
      return true;

   if (m_iNumTileSets > 0)
      return false; // non-concealed hand is not possible

   CHand tmp = *this;
   CTile t = m_Tiles[0].tile;

   tmp.RemoveTile(t);
   if (tmp.RemoveTile(t)) {
      if (tmp.RemoveTile(t)) {
         // three or more same tiles is not allowed!
         return false;
      }
   } else {
      t = m_Tiles[1].tile;
      tmp = *this;
      tmp.RemoveTile(t);
      if (tmp.RemoveTile(t)) {
         if (tmp.RemoveTile(t)) {
            // three or more same tiles is not allowed!
            return false;
         }
      } else {
         return false; // 2 or more "orphaned" tiles are found
      }
   }

   return tmp.IsSevenPairsReady();
}

// Gets the waiting tiles, store it into the tiles array
// and returns the number of found waiting tiles.
int CHand::GetWaitingTile(CTile tiles[], int max)
{
   int current = 0;
   return GetWaitingTile(tiles, max, current);
}

// This is the real function to get the waiting tiles.
int CHand::GetWaitingTile(CTile tiles[], int max, int &current)
{
   int i, j, concealed = 0;
   CTile t[MAX_HANDTILE];

   for (i = 0; i < m_iNumTiles; i++) {
      if (m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue; // skip locked tiles

      CHand tmp;

      tmp = *this;
      if (tmp.Pung(m_Tiles[i].tile, false)) {
         tmp.GetWaitingTile(tiles, max, current);
      }

      tmp = *this;
      if (tmp.Chow(m_Tiles[i].tile, CHOW_LOWER, false)) {
         tmp.GetWaitingTile(tiles, max, current);
      }

      t[concealed++] = m_Tiles[i].tile;
   }

   if (concealed == 1) {
      // only 1 tile remaining; can form a pair with another tile
      for (j = 0; j < current; j++) {
         if (tiles[j] == t[0])
            break;
      }
      if (j >= current) {
         if (current >= max) {
            fprintf(stderr, "CHand::GetWaitingTile(): current >= max!\n");
            return current;
         }
         tiles[current++] = t[0];
      }
   } else if (concealed == 4) {
      // check if we have 1 pair and another 2 tiles which can
      // form a chew or pung with another tile.

      // search for a pair
      int m, n, other[2], count = 0;

      for (m = 0; m < 4; m++) {
         for (n = m + 1; n < 4; n++) {
            if (m == n)
               continue;
            if (t[m] == t[n]) {
               break; // we've found the pair
            }
         }
         if (n < 4)
            break; // break out if we've already found the pair
      }

      if (m >= 4 || n >= 4)
         return current; // no pair found; fail

      for (i = 0; i < 4; i++) {
         if (i != m && i != n) {
            assert(count < 2);
            other[count++] = i;
         }
      }
      assert(count == 2);

      // If the other 2 tiles are not of the same suit...
      if (t[other[0]].GetSuit() != t[other[1]].GetSuit())
         return current; // fail!

      if (t[other[0]].GetValue() > t[other[1]].GetValue()) {
         int temp = other[0];
         other[0] = other[1];
         other[1] = temp;
      }

      int d = t[other[1]].GetValue() - t[other[0]].GetValue();
      assert(d >= 0);

      switch (d) {
      case 0:
         for (j = 0; j < current; j++) {
            if (tiles[j] == t[other[0]])
               break;
         }
         if (j >= current) {
            if (current >= max) {
               fprintf(stderr, "CHand::GetWaitingTile(): current >= max!\n");
               return current;
            }
            tiles[current++] = t[other[0]];
         }
         break;

      case 1:
         if (t[other[0]].GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)) {
            break; // chow is impossible for Winds or Dragons
         }

         if (t[other[0]].GetValue() > 1) {
            for (j = 0; j < current; j++) {
               if (tiles[j] == t[other[0]]() - 1)
                  break;
            }
            if (j >= current) {
               if (current >= max) {
                  fprintf(stderr, "CHand::GetWaitingTile(): current >= max!\n");
                  return current;
               }
               tiles[current++] = t[other[0]]() - 1;
            }
         }

         if (t[other[1]].GetValue() < 9) {
            for (j = 0; j < current; j++) {
               if (tiles[j] == t[other[1]]() + 1)
                  break;
            }
            if (j >= current) {
               if (current >= max) {
                  fprintf(stderr, "CHand::GetWaitingTile(): current >= max!\n");
                  return current;
               }
               tiles[current++] = t[other[1]]() + 1;
            }
         }
         break;

      case 2:
         if (t[other[0]].GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)) {
            break; // chow is impossible for Winds or Dragons
         }

         for (j = 0; j < current; j++) {
            if (tiles[j] == t[other[0]]() + 1)
               break;
         }
         if (j >= current) {
            if (current >= max) {
               fprintf(stderr, "CHand::GetWaitingTile(): current >= max!\n");
               return current;
            }
            tiles[current++] = t[other[0]]() + 1;
         }
         break;
      }
   }

   if (current <= 0) {
      GetThirteenWondersWaitingTile(tiles, max, current);
   }

   if (current <= 0) {
      GetSevenPairsWaitingTile(tiles, max, current);
   }

   return current;
}

int CHand::GetThirteenWondersWaitingTile(CTile tiles[], int max, int &current)
{
   const CTile ttw[] = {"1C", "9C", "1D", "9D", "1B", "9B",
      "EW", "SW", "WW", "NW", "RD", "GD", "WD"};

   int i, j, zero = -1;
   bool havetwo = false;

   // first check if all the tiles are majors...
   for (i = 0; i < m_iNumTiles; i++) {
      if (!m_Tiles[i].tile.IsMajor())
         return current; // we have minors; fail
   }

   for (i = 0; i < 13; i++) {
      int count = 0;
      for (j = 0; j < m_iNumTiles; j++) {
         if (m_Tiles[j].tile == ttw[i]) {
            count++;
         }
      }
      if (count > 2) {
         return current; // we have 3 or more same tiles; fail
      } else if (count == 2) {
         if (havetwo) {
            return current; // fail
         }
         havetwo = true;
      } else if (count == 0) {
         zero = i;
      }
   }

   if (!havetwo) {
      // we're waiting for ALL majors!
      for (i = 0; i < 13; i++) {
         if (current >= max) {
            fprintf(stderr, "CHand::GetThirteenWondersWaitingTile(): current >= max\n");
            return current;
         }
         tiles[current++] = ttw[i];
      }
   } else if (zero != -1) {
      if (current >= max) {
         fprintf(stderr, "CHand::GetThirteenWondersWaitingTile(): current >= max\n");
         return current;
      }
      tiles[current++] = ttw[zero];
   }

   return current; // success!
}

int CHand::GetSevenPairsWaitingTile(CTile tiles[], int max, int &current)
{
   assert(m_iNumTiles >= 1);

   if (m_iNumTiles == 1) {
      // We've got the wanted tile
      if (current >= max) {
         fprintf(stderr, "CHand::GetSevenPairsWaitingTile(): current >= max\n");
         return current;
      }
      tiles[current++] = m_Tiles[0].tile;
      return current;
   }

   if (m_iNumTileSets > 0)
      return current; // non-concealed hand is not possible

   CHand tmp = *this;
   CTile t = m_Tiles[0].tile;

   tmp.RemoveTile(t);
   if (tmp.RemoveTile(t)) {
      if (tmp.RemoveTile(t)) {
         return current; // three or more same tiles is not allowed!
      }
   } else {
      t = m_Tiles[1].tile;
      tmp = *this;
      tmp.RemoveTile(t);
      if (tmp.RemoveTile(t)) {
         if (tmp.RemoveTile(t)) {
            return current; // three or more same tiles is not allowed!
         }
      } else {
         return current; // 2 or more "orphaned" tiles are found
      }
   }

   return tmp.GetSevenPairsWaitingTile(tiles, max, current);
}

// Return true if the hand in its current state of
// organization is a concealed hand.
bool CHand::IsConcealed()
{
   for (int i = 0; i < m_iNumTileSets; i++) {
      if (m_TileSets[i].type & (HT_OPEN & ~HT_CLOSEDKONG)) {
         if (m_TileSets[i].type & HT_RON)
            continue;
         return false; // it is NOT a concealed hand
      }
   }
   return true; // it is a concealed hand
}

void CHand::Restore()
{
   int i, n = 0;
   const int flag = (HT_CLOSEDPUNG | HT_CLOSEDCHOW |
      HT_PAIR | HT_THIRTEENWONDERS);

   for (i = 0; i < m_iNumTiles; i++) {
      m_Tiles[i].flags &= ~flag;
   }

   for (i = 0; i < m_iNumTileSets; i++) {
      if (m_TileSets[i].type & flag)
         continue;
      m_TileSets[n++] = m_TileSets[i];
   }
   m_iNumTileSets = n;
}
