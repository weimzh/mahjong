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

#ifndef PLAYER_H
#define PLAYER_H

typedef struct discarded_s
{
   CTile        tile;
   int          flags;
} discarded_t;

enum discarded_flags
{
   DT_NULL           = 0,

   DT_REACH          = (1 << 0), // player discarded this tile when reach'ed
   DT_OPPONENTREACH  = (1 << 1), // opponent reached after we discarded this tile
};

enum playerstate_flags
{
   PS_KONGED         = (1 << 0), // Right after disclared a Kong
   PS_ASKED          = (1 << 1), // Right after asked for a tile
   PS_CALLREACH      = (1 << 2), // Right after called Reach
   PS_NOTDISCARDED   = (1 << 3), // Not discarded any tiles
};

enum playeraction
{
   PA_NONE           = 0,
   PA_DRAW,
   PA_DISCARD,
   PA_MAHJONG,
   PA_PUNG,
   PA_CHOW,
   PA_KONG,
   PA_REACH,

   PA_COUNT
};

enum actionstate
{
   AS_DRAW           = 0,
   AS_DISCARD,
};

#define MAX_DISCARDED 20 /* max of 20 discarded tiles */

enum scorelevel
{
   SL_ONEHAN = 0,
   SL_TWOHAN,
   SL_THREEHAN,
   SL_FOURHAN,
   SL_MANGAN,
   SL_HANEMAN,
   SL_BAIMAN,
   SL_SANBAIMAN,
   SL_YAKUMAN,
};

typedef struct result_s
{
   bool concealed;
   bool selfdrawn;

   int score;
   int level;

   int fu;

   int tenhou;
   int renhou;
   int chihou;
   int tuisou;
   int chinraotou;
   int ryuisou;
   int daisangen;
   int daisusi;
   int syosusi;
   int suankou;
   int surenkou;
   int kokusi;
   int cyurenpotou;
   int sukantu;

   int reach;
   int pinfu;
   int ippatu;
   int double_reach;
   int tanyao;
   int tumo;
   int ryanpeikou;
   int iipeikou;
   int sansiki;
   int sansiki_doukoku;
   int toitoi;
   int sanankou;
   int sanrenkou;
   int junchan;
   int chanta;
   int ittu;
   int chinitu;
   int honitu;
   int chitoitu;
   int honraotou;
   int syosangen;
   int haitei;
   int houtei;
   int rinshan;
   int fanpai;
   int dora;
   int sankantu;

   int nofan;
} result_t;

class CBasePlayer
{
public:
   CBasePlayer();
   virtual ~CBasePlayer();

   virtual bool       IsBot() = 0;

   CHand              m_Hand;

   int                m_iState;

   discarded_t        m_Discarded[MAX_DISCARDED];
   int                m_iNumDiscarded;

   result_t           m_Result;

   bool               m_fReach;

   bool               HasDiscarded(const CTile &t);
   inline CTile       LastDiscardedTile()
   {
      if (m_iNumDiscarded <= 0)
         return 0;
      return m_Discarded[m_iNumDiscarded - 1].tile;
   }

   virtual void       NewRound();
   void               GetHand();
   void               GetBonusHand();

   bool               HasYakuman();
   int                HasFan();

   void               CalcResult();

   virtual void       DrawHand(bool shown = false) = 0;
   virtual void       DrawDiscarded() = 0;

   CBasePlayer       *m_pOpponent;
   int                m_iTurn;
   int                m_iScore;

   virtual void           FirstChance() { }
   virtual bool           LastChance() { return false; }
   virtual playeraction   Action(enum actionstate state) = 0;

   bool               ActionIsValid(enum actionstate state, playeraction act);
   bool               IsRoundDraw();

   void               DrawResult();

private:
   void               CalcResult_tenhou();
   void               CalcResult_renhou();
   void               CalcResult_tuisou();
   void               CalcResult_chinraotou();
   void               CalcResult_ryuisou();
   void               CalcResult_daisangen();
   void               CalcResult_daisusi();
   void               CalcResult_suankou();
   void               CalcResult_surenkou();
   void               CalcResult_kokusi();
   void               CalcResult_cyurenpotou();
   void               CalcResult_sukantu();

   void               CalcResult_reach();
   void               CalcResult_ippatu();
   void               CalcResult_double_reach();
   void               CalcResult_tanyao();
   void               CalcResult_ryanpeikou();
   void               CalcResult_sansiki();
   void               CalcResult_sansiki_doukoku();
   void               CalcResult_toitoi();
   void               CalcResult_junchan();
   void               CalcResult_ittu();
   void               CalcResult_chinitu();
   void               CalcResult_honitu();
   void               CalcResult_chitoitu();
   void               CalcResult_honraotou();
   void               CalcResult_haitei();
   void               CalcResult_rinshan();
   void               CalcResult_fanpai();
   void               CalcResult_dora();

   void               CalcFu();
   void               CalcScore();

protected:
   int                IndexToLoc(int index);

   bool               m_fTuiDaoHu;
};

//---------------------------------------------------------

class CPlayer : public CBasePlayer
{
public:
   CPlayer();
   virtual ~CPlayer();

   bool             IsBot() { return false; }

   void             DrawHand(bool shown = false);
   void             DrawDiscarded();

   int              KeyToIndex(SDLKey key);

   void             FirstChance();
   bool             LastChance();
   void             SanGenInit();
   playeraction     Action(enum actionstate state);

   void             NewRound();

private:
   void             DrawAction(playeraction act);

   playeraction     ActionDraw();
   playeraction     ActionDiscard();
   playeraction     ActionChow();
   playeraction     ActionKong();

   bool             SanGenRush();

   bool             m_fAutoDraw;

   CTile            m_SanGenRush[9];
   unsigned char    m_iNumSanGenRush, m_iCountSanGenRush;
};

#endif

