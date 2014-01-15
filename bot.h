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

#ifndef BOT_H
#define BOT_H

#include "player.h"
#include "hand.h"

enum botgoal
{
   GOAL_NULL         = 0,

   GOAL_PAIRS,             // seven pairs
   GOAL_WONDERS,           // 13 unique wonders
   GOAL_ALLPUNG,           // all pung hand
   GOAL_ALLMINOR,          // all minor hand

   GOAL_ALLCHAR,           // all character tile
   GOAL_ALLBAMBOO,         // all bamboo tile
   GOAL_ALLCIRCLE,         // all circle tile

   GOAL_FANPAI,

   GOAL_COUNT
};

#define MAX_HANDRESULT  100

typedef struct botbrain_s
{
   playeraction      action;
   chow_location     chowlocation;
   int               act_index;

   int               goal;

   float             aggression; // [0, 1.0]
   float             goalvalue[GOAL_COUNT];

   float             tilevalue[MAX_HANDTILE];
   float             tiledanger[MAX_HANDTILE];

   CHand             handresult[MAX_HANDRESULT];
   int               handresult_ft[MAX_HANDRESULT];
   int               num_handresult;
   int               handresult_rest;
   int               ahand_kinds[5][9];
   float             danger[5][9];
} botbrain_t;

// computer player (bot)
class CBot : public CBasePlayer
{
public:
   CBot();
   virtual ~CBot();

   bool            IsBot() { return true; }

   void            NewRound();

   void            DrawHand(bool shown = false);
   void            DrawDiscarded();

   playeraction    Action(enum actionstate state);

private:
   playeraction    ActionDraw();
   playeraction    ActionDiscard();

   void            BotThink(enum actionstate state);
   void            UpdateStatus();

   void            UpdateDanger();
   float           TileDanger(const CTile &t);

   void            AnalyzeGoal();

   void            AnalyzeHand();
   void            AnalyzeHand_real(CHand &hand, int begin);
   void            AnalyzeHand_add(CHand &hand, int ft);
   void            AnalyzeHand_sort();

   void            SelectGoal();

   bool            WantToChow();
   bool            WantToPung();
   bool            WantToOpenKong();
   bool            WantToClosedKong();
   bool            WantToReach();

   void            AnalyzeDiscard();
   void            CalcTileValue(int rests[]);

   void            AnalyzeDora();
   void            AnalyzeColor();
   void            AnalyzeZone();
   void            AnalyzeNum();
   void            AnalyzeDanger();

   int             m_iLastDiscardIndex;
   botbrain_t      m_BotBrain;
};

#endif

