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

#ifndef TILE_H
#define TILE_H

enum TileSuit
{
   TILESUIT_CHAR      =    (1 << 4),
   TILESUIT_CIRCLE    =    (1 << 5),
   TILESUIT_BAMBOO    =    (1 << 6),
   TILESUIT_WIND      =    (1 << 7),
   TILESUIT_DRAGON    =    (1 << 8),
};

enum Winds { EASTWIND = 1, SOUTHWIND, WESTWIND, NORTHWIND };
enum Dragons { WHITEDRAGON = 1, GREENDRAGON, REDDRAGON };

class CTile
{
public:
   CTile() : m_iKind(-1) {}
   CTile(int kind) : m_iKind(kind) {}
   CTile(int suit, int value) : m_iKind(suit | value) {}
   CTile(const char *code) { SetKind(code); }

   inline int  GetKind() const { return m_iKind; }
   const char *GetCode() const;

   int         GetSuit() const;
   int         GetValue() const;

   inline int         operator()(void) const      { return m_iKind; }
   inline int         operator=(int num)          { m_iKind = num; return num; }
   inline const char *operator=(const char *code) { SetKind(code); return code; }

   inline bool   operator==(const CTile &a) const { return (m_iKind == a.m_iKind); }
   inline bool   operator>(const CTile &a)  const { return (m_iKind > a.m_iKind); }
   inline bool   operator>=(const CTile &a) const { return (m_iKind >= a.m_iKind); }
   inline bool   operator<(const CTile &a)  const { return (m_iKind < a.m_iKind); }
   inline bool   operator<=(const CTile &a) const { return (m_iKind <= a.m_iKind); }

   inline bool   IsMajor() const
   {
      return (GetKind() & (TILESUIT_WIND | TILESUIT_DRAGON)) ||
         GetValue() == 1 || GetValue() == 9;
   }

   inline bool   IsNumMajor() const
   {
      return !(GetKind() & (TILESUIT_WIND | TILESUIT_DRAGON)) &&
         (GetValue() == 1 || GetValue() == 9);
   }

   inline bool   IsMinor() const { return !IsMajor(); }

   CTile         Dora() const;

   static void   NewRound();
   static CTile  RandomTile();
   static int    RemainingTile(const CTile &t);

   static bool   FetchTile(const CTile &t);
   static void   ReturnTile(const CTile &t);

   inline int    index1() const { return log2(GetSuit() >> 4); }
   inline int    index2() const { return GetValue() - 1; }

private:
   int         m_iKind;

   void        SetKind(const char *code);
};

#endif

