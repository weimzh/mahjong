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

#ifndef GAME_H
#define GAME_H

enum
{
   TURN_EAST    = 0,
   TURN_SOUTH,
};

enum
{
   USRSTATE_DRAWTILE = 0,
   USRSTATE_DISCARDTILE,
};

enum
{
   COMPUTER = 0,
   PLAYER,
};

class CGame
{
public:
   CGame();
   virtual ~CGame();

   CBasePlayer    *m_pPlayer;   // human player
   CBasePlayer    *m_pBot;      // computer player

   bool            m_fReachBonus;

   inline int      GetRound()          { return m_iRound; }
   inline void     SetRound(int a)     { m_iRound = a; }

   inline int      GetCurGirl()        { return m_iCurGirl; }
   inline void     SetCurGirl(int a)   { m_iCurGirl = a; }

   inline int      GetNumDeal()        { return m_iNumDeal; }
   inline int      GetPendingPoint()   { return m_iPendingPoint; }

   void            RunGame();

   void            DonDen();

   void            NewRound();
   void            GameInit();
   void            SelectCurGirl();
   void            PlayRound();

   void            StageClear();
   void            HighScore();

   int             IsDora(const CTile &t, bool reach = false);
   void            KongDeclared();
   void            DrawScore(int x = 10, int y = 200);
   void            DrawDonDen();
   inline bool     CanDeclareKong()    { return m_iKongs < 4; }

private:
   int      m_iRound;
   int      m_iNumDeal;
   int      m_iPendingPoint;
   bool     m_fTurned;
   bool     m_fBonusGame;
   bool     m_fSanGenRush;
   int      m_iDonDen;
   int      m_iTotalDonDen;

   int      m_iTotalScore;

   int      m_iCurGirl;  // current girl player's playing against (0-3)
   int      m_iLevelClearFlag;

   CTile    m_Dora[6];   // 0-4 = Doras, 5 = Concealed Dora

   int      m_iKongs;

   int      m_iFirstChancePercent, m_iLastChancePercent;
   int      m_iReachBonusPercent;
   int      m_iBonusGamePercent, m_iSanGenRushPercent;

   void     DealTiles();
};

extern CGame *gpGame;

#endif

