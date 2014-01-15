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

static int RemainTiles[5][9];

int CTile::GetSuit() const
{
   int suit = GetKind();
   for (int i = 0; i < 4; i++) {
      suit &= ~(1 << i);
   }
   return suit;
}

int CTile::GetValue() const
{
   return m_iKind & ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
}

// Given a tile, return its two letter string. The string must be
// in static storage and immutable.
const char *CTile::GetCode() const
{
   // two letter codes for tiles
   static char *chartiles[] = {"1C", "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C"};
   static char *dottiles[] = {"1D", "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D"};
   static char *bamtiles[] = {"1B", "2B", "3B", "4B", "5B", "6B", "7B", "8B", "9B"};

   if (GetSuit() == TILESUIT_CHAR) {
      return chartiles[GetValue() - 1];
   } else if (GetSuit() == TILESUIT_BAMBOO) {
      return bamtiles[GetValue() - 1];
   } else if (GetSuit() == TILESUIT_CIRCLE) {
      return dottiles[GetValue() - 1];
   } else if (GetSuit() == TILESUIT_WIND) {
      switch (GetValue()) {
         case EASTWIND:   return "EW";
         case SOUTHWIND:  return "SW";
         case WESTWIND:   return "WW";
         case NORTHWIND:  return "NW";
      }
   } else if (GetSuit() == TILESUIT_DRAGON) {
      switch (GetValue()) {
         case REDDRAGON:    return "RD";
         case GREENDRAGON:  return "GD";
         case WHITEDRAGON:  return "WD";
      }
   }

   TerminateOnError("CTile::GetCode() error: %i", m_iKind);
   return "XX";
}

void CTile::SetKind(const char *code)
{
   if (code[0] >= '0' && code[0] <= '9') {
      m_iKind = code[0] - '0';
      switch (code[1]) {
         case 'B':
         case 'b':
            m_iKind |= TILESUIT_BAMBOO;
            break;

         case 'C':
         case 'c':
            m_iKind |= TILESUIT_CHAR;
            break;

         case 'D':
         case 'd':
            m_iKind |= TILESUIT_CIRCLE;
            break;

         default:
            TerminateOnError("CTile::SetKind() error #1: %s", code);
            break;
      }
   } else {
      if (code[1] == 'w' || code[1] == 'W') {
         switch (code[0]) {
            case 'E':
            case 'e':
               m_iKind = EASTWIND;
               break;

            case 'S':
            case 's':
               m_iKind = SOUTHWIND;
               break;

            case 'W':
            case 'w':
               m_iKind = WESTWIND;
               break;

            case 'N':
            case 'n':
               m_iKind = NORTHWIND;
               break;

            default:
               TerminateOnError("CTile::SetKind() error #2: %s", code);
               break;
         }
         m_iKind |= TILESUIT_WIND;
      } else if (code[1] == 'd' || code[1] == 'D') {
         switch (code[0]) {
            case 'R':
            case 'r':
               m_iKind = REDDRAGON;
               break;

            case 'G':
            case 'g':
               m_iKind = GREENDRAGON;
               break;

            case 'W':
            case 'w':
               m_iKind = WHITEDRAGON;
               break;

            default:
               TerminateOnError("CTile::SetKind() error #3: %s", code);
               break;
         }
         m_iKind |= TILESUIT_DRAGON;
      } else {
         TerminateOnError("CTile::SetKind() error #4: %s", code);
      }
   }
}

CTile CTile::Dora() const
{
   if (GetSuit() == TILESUIT_DRAGON) {
      switch (GetValue()) {
         case REDDRAGON:
            return CTile(GetSuit() | WHITEDRAGON);

         case WHITEDRAGON:
            return CTile(GetSuit() | GREENDRAGON);

         case GREENDRAGON:
            return CTile(GetSuit() | REDDRAGON);
      }
   } else {
      int v = GetValue() + 1;
      if (v > 9 || (GetSuit() == TILESUIT_WIND && v > 4))
         v = 1;
      return CTile(GetSuit() | v);
   }

   TerminateOnError("CTile::Dora(): Invalid tile!");
   return CTile("1C"); // to remove the compiler warning
}

void CTile::NewRound()
{
   int i, j;

   for (i = 0; i < 5; i++) {
      for (j = 0; j < 9; j++) {
         RemainTiles[i][j] = 4;
      }
   }

   // we only have 4 kinds of winds and 3 kinds of dragons
   for (i = 4; i < 9; i++) {
      RemainTiles[3][i] = 0;
   }
   for (i = 3; i < 9; i++) {
      RemainTiles[4][i] = 0;
   }
}

CTile CTile::RandomTile()
{
   CTile wall[172];
   int num = 0;

   // Push all possible tiles to the wall
   for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 9; j++) {
         for (int k = 0; k < RemainTiles[i][j]; k++) {
            int suit = ((1 << 4) << i);
            int value = j + 1;
            assert((suit + value) == (suit | value));
            wall[num++] = CTile(suit | value);
         }
      }
   }

   // pick a random one in the wall
   if (num <= 0) {
      // can this ever happen if nothing messed up?
      TerminateOnError("CTile::RandomTile(): No tile left!");
   }

   CTile &got = wall[RandomLong(0, num - 1)];
   RemainTiles[got.index1()][got.index2()] -= 1;

   return got;
}

int CTile::RemainingTile(const CTile &t)
{
   return RemainTiles[t.index1()][t.index2()];
}

bool CTile::FetchTile(const CTile &t)
{
   if (RemainTiles[t.index1()][t.index2()] <= 0)
      return false; // No this tile left
   RemainTiles[t.index1()][t.index2()] -= 1;
   return true;
}

void CTile::ReturnTile(const CTile &t)
{
   RemainTiles[t.index1()][t.index2()] += 1;
}

