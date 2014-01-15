//
// Copyright (c) 2005, Wei Mingzhi <whistler@openoffice.org>
// Portions copyright (c) 1994, 1995 Koji Suzuki <suz@d2.bs1.fc.nec.co.jp>
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

// Many parts of this file is based on Netmaj by Koji Suzuki
// so credit goes to him.

#include "main.h"

#define DRAW_LOCATION(l) \
   ((l >= 13) ? 640 - (20 + TILE_WIDTH * (l + 1) + 8) : 640 - (20 + TILE_WIDTH * (l + 1)))

// Uncomment this to enable debug messages
//#define BOT_DEBUG

CBot::CBot()
{
}

CBot::~CBot()
{
}

void CBot::NewRound()
{
   m_BotBrain.goal = -1;

   m_BotBrain.aggression = (float)m_iScore / (m_iScore + m_pOpponent->m_iScore);
   m_BotBrain.aggression -= 0.5;
   m_BotBrain.aggression *= 2;

   m_BotBrain.num_handresult = 0;

   CBasePlayer::NewRound();

#ifdef BOT_DEBUG
   fprintf(stdout, "*** NEW ROUND STARTED ***\n");
#endif
}

void CBot::DrawHand(bool shown)
{
   int i, j, loc = 0;

   if (m_pOpponent->m_fReach) {
      shown = true; // alway show computer's hand if player reached
   }

#ifdef BOT_DEBUG
   shown = true;
#endif

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if ((m_Hand.m_TileSets[i].type & HT_OPEN) &&
         !(m_Hand.m_TileSets[i].type & HT_RON))
      {
         if (m_Hand.m_TileSets[i].type & HT_OPENKONG) {
            for (j = 0; j < 3; j++) {
               gpGeneral->EraseArea(DRAW_LOCATION(loc), 10,
                  TILE_WIDTH, 80);
               gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
                  DRAW_LOCATION(loc), 25, COMPUTER_SHOWN);
               loc++;
            }
            // Draw another tile as it's a kong
            gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
               DRAW_LOCATION(loc - 2), 8, COMPUTER_SHOWN);
         } else if (m_Hand.m_TileSets[i].type & HT_CLOSEDKONG) {
            for (j = 0; j < 3; j++) {
               gpGeneral->EraseArea(DRAW_LOCATION(loc), 10,
                  TILE_WIDTH, 80);
               gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
                  DRAW_LOCATION(loc), 25, WALL_CONCEALED);
               loc++;
            }
            // Draw another tile as it's a kong
            gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
               DRAW_LOCATION(loc - 2), 8, COMPUTER_SHOWN);
         } else if (m_Hand.m_TileSets[i].type & HT_OPENPUNG) {
            for (j = 0; j < 3; j++) {
               gpGeneral->EraseArea(DRAW_LOCATION(loc), 10,
                  TILE_WIDTH, 80);
               gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
                  DRAW_LOCATION(loc), 25, COMPUTER_SHOWN);
               loc++;
            }
         } else if (m_Hand.m_TileSets[i].type & HT_OPENCHOW) {
            CTile t = m_Hand.m_TileSets[i].first;
            for (j = 0; j < 3; j++) {
               gpGeneral->EraseArea(DRAW_LOCATION(loc), 10,
                  TILE_WIDTH, 80);
               gpGeneral->DrawTile(t, DRAW_LOCATION(loc),
                  25, COMPUTER_SHOWN);
               loc++;
               t = t() + 1;
            }
         } else {
            assert(false); // this should NOT happen
         }
      }
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if ((m_Hand.m_Tiles[i].flags & HT_OPEN) &&
         !(m_Hand.m_Tiles[i].flags & HT_RON))
      {
         continue;
      }
      gpGeneral->EraseArea(DRAW_LOCATION(loc), 10,
         TILE_WIDTH, 80);
      gpGeneral->DrawTile(m_Hand.m_Tiles[i].tile,
         DRAW_LOCATION(loc), shown ? 18 : 10,
         shown ? COMPUTER_SHOWN : COMPUTER_CONCEALED);
      loc++;
   }

   gpGeneral->EraseArea(0, 0, DRAW_LOCATION(loc - 1),
      18 + TILE_HEIGHT_CONCEALED);
   gpGeneral->UpdateScreen(0, 0, 640, 18 + TILE_HEIGHT_CONCEALED);
}

void CBot::DrawDiscarded()
{
   int i, x, y;

   for (i = 0; i < 10 && i < m_iNumDiscarded - 10; i++) {
      gpGeneral->DrawTile(m_Discarded[i + 10].tile, DRAW_LOCATION(i),
         94, WALL_SHOWN);
      if (m_Discarded[i + 10].flags & DT_REACH) {
         bool locked = false;
         if (SDL_MUSTLOCK(gpScreen)) {
            SDL_LockSurface(gpScreen);
            locked = true;
         }
         for (x = DRAW_LOCATION(i); x < DRAW_LOCATION(i) + TILE_WIDTH; x++) {
            for (y = 94; y < 94 + TILE_HEIGHT_SHOWN; y++) {
               unsigned char r, g, b;
               UTIL_GetPixel(gpScreen, x, y, &r, &g, &b);
               if (g >= 255 - 35) {
                  g = 255;
               } else {
                  g += 35;
               }
               UTIL_PutPixel(gpScreen, x, y, r, g, b);
            }
         }
         if (locked) {
            SDL_UnlockSurface(gpScreen);
         }
      }
   }

   gpGeneral->EraseArea(DRAW_LOCATION(9), 94, TILE_WIDTH * (10 - i),
      TILE_HEIGHT_SHOWN - 22);

   for (i = 0; i < 10 && i < m_iNumDiscarded; i++) {
      gpGeneral->DrawTile(m_Discarded[i].tile, DRAW_LOCATION(i),
         132, WALL_SHOWN);
      if (m_Discarded[i].flags & DT_REACH) {
         bool locked = false;
         if (SDL_MUSTLOCK(gpScreen)) {
            SDL_LockSurface(gpScreen);
            locked = true;
         }
         for (x = DRAW_LOCATION(i); x < DRAW_LOCATION(i) + TILE_WIDTH; x++) {
            for (y = 132; y < 132 + TILE_HEIGHT_SHOWN; y++) {
               unsigned char r, g, b;
               UTIL_GetPixel(gpScreen, x, y, &r, &g, &b);
               if (g >= 255 - 35) {
                  g = 255;
               } else {
                  g += 35;
               }
               UTIL_PutPixel(gpScreen, x, y, r, g, b);
            }
         }
         if (locked) {
            SDL_UnlockSurface(gpScreen);
         }
      }
   }

   gpGeneral->EraseArea(DRAW_LOCATION(9), 132, TILE_WIDTH * (10 - i),
      TILE_HEIGHT_SHOWN);

   if (m_fReach) {
      gpGeneral->DrawDotBar(DRAW_LOCATION(9) - 20, 99);
   } else {
      gpGeneral->EraseArea(DRAW_LOCATION(9) - 20, 99, 20, 85);
   }

   gpGeneral->UpdateScreen(DRAW_LOCATION(9) - 20, 94, 660 - DRAW_LOCATION(9),
      TILE_HEIGHT_SHOWN * 2 - 22);
}

playeraction CBot::Action(enum actionstate state)
{
   playeraction ret = PA_NONE;
   int first = SDL_GetTicks();

   if (ActionIsValid(state, PA_MAHJONG)) {
      // Do we still need to think more as we've already won?
      if (state == AS_DRAW) {
         m_Hand.AddTile(m_pOpponent->LastDiscardedTile());
      }
      UTIL_Delay(800); // delay a while
      return PA_MAHJONG; // we've already won the round
   } else if ((m_iState & PS_KONGED) && state == AS_DRAW) {
      m_BotBrain.action = PA_DRAW;
   } else {
      BotThink(state);
   }

   int time;

   switch (state) {
      case AS_DRAW:
         ret = ActionDraw();
         time = first + 300 - SDL_GetTicks();
         if (time > 0) {
            UTIL_Delay(time);
         }
         break;

      case AS_DISCARD:
         ret = ActionDiscard();
         time = first + 350 - SDL_GetTicks();
         if (time > 0) {
            UTIL_Delay(time);
         }
         if (m_iLastDiscardIndex != -1) {
            gpGeneral->EraseArea(DRAW_LOCATION(IndexToLoc(m_iLastDiscardIndex)),
               10, TILE_WIDTH, 80);
            gpGeneral->UpdateScreen(DRAW_LOCATION(IndexToLoc(m_iLastDiscardIndex)),
               10, TILE_WIDTH, 80);
            UTIL_Delay(100);
         }
         break;

      default:
         assert(false); // can this ever happen?
         break;
   }

   return ret;
}

playeraction CBot::ActionDraw()
{
   if (IsRoundDraw()) {
      return PA_NONE; // round has ended
   }

   int i;

   switch (m_BotBrain.action) {
      case PA_DRAW:
         for (i = 0; i < m_Hand.m_iNumTiles; i++) {
            m_Hand.m_Tiles[i].flags &= ~HT_JUSTGOT;
         }
         m_Hand.AddRandomTile(HT_JUSTGOT);
         m_iState &= (PS_CALLREACH | PS_KONGED);
         break;


      case PA_CHOW:
         if (!m_Hand.Chow(m_pOpponent->LastDiscardedTile(), m_BotBrain.chowlocation)) {
            TerminateOnError("CBot::ActionDraw(): cannot chow");
         }
         m_pOpponent->m_iNumDiscarded--;
         m_iState |= PS_ASKED;
         break;

      case PA_PUNG:
         if (!m_Hand.Pung(m_pOpponent->LastDiscardedTile())) {
            TerminateOnError("CBot::ActionDraw(): cannot pung");
         }
         m_pOpponent->m_iNumDiscarded--;
         m_iState |= PS_ASKED;
         break;

      case PA_KONG:
         if (!m_Hand.Kong(m_pOpponent->LastDiscardedTile())) {
            TerminateOnError("CBot::ActionDraw(): cannot kong");
         }
         m_pOpponent->m_iNumDiscarded--;
         m_iState |= (PS_ASKED | PS_KONGED);
         break;

      default:
         TerminateOnError("CBot::ActionDraw(): unknown action");
         break;
   }

   return m_BotBrain.action;
}

playeraction CBot::ActionDiscard()
{
   int index;

   m_iLastDiscardIndex = -1;

   switch (m_BotBrain.action) {
      case PA_REACH:
         gpGeneral->PlayBGMusic(-1);
         gpGeneral->PlaySound(SND_REACH);
         gpGeneral->DrawDotBar(DRAW_LOCATION(9) - 20, 99, true);
         gpGeneral->UpdateScreen(DRAW_LOCATION(9) - 20, 99, 20, 85);
         UTIL_Delay(800);
         m_iState = PS_CALLREACH;
         m_fReach = true;

      case PA_DISCARD:
         index = m_BotBrain.act_index;
         if (index < 0 || index >= m_Hand.m_iNumTiles) {
            TerminateOnError("CBot::ActionDiscard(): index out of range");
         }

         m_Discarded[m_iNumDiscarded].tile = m_Hand.m_Tiles[index].tile;
         m_Discarded[m_iNumDiscarded].flags = 0;
         m_iNumDiscarded++;

         if (m_BotBrain.action == PA_REACH) {
            m_Discarded[m_iNumDiscarded - 1].flags |= DT_REACH;
            if (m_pOpponent->m_iNumDiscarded > 0) {
               m_pOpponent->m_Discarded[m_pOpponent->m_iNumDiscarded - 1].flags |= DT_OPPONENTREACH;
            }
         } else {
            m_iState = 0;
         }

         m_Hand.RemoveTile(index);

         m_iLastDiscardIndex = index;

         for (index = 0; index < m_Hand.m_iNumTiles; index++) {
            m_Hand.m_Tiles[index].flags &= ~HT_JUSTGOT;
         }

         m_Hand.Sort();

         break;

      case PA_KONG:
         if (!m_Hand.Kong(m_Hand.m_Tiles[m_BotBrain.act_index].tile, false)) {
            TerminateOnError("CBot::ActionDiscard(): cannot kong");
         }
         m_iState |= PS_KONGED;
         break;

      default:
         TerminateOnError("CBot::ActionDiscard(): unknown bot action");
         break;
   }

   return m_BotBrain.action;
}

void CBot::BotThink(enum actionstate state)
{
#ifdef BOT_DEBUG
   fprintf(stdout, "=== CBot::BotThink() begin ===\n");
#endif

   int i;

   if (m_fReach) {
      // we've reached, no need to think too much
      if (state == AS_DRAW) {
         // we can do nothing other than drawing a tile now
         m_BotBrain.action = PA_DRAW;
      } else if (state == AS_DISCARD) {
         // check if we can declare a kong, and after declaring
         // we still have a ready hand...
         for (i = m_Hand.m_iNumTiles - 1; i >= 0; i--) {
            if ((m_Hand.m_Tiles[i].flags & HT_JUSTGOT) == 0) {
               continue;
            }
            m_BotBrain.act_index = i;
            CHand tmp = m_Hand;
            if (tmp.Kong(m_Hand.m_Tiles[i].tile, false)) {
               if (tmp.IsReady()) {
                  m_BotBrain.action = PA_KONG;
                  return;
               }
            }
            break;
         }
         m_BotBrain.action = PA_DISCARD;
      } else {
         TerminateOnError("CBot::BotThink(): unknown action state");
      }
      return;
   }

   UpdateStatus();

   switch (state) {
      case AS_DRAW:
         m_BotBrain.action = PA_DRAW;
         if (WantToChow()) {
#ifdef BOT_DEBUG
            fprintf(stdout, "Bot want to chow\n");
#endif
         }
         if (WantToPung()) {
#ifdef BOT_DEBUG
            fprintf(stdout, "Bot want to pung\n");
#endif
         }
         if (WantToOpenKong()) {
#ifdef BOT_DEBUG
            fprintf(stdout, "Bot want to open kong\n");
#endif
         }
         break;

      case AS_DISCARD:
         m_BotBrain.action = PA_DISCARD;
         if (m_BotBrain.action == PA_DISCARD ||
            m_BotBrain.action == PA_REACH) {
            AnalyzeDiscard();
         }
         if (WantToReach()) {
#ifdef BOT_DEBUG
            fprintf(stdout, "Bot want to reach\n");
#endif
         }
         break;
   }

#ifdef BOT_DEBUG
   fprintf(stdout, "=== CBot::BotThink() end ===\n");
#endif
}

void CBot::UpdateStatus()
{
#ifdef BOT_DEBUG
   fprintf(stdout, "=== CBot::UpdateStatus() begin ===\n");
#endif

   // TODO
   int i;

   // update tile danger value
   UpdateDanger();

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & HT_OPEN) {
         m_BotBrain.tiledanger[i] = 0;
         continue;
      }
      m_BotBrain.tiledanger[i] = TileDanger(m_Hand.m_Tiles[i].tile);
   }

   AnalyzeGoal();
   SelectGoal();

#ifdef BOT_DEBUG
   fprintf(stdout, "=== CBot::UpdateStatus() end ===\n");
#endif
}

void CBot::UpdateDanger()
{
   int i, j, rest_kind[5][9], d[5][9], d2[5][9], n = 0;

   for (i = 0; i < 5; i++) {
      for (j = 0; j < 9; j++) {
         if (i < 3) {
            int value[] = {250, 440, 700, 800, 870, 800, 700, 440, 250};
            m_BotBrain.danger[i][j] = value[j];
         } else {
            m_BotBrain.danger[i][j] = 50;
         }
         rest_kind[i][j] = CTile::RemainingTile(CTile((TILESUIT_CHAR << i) | (j + 1)));
         d[i][j] = d2[i][j] = 0;
      }
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & HT_LOCKED)
         continue;
      rest_kind[m_Hand.m_Tiles[i].tile.index1()][m_Hand.m_Tiles[i].tile.index2()]--;
   }

   if (m_pOpponent->m_fReach) {
      bool reach = false;

      for (i = 0; i < m_iNumDiscarded; i++) {
         if (m_Discarded[i].flags & DT_OPPONENTREACH) {
            reach = true;
         }
         CTile &t = m_Discarded[i].tile;
         if (reach) {
            m_BotBrain.danger[t.index1()][t.index2()] = 0;
         } else {
            d[t.index1()][t.index2()] = 1;
         }
         d2[t.index1()][t.index2()] = 1;
      }

      reach = false;

      for (i = 0; i < m_pOpponent->m_iNumDiscarded; i++) {
         if (m_pOpponent->m_Discarded[i].flags & DT_REACH) {
            reach = true;
         }
         CTile &t = m_pOpponent->m_Discarded[i].tile;
         m_BotBrain.danger[t.index1()][t.index2()] = 0;
         if (!reach) {
            d[t.index1()][t.index2()] = 1;
         }
         d2[t.index1()][t.index2()] = 1;
      }

      for (i = 0; i < 3; i++) {
         if (d[i][1 - 1]) {
            m_BotBrain.danger[i][2 - 1] *= 4.2f / 4.4f;
            m_BotBrain.danger[i][3 - 1] *= 6.8f / 7.0f;
            m_BotBrain.danger[i][4 - 1] *= 6.9f / 8.0f;
            m_BotBrain.danger[i][5 - 1] *= 9.0f / 8.7f;
            m_BotBrain.danger[i][6 - 1] *= 8.5f / 8.0f;
         }
         if (d[i][9 - 1]) {
            m_BotBrain.danger[i][8 - 1] *= 4.2f / 4.4f;
            m_BotBrain.danger[i][7 - 1] *= 6.8f / 7.0f;
            m_BotBrain.danger[i][6 - 1] *= 6.9f / 8.0f;
            m_BotBrain.danger[i][5 - 1] *= 9.0f / 8.7f;
            m_BotBrain.danger[i][4 - 1] *= 8.5f / 8.0f;
         }
         if (d[i][2 - 1]) {
            m_BotBrain.danger[i][1 - 1] *= 1.4f / 2.5f;
            m_BotBrain.danger[i][3 - 1] *= 5.9f / 7.0f;
            m_BotBrain.danger[i][4 - 1] *= 8.2f / 8.0f;
            m_BotBrain.danger[i][5 - 1] *= 7.5f / 8.7f;
            m_BotBrain.danger[i][6 - 1] *= 9.3f / 8.0f;
         }
         if (d[i][8 - 1]) {
            m_BotBrain.danger[i][9 - 1] *= 1.4f / 2.5f;
            m_BotBrain.danger[i][7 - 1] *= 5.9f / 7.0f;
            m_BotBrain.danger[i][6 - 1] *= 8.2f / 8.0f;
            m_BotBrain.danger[i][5 - 1] *= 7.5f / 8.7f;
            m_BotBrain.danger[i][4 - 1] *= 9.3f / 8.0f;
         }
         if (d[i][3 - 1]) {
            m_BotBrain.danger[i][1 - 1] *= 0.9f / 2.5f;
            m_BotBrain.danger[i][2 - 1] *= 1.9f / 4.4f;
            m_BotBrain.danger[i][4 - 1] *= 6.6f / 8.0f;
            m_BotBrain.danger[i][5 - 1] *= 8.6f / 8.7f;
            m_BotBrain.danger[i][6 - 1] *= 6.5f / 8.0f;
         }
         if (d[i][7 - 1]) {
            m_BotBrain.danger[i][9 - 1] *= 0.9f / 2.5f;
            m_BotBrain.danger[i][8 - 1] *= 1.9f / 4.4f;
            m_BotBrain.danger[i][6 - 1] *= 6.6f / 8.0f;
            m_BotBrain.danger[i][5 - 1] *= 8.6f / 8.7f;
            m_BotBrain.danger[i][4 - 1] *= 6.5f / 8.0f;
         }
         if (d[i][4 - 1]) {
            m_BotBrain.danger[i][2 - 1] *= 2.4f / 4.4f;
            m_BotBrain.danger[i][3 - 1] *= 3.5f / 7.0f;
            m_BotBrain.danger[i][5 - 1] *= 7.6f / 8.0f;
            m_BotBrain.danger[i][6 - 1] *= 8.0f / 8.7f;
            m_BotBrain.danger[i][7 - 1] *= 5.3f / 7.0f;
         }
         if (d[i][6 - 1]) {
            m_BotBrain.danger[i][8 - 1] *= 2.4f / 4.4f;
            m_BotBrain.danger[i][7 - 1] *= 3.5f / 7.0f;

            m_BotBrain.danger[i][5 - 1] *= 7.6f / 8.0f;
            m_BotBrain.danger[i][4 - 1] *= 8.0f / 8.7f;
            m_BotBrain.danger[i][3 - 1] *= 5.3f / 7.0f;
         }
         if (d[i][5 - 1]) {
            m_BotBrain.danger[i][2 - 1] *= 1.9f / 4.4f;
            m_BotBrain.danger[i][3 - 1] *= 5.7f / 7.0f;
            m_BotBrain.danger[i][4 - 1] *= 5.4f / 8.0f;
            m_BotBrain.danger[i][6 - 1] *= 5.4f / 8.0f;
            m_BotBrain.danger[i][7 - 1] *= 5.7f / 7.0f;
            m_BotBrain.danger[i][8 - 1] *= 1.9f / 4.4f;
         }
      }
      for (i = 0; i < 3; i++) {
         for (j = 0; j < 5; j++) {
            if (!d2[i][j] && !d2[i][j + 1] && !d2[i][j + 2]) {
               m_BotBrain.danger[i][j + 1] *= 1.2f;
            }
         }
         for (j = 0; j < 4; j++) {
            if (!d2[i][j] && !d2[i][j + 1] && !d2[i][j + 2] && !d2[i][j + 3]) {
               m_BotBrain.danger[i][j] *= 1.3f;
               m_BotBrain.danger[i][j + 3] *= 1.3f;
            } 
         }
      }
      for (i = 0; i < 3; i++) {
         if (rest_kind[i][2 - 1] == 0) {
            m_BotBrain.danger[i][1 - 1] *= 0.3f;
         }
         if (rest_kind[i][2 - 1] == 1) {
            m_BotBrain.danger[i][1 - 1] *= 0.6f;
         }
         if (rest_kind[i][8 - 1] == 0) {
            m_BotBrain.danger[i][9 - 1] *= 0.3f;
         }
         if (rest_kind[i][8 - 1] == 1) {
            m_BotBrain.danger[i][9 - 1] *= 0.6f;
         }
         if (rest_kind[i][3 - 1] == 0) {
            m_BotBrain.danger[i][1 - 1] *= 0.3f;
            m_BotBrain.danger[i][2 - 1] *= 0.3f;
         }
         if (rest_kind[i][3 - 1] == 1) {
            m_BotBrain.danger[i][1 - 1] *= 0.6f;
            m_BotBrain.danger[i][2 - 1] *= 0.6f;
         }
         if (rest_kind[i][7 - 1] == 0) {
            m_BotBrain.danger[i][9 - 1] *= 0.3f;
            m_BotBrain.danger[i][8 - 1] *= 0.3f;
         }
         if (rest_kind[i][7 - 1] == 1) {
            m_BotBrain.danger[i][9 - 1] *= 0.6f;
            m_BotBrain.danger[i][8 - 1] *= 0.6f;
         }
         for (j = 0; j < 6; j++) {
            n = rest_kind[i][j] + rest_kind[i][j + (4 - 1)];
            if (n > 4) {
               m_BotBrain.danger[i][j] *= 1.3f;
               m_BotBrain.danger[i][j + (4 - 1)] *= 1.3f;
            }
         }
      }
      for (i = 0; i < 4; i++) {
         switch (rest_kind[3][i]) {
            case 0: m_BotBrain.danger[3][i] = 0.0f; break;
            case 1: m_BotBrain.danger[3][i] *= 0.3f; break;
            case 2: m_BotBrain.danger[3][i] *= 1.0f; break;
            case 3: m_BotBrain.danger[3][i] *= 3.0f; break;
            case 4: m_BotBrain.danger[3][i] *= 10.0f; break;
         }
      }
      for (i = 0; i < 3; i++) {
         switch (rest_kind[4][i]) {

            case 0: m_BotBrain.danger[4][i] = 0.0f; break;
            case 1: m_BotBrain.danger[4][i] *= 0.3f; break;
            case 2: m_BotBrain.danger[4][i] *= 1.0f; break;
            case 3: m_BotBrain.danger[4][i] *= 3.0f; break;
            case 4: m_BotBrain.danger[4][i] *= 10.0f; break;
         }
      }
   }
#ifdef BOT_DEBUG
   fprintf(stdout, "-=-=-= CBot::UpdateDanger() result =-=-=-\n");
   for (i = 0; i < 5; i++) {
      for (j = 0; j < 9; j++) {
         fprintf(stdout, "%3.2f ", m_BotBrain.danger[i][j]);
      }
      fprintf(stdout, "\n");
   }
   fprintf(stdout, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
#endif
}

// Returns how "dangerous" discarding this tile is.
float CBot::TileDanger(const CTile &t)
{
   return m_BotBrain.danger[t.index1()][t.index2()];
}

void CBot::AnalyzeGoal()
{
   int i, j, num_chow = 0, num_pung = 0;

   int dorapt = 0, dorapt_minor = 0, dorapt_char = 0;
   int dorapt_bamboo = 0, dorapt_circle = 0;
   int yakupt = 0;

   int char3[2] = {0, 0}, char2[2] = {0, 0};
   int bamboo3[2] = {0, 0}, bamboo2[2] = {0, 0};
   int circle3[2] = {0, 0}, circle2[2] = {0, 0};
   int wd3[2] = {0, 0}, wd2[2] = {0, 0};

   int num_char = 0, num_bamboo = 0, num_circle = 0, num_wd = 0;

   int minor3 = 0, minor3f = 0, minor2 = 0, minorn = 0;
   int v2_num = 0, v3_num = 0, major_num = 0, major_kinds = 0;

   int stage = m_iNumDiscarded / 7;
   int kinds[5][9];

   AnalyzeHand();

#ifdef BOT_DEBUG
   fprintf(stdout, "-----------AnalyzeHand()---------\n");
   for (i = 0; i < m_BotBrain.num_handresult; i++) {
      for (j = 0; j < m_BotBrain.handresult[i].m_iNumTileSets; j++) {
         fprintf(stdout, "%d first=%s type=%x\n", j,
            m_BotBrain.handresult[i].m_TileSets[j].first.GetCode(),
            m_BotBrain.handresult[i].m_TileSets[j].type);
      }
      for (j = 0; j < m_BotBrain.handresult[i].m_iNumTiles; j++) {
         fprintf(stdout, "%d %s %d %x\n", j,
            m_BotBrain.handresult[i].m_Tiles[j].tile.GetCode(),
            m_BotBrain.handresult[i].m_Tiles[j].tileset,
            m_BotBrain.handresult[i].m_Tiles[j].flags);
      }

      fprintf(stdout, "FREE = %d, REST = %d\n", m_BotBrain.handresult_ft[i], m_BotBrain.handresult_rest);
      fprintf(stdout, "---------------------------------\n");
   }
   fprintf(stdout, "=====================================\n\n");
#endif

   // clear the goal value array
   for (i = 0; i < GOAL_COUNT; i++) {
      m_BotBrain.goalvalue[i] = 0.0f;
   }

   // clear the kinds array
   for (i = 0; i < 5; i++) {
      for (j = 0; j < 9; j++) {
         kinds[i][j] = 0;
      }
   }

   bool concealed = m_Hand.IsConcealed();

   if (!concealed) {
      // non-concealed hand
      m_BotBrain.goalvalue[GOAL_WONDERS] = -1;
      m_BotBrain.goalvalue[GOAL_PAIRS] = -1;

      for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
         minor3f++;
         if (m_Hand.m_TileSets[i].type & (HT_OPENPUNG | HT_OPENKONG | HT_CLOSEDKONG)) {
            num_pung++;
            CTile &t = m_Hand.m_TileSets[i].first;
            kinds[t.index1()][t.index2()] += 3;
            if (t.IsMajor()) {
               m_BotBrain.goalvalue[GOAL_ALLMINOR] = -1;
            }
            if (t.GetSuit() & TILESUIT_CHAR) {
               num_char += 3;
               char3[0]++;
               char3[1]++;
            } else if (t.GetSuit() & TILESUIT_BAMBOO) {
               num_bamboo += 3;
               bamboo3[0]++;
               bamboo3[1]++;
            } else if (t.GetSuit() & TILESUIT_CIRCLE) {
               num_circle += 3;
               circle3[0]++;
               circle3[1]++;
            } else {
               num_wd += 3;
               wd3[0]++;
               wd3[1]++;

               if (t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW")) ||
                  t.GetSuit() == TILESUIT_DRAGON) {
                  yakupt += 15;
               }
               if (t == (m_iTurn == TURN_EAST ? CTile("EW") : CTile("SW"))) {
                  yakupt += 15;
               }
            }
         } else if (m_Hand.m_TileSets[i].type & HT_OPENCHOW) {
            num_chow++;
            m_BotBrain.goalvalue[GOAL_ALLPUNG] = -1;
            CTile &t = m_Hand.m_TileSets[i].first;
            assert(t.index2() < 7);
            kinds[t.index1()][t.index2()]++;
            kinds[t.index1()][t.index2() + 1]++;
            kinds[t.index1()][t.index2() + 2]++;
            if (t.GetValue() == 7 || t.IsMajor()) {
               m_BotBrain.goalvalue[GOAL_ALLMINOR] = -1;
            }
            if (t.GetSuit() & TILESUIT_CHAR) {
               num_char += 3;
               char3[0]++;
               char3[1]++;
            } else if (t.GetSuit() & TILESUIT_BAMBOO) {
               num_bamboo += 3;
               bamboo3[0]++;
               bamboo3[1]++;
            } else if (t.GetSuit() & TILESUIT_CIRCLE) {
               num_circle += 3;
               circle3[0]++;
               circle3[1]++;
            } else {
               TerminateOnError("CBot::AnalyzeGoal(): chow is non-numeric");
            }
         }
      }

      // check for open doras
      for (i = 0; i < m_Hand.m_iNumTiles; i++) {
         if (m_Hand.m_Tiles[i].flags & HT_OPEN) {
            CTile &t = m_Hand.m_Tiles[i].tile;
            if (gpGame->IsDora(t, false)) {
               dorapt += 10;
               if (!t.IsMajor()) {
                  dorapt_minor += 10;
               }
               if (t.GetSuit() & TILESUIT_CHAR) {

                  dorapt_char++;
               } else if (t.GetSuit() & TILESUIT_BAMBOO) {
                  dorapt_bamboo++;
               } else if (t.GetSuit() & TILESUIT_CIRCLE) {
                  dorapt_circle++;
               }
            }
         }
      }
   }

   for (i = 0; i < m_BotBrain.num_handresult; i++) {
      CHand &h = m_BotBrain.handresult[i];
      for (j = 0; j < h.m_iNumTileSets; j++) {
         if (h.m_TileSets[i].type & HT_OPEN) {
            continue; // skip open sets
         }

         if (h.m_TileSets[i].type & HT_CLOSEDPUNG) {
            if (h.m_TileSets[i].first.IsMajor()) {
               m_BotBrain.goalvalue[GOAL_ALLMINOR] = -1;
            } else {
               minor3++;
            }
         } else if (h.m_TileSets[i].type & HT_CLOSEDCHOW) {
            if (h.m_TileSets[i].first.GetValue() != 1 &&
               h.m_TileSets[i].first.GetValue() != 7) {

               minor3++;
            }
         } else if (h.m_TileSets[i].type & (HT_PAIR | HT_TOCHOW)) {
            if (h.m_TileSets[i].first.IsMinor()) {
               minor2++;
            }
         }

         if (i < 2) {
            switch (h.m_TileSets[j].first.GetSuit()) {
               case TILESUIT_CHAR:
                  if (h.m_TileSets[j].type & (HT_PAIR | HT_TOCHOW))
                     char2[i]++;
                  else
                     char3[i]++;
                  break;

               case TILESUIT_BAMBOO:
                  if (h.m_TileSets[j].type & (HT_PAIR | HT_TOCHOW))
                     bamboo2[i]++;
                  else
                     bamboo3[i]++;
                  break;

               case TILESUIT_CIRCLE:
                  if (h.m_TileSets[j].type & (HT_PAIR | HT_TOCHOW))
                     circle2[i]++;
                  else
                     circle3[i]++;
                  break;

               default:
                  if (h.m_TileSets[j].type & (HT_PAIR | HT_TOCHOW))
                     wd2[i]++;
                  else
                     wd3[i]++;
                  break;
            }
         }
      }
      if (minor3 * 2 + minor2 > minorn)
         minorn = minor3 * 2 + minor2;
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & HT_LOCKED) {
         continue; // skip locked tiles
      }
      CTile &t = m_Hand.m_Tiles[i].tile;
      kinds[t.index1()][t.index2()]++;
      if (gpGame->IsDora(t, false)) {
         dorapt += 10;
         if (!t.IsMajor()) {
            dorapt_minor += 10;
         }
         if (t.GetSuit() & TILESUIT_CHAR) {
            dorapt_char++;
         } else if (t.GetSuit() & TILESUIT_BAMBOO) {
            dorapt_bamboo++;
         } else if (t.GetSuit() & TILESUIT_CIRCLE) {
            dorapt_circle++;
         }
      }
   }

   for (i = 0; i < 5; i++) {
      for (j = 0; j < 9; j++) {
         if ((i == 3 && j > 3) || (i == 4 && j > 2))
            break;
         CTile t = CTile((TILESUIT_CHAR << i) | (j + 1));
         if (kinds[i][j] == 2) {
            v2_num++;
         } else if (kinds[i][j] == 3) {
            v3_num++;
         }
         if (t.IsMajor()) {
            major_num += kinds[i][j];
            if (kinds[i][j]) {
               major_kinds++;
            }
            if (kinds[i][j] == 0 && CTile::RemainingTile(t) == 0) {
               m_BotBrain.goalvalue[GOAL_WONDERS] = -1;
            }
         } else {
            if (kinds[i][j] >= 3 ||
               (kinds[i][j] == 2 && CTile::RemainingTile(t) > 0)) {
               if (t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW")) ||
                  t.GetSuit() == TILESUIT_DRAGON) {
                  yakupt += ((kinds[i][j] >= 3) ? 15 : 10);
               }
               if (t == (m_iTurn == TURN_EAST ? CTile("EW") : CTile("SW"))) {
                  yakupt += ((kinds[i][j] >= 3) ? 15 : 10);
               }
            }
         }
      }
   }

   if (m_BotBrain.goalvalue[GOAL_PAIRS] >= 0) {
      m_BotBrain.goalvalue[GOAL_PAIRS] = (v2_num + v3_num) * 700 / (4 + stage);
      m_BotBrain.goalvalue[GOAL_PAIRS] += (yakupt + dorapt) * 5;
      m_BotBrain.goalvalue[GOAL_PAIRS] += m_BotBrain.aggression * 50;
   }

   if (m_BotBrain.goalvalue[GOAL_WONDERS] >= 0) {
      m_BotBrain.goalvalue[GOAL_WONDERS] = major_kinds * 700 / (10 - stage);
      m_BotBrain.goalvalue[GOAL_WONDERS] -= m_BotBrain.aggression * 50;
   }

   if (m_BotBrain.handresult_rest < 3 && yakupt > 0) {
      m_BotBrain.goalvalue[GOAL_FANPAI] = (dorapt + yakupt) * 20;
      m_BotBrain.goalvalue[GOAL_FANPAI] += (3 - m_BotBrain.handresult_rest) * 100;
      m_BotBrain.goalvalue[GOAL_FANPAI] += m_BotBrain.aggression * 50;
   }

   if (m_BotBrain.goalvalue[GOAL_ALLPUNG] >= 0) {
      int n = (v2_num > 5 - v3_num) ? (5 - v3_num) : v2_num;
      m_BotBrain.goalvalue[GOAL_ALLPUNG] = (v3_num * 2 + n) * 700 / 6;
      m_BotBrain.goalvalue[GOAL_ALLPUNG] += (yakupt + dorapt) * 5;
      m_BotBrain.goalvalue[GOAL_ALLPUNG] += m_BotBrain.aggression * 50;
   }

   if (num_bamboo <= 0 && num_circle <= 0 && bamboo2[0] <= 0 &&
      bamboo2[1] <= 0 && circle2[0] <= 0 && circle2[1] <= 0) {
      int n = (char3[0] + wd3[0]) * 2 + char2[0] + wd2[0];
      if (n < (char3[1] + wd3[1]) * 2 + char2[1] + wd2[1]) {
         n = (char3[1] + wd3[1]) * 2 + char2[1] + wd2[1];
      }
      m_BotBrain.goalvalue[GOAL_ALLCHAR] = n * 700 / 6;
      m_BotBrain.goalvalue[GOAL_ALLCHAR] += (yakupt + dorapt_char) * 5;
      m_BotBrain.goalvalue[GOAL_ALLCHAR] += m_BotBrain.aggression * 50;
   }

   if (num_char <= 0 && num_circle <= 0 && char2[0] <= 0 &&
      char2[1] <= 0 && circle2[0] <= 0 && circle2[1] <= 0) {
      int n = (bamboo3[0] + wd3[0]) * 2 + bamboo2[0] + wd2[0];
      if (n < (bamboo3[1] + wd3[1]) * 2 + bamboo2[1] + wd2[1]) {
         n = (bamboo3[1] + wd3[1]) * 2 + bamboo2[1] + wd2[1];
      }
      m_BotBrain.goalvalue[GOAL_ALLBAMBOO] = n * 700 / 6;
      m_BotBrain.goalvalue[GOAL_ALLBAMBOO] += (yakupt + dorapt_bamboo) * 5;
      m_BotBrain.goalvalue[GOAL_ALLBAMBOO] += m_BotBrain.aggression * 50;
   }

   if (num_bamboo <= 0 && num_char <= 0 && bamboo2[0] <= 0 &&
      bamboo2[1] <= 0 && char2[0] <= 0 && char2[1] <= 0) {
      int n = (circle3[0] + wd3[0]) * 2 + circle2[0] + wd2[0];
      if (n < (circle3[1] + wd3[1]) * 2 + circle2[1] + wd2[1]) {
         n = (circle3[1] + wd3[1]) * 2 + circle2[1] + wd2[1];
      }
      m_BotBrain.goalvalue[GOAL_ALLCIRCLE] = n * 700 / 6;
      m_BotBrain.goalvalue[GOAL_ALLCIRCLE] += (yakupt + dorapt_circle) * 5;
      m_BotBrain.goalvalue[GOAL_ALLCIRCLE] += m_BotBrain.aggression * 50;
   }

   if (m_BotBrain.goalvalue[GOAL_ALLMINOR] >= 0) {
      m_BotBrain.goalvalue[GOAL_ALLMINOR] = (minor3f * 2 + minorn) * 100;
      m_BotBrain.goalvalue[GOAL_ALLMINOR] += dorapt_minor * 5;
      m_BotBrain.goalvalue[GOAL_ALLMINOR] -= m_BotBrain.aggression * 50;
   }

#ifdef BOT_DEBUG
   fprintf(stdout, "\n-----------AnalyzeGoal()---------\n");
   for (i = 1; i < GOAL_COUNT; i++) {
      fprintf(stdout, "GOAL %d - VALUE %f\n", i, m_BotBrain.goalvalue[i]);
   }
   fprintf(stdout, "---------------------------------\n\n");
#endif
}

void CBot::AnalyzeHand()
{
   int i, j;

   m_BotBrain.num_handresult = 0;

   for (i = 0; i < 5; i++) {
      for (j = 0; j < 9; j++) {
         m_BotBrain.ahand_kinds[i][j] = 0;
      }
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & HT_LOCKED)
         continue; // skip locked tiles
      CTile &t = m_Hand.m_Tiles[i].tile;
      m_BotBrain.ahand_kinds[t.index1()][t.index2()]++;
   }

   int m = 0, h = 0;

   for (i = 0; i < 5; i++) {
      for (j = 0; j < 9; j++) {
         if ((i == 3 && j > 3) || (i == 4 && j > 2)) {
            break;
         }
         CTile t((TILESUIT_CHAR << i) | (j + 1));
         if (!t.IsMajor()) {
            continue;
         }
         if (m_BotBrain.ahand_kinds[i][j]) {
            m++;
            if (m_BotBrain.ahand_kinds[i][j] >= 2) {
               h = 1;
            }
         }
      }
   }

   m_BotBrain.handresult_rest = 14 - m - h; // for thirteen-wonders

   CHand hand = m_Hand;
   hand.Sort();

   AnalyzeHand_real(hand, 0);
   AnalyzeHand_sort();
}

void CBot::AnalyzeHand_real(CHand &hand, int begin)
{
   int i, j;
   bool ended = true;
   CTile prev = 0;

   for (i = begin; i < hand.m_iNumTiles; i++) {
      if (hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
         continue; // skip locked tiles

      CTile &t = hand.m_Tiles[i].tile;

      if (prev == t)
         continue; // don't process 2 same tiles

      prev = t;

      CHand tmp;

      for (j = 0; j < i; j++) {
         if (tmp.m_Tiles[j].flags & (HT_LOCKED | HT_TOCHOW))
            continue; // skip locked tiles
         if (hand.m_Tiles[i].tile == hand.m_Tiles[j].tile)
            break;
      }

      if (j >= i) {
         tmp = hand;
         if (tmp.Pung(tmp.m_Tiles[i].tile, false)) {
            ended = false;
            AnalyzeHand_real(tmp, i + 1);
         }
      }

      tmp = hand;
      if (tmp.Chow(tmp.m_Tiles[i].tile, CHOW_LOWER, false)) {
         ended = false;
         AnalyzeHand_real(tmp, i + 1);
      }

      tmp = hand;
      for (j = i + 1; j < tmp.m_iNumTiles; j++) {
         if (tmp.m_Tiles[j].flags & (HT_LOCKED | HT_TOCHOW))
            continue; // skip locked tiles

         int f = 0;

         if (tmp.m_Tiles[i].tile.GetSuit() == tmp.m_Tiles[j].tile.GetSuit()) {
            if (tmp.m_Tiles[i].tile == tmp.m_Tiles[j].tile) {
               f = HT_PAIR;
            } else if (tmp.m_Tiles[i].tile.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)) {
               // do nothing here
            } else if (tmp.m_Tiles[i].tile.GetValue() == tmp.m_Tiles[j].tile.GetValue() - 1) {
               if (tmp.m_Tiles[i].tile.GetValue() != 1) {
                  f |= HT_TOCHOWLOWER;
               }
               if (tmp.m_Tiles[i].tile.GetValue() != 8) {
                  f |= HT_TOCHOWUPPER;
               }
            } else if (tmp.m_Tiles[i].tile.GetValue() == tmp.m_Tiles[j].tile.GetValue() - 2) {
               f = HT_TOCHOWMIDDLE;
            }
         }

         if (f != 0) {
            tmp.m_Tiles[i].flags |= f;
            tmp.m_Tiles[j].flags |= f;
            tmp.m_Tiles[i].tileset = tmp.m_Tiles[j].tileset = tmp.m_iNumTileSets;

            // Add this pair to the tileset
            tmp.m_TileSets[tmp.m_iNumTileSets].first = tmp.m_Tiles[i].tile;
            tmp.m_TileSets[tmp.m_iNumTileSets].type = f;
            tmp.m_iNumTileSets++;

            ended = false;
            AnalyzeHand_real(tmp, i + 1);

            break; // no need to go further
         }
      }
   }

   if (ended) {
      int ft = 0;
      for (i = 0; i < hand.m_iNumTiles; i++) {
         if (hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW))
            continue;
         ft++;
      }
      AnalyzeHand_add(hand, ft);
   }
}

void CBot::AnalyzeHand_add(CHand &hand, int ft)
{
   int i, rest, v2 = 0, two = 0, three = 0, total = 0, m, over;

   for (i = 0; i < hand.m_iNumTileSets; i++) {
      if (hand.m_TileSets[i].type & (HT_LOCKED & ~HT_PAIR)) {
         three++;
         total += 3;
      } else {
         two++;
         total += 2;
         if (hand.m_TileSets[i].type & HT_PAIR) {
            v2++;
         }
      }
   }

   three += (14 - total - ft) / 3;

   if (v2) {
      m = two + three - 1;
   } else {
      m = two + three;
   }

   if (m > 4) {
      over = m - 4;
   } else {
      over = 0;
   }

   rest = 9 - (three + three + two) + over;

   if (rest > 7 - v2) {
      rest = 7 - v2;
      if (v2 == 6) {
         int same = 0, j;
         for (i = 0; i < hand.m_iNumTiles; i++) {
            if (hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
               continue;
            }
            for (j = 0; j < hand.m_iNumTileSets; j++) {
               if (hand.m_Tiles[i].tile == hand.m_TileSets[j].first) {
                  same++;
               }
            }
         }
         if (same == ft) {
            rest++;
         }
      }
   }

   // check for unready hand
   if (rest == 1 && three == 4 && ft == 1) {
      CTile r = 0;
      for (i = 0; i < hand.m_iNumTiles; i++) {
         if (hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
            continue;
         }
         r = hand.m_Tiles[i].tile;
         break;
      }
      for (i = 0; i < hand.m_iNumTileSets; i++) {
         if (hand.m_TileSets[i].type &
            (HT_OPENPUNG | HT_CLOSEDPUNG | HT_OPENKONG | HT_CLOSEDKONG)) {
            if (hand.m_TileSets[i].first == r) {
               rest++;
               break;
            }
         }
      }
   }

   if (rest == 1 && three == 3) {
      CTile m1 = 0, m2 = 0;
      for (i = 0; i < hand.m_iNumTileSets; i++) {
         if (hand.m_TileSets[i].type & HT_TOCHOWUPPER) {
            m1 = hand.m_TileSets[i].first() - 1;
         }
         if (hand.m_TileSets[i].type & HT_TOCHOWMIDDLE) {
            m1 = hand.m_TileSets[i].first() + 1;
         }
         if (hand.m_TileSets[i].type & HT_TOCHOWLOWER) {
            m2 = hand.m_TileSets[i].first() + 2;
         }
      }

      bool p = false;

      if (m1() != 0 && m2() != 0) {
         if (m_BotBrain.ahand_kinds[m1.index1()][m1.index2()] +
            m_BotBrain.ahand_kinds[m2.index1()][m2.index2()] == 8) {
            p = true;
         }
      }

      if (m1() != 0 && m2() == 0 && m_BotBrain.ahand_kinds[m1.index1()][m1.index2()] == 4) {
         p = true;
      }

      if (m1() == 0 && m2() != 0 && m_BotBrain.ahand_kinds[m2.index1()][m2.index2()] == 4) {
         p = true;
      }

      rest += (p ? 1 : 0);
   }

   if (m_BotBrain.handresult_rest > rest) {
      m_BotBrain.handresult_rest = rest;
      m_BotBrain.num_handresult = 0;
   } else if (m_BotBrain.handresult_rest < rest) {
      return; // discard this one
   }

   if (m_BotBrain.num_handresult > MAX_HANDRESULT) {
      fprintf(stderr, "WARNING, max hand result reached\n");
      return;
   }

   m_BotBrain.handresult[m_BotBrain.num_handresult] = hand;
   m_BotBrain.handresult_ft[m_BotBrain.num_handresult] = ft;
   m_BotBrain.num_handresult++;
}

// sort the analyze results by number of "free" tiles
void CBot::AnalyzeHand_sort()
{
   CHand tmp;
   int i, j, t;

   for (i = 0; i < m_BotBrain.num_handresult; i++) {
      bool changed = false;
      for (j = 0; j < m_BotBrain.num_handresult - i - 1; j++) {
         if (m_BotBrain.handresult_ft[j + 1] < m_BotBrain.handresult_ft[j]) {
            tmp = m_BotBrain.handresult[j + 1];
            m_BotBrain.handresult[j + 1] = m_BotBrain.handresult[j];
            m_BotBrain.handresult[j] = tmp;

            t = m_BotBrain.handresult_ft[j + 1];
            m_BotBrain.handresult_ft[j + 1] = m_BotBrain.handresult_ft[j];
            m_BotBrain.handresult_ft[j] = t;

            changed = true;
         }
      }
      if (!changed) {
         break;

      }
   }
}

void CBot::SelectGoal()
{
   int goal = GOAL_NULL, i;
   float value = 0;

   for (i = 0; i < GOAL_COUNT; i++) {
      if (m_BotBrain.goalvalue[i] > value) {
         goal = i;
         value = m_BotBrain.goalvalue[i];
      }
   }

   m_BotBrain.goal = (value >= 700) ? goal : GOAL_NULL;

#ifdef BOT_DEBUG
   fprintf(stdout, "CBot::SelectGoal(): goal = %d\n", m_BotBrain.goal);
#endif
}

bool CBot::WantToChow()
{
   if (m_BotBrain.goal == GOAL_ALLPUNG ||
      m_BotBrain.goal == GOAL_PAIRS ||
      m_pOpponent->m_iNumDiscarded <= 0 ||
      m_BotBrain.goal == GOAL_NULL)
      return false;

   const int param = 300;

   int i, rest = m_BotBrain.handresult_rest, goal = m_BotBrain.goal;
   int chow_dir = -1;
   CHand tmp = m_Hand;
   botbrain_t bak = m_BotBrain;

   for (i = CHOW_LOWER; i <= CHOW_UPPER; i++) {
      if (m_Hand.Chow(m_pOpponent->LastDiscardedTile(), i)) {
         AnalyzeHand();
         AnalyzeGoal();
         SelectGoal();
         if (m_BotBrain.handresult_rest < rest ||
            (m_BotBrain.handresult_rest == rest && m_BotBrain.goal > goal)) {
            rest = m_BotBrain.handresult_rest;
            goal = m_BotBrain.goal;
            chow_dir = i;
         }
         m_Hand = tmp;
      }
   }

   m_BotBrain = bak;

   if (goal > m_BotBrain.goal ||
      (goal == m_BotBrain.goal && rest < m_BotBrain.handresult_rest)) {
#ifdef BOT_DEBUG
      fprintf(stdout, "CBot::WantToChow(): check 1 ok (%d, %d, %d, %d)\n",
         goal, m_BotBrain.goal, rest, m_BotBrain.handresult_rest);
#endif
      m_BotBrain.action = PA_CHOW;
      m_BotBrain.chowlocation = (chow_location)chow_dir;
      return true;
   }

   bool has_fanpai = false;

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (!(m_Hand.m_TileSets[i].type &
         (HT_OPENPUNG | HT_CLOSEDPUNG | HT_OPENKONG | HT_CLOSEDKONG)))
         continue;

      const CTile &t = m_Hand.m_TileSets[i].first;
      if (t.GetSuit() == TILESUIT_DRAGON ||
         t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         has_fanpai = true;
         break;
      }

      if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         has_fanpai = true;
         break;
      }
   }

   if (rest < m_BotBrain.handresult_rest && has_fanpai &&
      (RandomLong(1, 1000) < param || !m_Hand.IsConcealed())) {
#ifdef BOT_DEBUG
      fprintf(stdout, "CBot::WantToChow(): check 2 ok\n");
#endif
      m_BotBrain.action = PA_CHOW;
      m_BotBrain.chowlocation = (chow_location)chow_dir;
      return true;
   }

   return false;
}

bool CBot::WantToPung()
{
   const int param = 300;

   CHand tmp = m_Hand;
   int rest = m_BotBrain.handresult_rest, goal = m_BotBrain.goal;

   if (goal == GOAL_PAIRS || goal == GOAL_WONDERS ||
      m_pOpponent->m_iNumDiscarded <= 0) {
#ifdef BOT_DEBUG
      fprintf(stdout, "CBot::WantToPung(): goal == GOAL_PAIRS || goal == GOAL_WONDERS\n");
#endif
      return false;
   }

   botbrain_t bak = m_BotBrain;

   if (!m_Hand.Pung(m_pOpponent->LastDiscardedTile())) {
      return false;
   }

#ifdef BOT_DEBUG
   fprintf(stdout, "CBot::WantToPung(): ANALYZE START\n");
#endif

   AnalyzeHand();
   AnalyzeGoal();
   SelectGoal();

#ifdef BOT_DEBUG
   fprintf(stdout, "CBot::WantToPung(): ANALYZE END\n");
#endif

   int goal2 = m_BotBrain.goal, rest2 = m_BotBrain.handresult_rest;
   m_BotBrain = bak;
   m_Hand = tmp;

   if (goal != GOAL_NULL &&
      ((goal2 >= goal && rest2 > rest) || goal2 > goal)) {
#ifdef BOT_DEBUG
      fprintf(stdout, "CBot::WantToPung(): check 1 ok\n");
#endif
      m_BotBrain.action = PA_PUNG;
      return true;
   }

   bool has_fanpai = false, is_fanpai = false;

   for (int i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (!(m_Hand.m_TileSets[i].type &
         (HT_OPENPUNG | HT_CLOSEDPUNG | HT_OPENKONG | HT_CLOSEDKONG)))
         continue;

      const CTile &t = m_Hand.m_TileSets[i].first;
      if (t.GetSuit() == TILESUIT_DRAGON ||
         t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         has_fanpai = true;
         break;
      }

      if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         has_fanpai = true;
         break;
      }
   }

   if (!has_fanpai) {
      const CTile &t = m_pOpponent->LastDiscardedTile();
      if (t.GetSuit() == TILESUIT_DRAGON ||
         t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         is_fanpai = true;
      }

      if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         is_fanpai = true;
      }
   }

   if ((RandomLong(1, 1000) < param || !m_Hand.IsConcealed()) &&
      (has_fanpai || is_fanpai) && rest2 < rest) {
#ifdef BOT_DEBUG

      fprintf(stdout, "CBot::WantToPung(): check 2 ok\n");
#endif
      m_BotBrain.action = PA_PUNG;
      return true;
   }

   return false;
}

bool CBot::WantToOpenKong()
{
   const int param = 300;

   CHand tmp = m_Hand;
   int rest = m_BotBrain.handresult_rest, goal = m_BotBrain.goal;

   if (goal == GOAL_PAIRS || goal == GOAL_WONDERS ||
      m_pOpponent->m_iNumDiscarded <= 0) {
#ifdef BOT_DEBUG
      fprintf(stdout, "CBot::WantToKong(): goal == GOAL_PAIRS || goal == GOAL_WONDERS\n");
#endif
      return false;
   }

   botbrain_t bak = m_BotBrain;

   if (!m_Hand.Kong(m_pOpponent->LastDiscardedTile())) {
      return false;
   }

#ifdef BOT_DEBUG
   fprintf(stdout, "CBot::WantToKong(): ANALYZE START\n");
#endif

   AnalyzeHand();
   AnalyzeGoal();
   SelectGoal();

#ifdef BOT_DEBUG
   fprintf(stdout, "CBot::WantToKong(): ANALYZE END\n");
#endif

   int num2 = m_BotBrain.num_handresult, rest2 = m_BotBrain.handresult_rest;
   m_BotBrain = bak;
   m_Hand = tmp;

   if (rest2 > 2 && RandomLong(1, 1000) < param) {
#ifdef BOT_DEBUG
      fprintf(stdout, "CBot::WantToKong(): check 1 ok\n");
#endif
      m_BotBrain.action = PA_KONG;
      return true;
   }


   bool has_fanpai = false, is_fanpai = false;

   for (int i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (!(m_Hand.m_TileSets[i].type &
         (HT_OPENPUNG | HT_CLOSEDPUNG | HT_OPENKONG | HT_CLOSEDKONG)))
         continue;

      const CTile &t = m_Hand.m_TileSets[i].first;
      if (t.GetSuit() == TILESUIT_DRAGON ||
         t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         has_fanpai = true;
         break;
      }


      if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         has_fanpai = true;
         break;
      }
   }

   if (!has_fanpai) {
      const CTile &t = m_pOpponent->LastDiscardedTile();
      if (t.GetSuit() == TILESUIT_DRAGON ||
         t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         is_fanpai = true;
      }

      if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
         is_fanpai = true;
      }
   }

   if ((RandomLong(1, 1000) < param || !m_Hand.IsConcealed()) &&
      (is_fanpai || (has_fanpai && num2 >= m_BotBrain.num_handresult && rest2 <= m_BotBrain.handresult_rest))) {
#ifdef BOT_DEBUG
      fprintf(stdout, "CBot::WantToKong(): check 2 ok\n");
#endif
      m_BotBrain.action = PA_KONG;
      return true;
   }

   return false;
}

bool CBot::WantToClosedKong()
{
   return false; // TODO
}

bool CBot::WantToReach()
{
   if (m_pOpponent->m_iNumDiscarded >= 20) {
      return false;
   }

   const int param = 700;

   if (m_BotBrain.handresult_rest == 1) {
      if (!m_Hand.IsConcealed()) {
         return false;
      }
      CHand tmp = m_Hand;
      tmp.RemoveTile(m_BotBrain.act_index);
      if (!tmp.IsReady()) {
         return false;
      }
      if (RandomLong(1, 1000) * 20 < param * (20 - m_iNumDiscarded)) {
         m_BotBrain.action = PA_REACH;
         return true;
      }
   }

   return false;
}

void CBot::AnalyzeDiscard()
{
   int rests[MAX_HANDTILE], i;
   botbrain_t bak = m_BotBrain;

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      CHand tmp = m_Hand;
      if (m_Hand.RemoveTile(i)) {
         AnalyzeHand();
         rests[i] = m_BotBrain.handresult_rest;
      } else {
         rests[i] = 9999;
      }
      m_Hand = tmp;
   }

   m_BotBrain = bak;

   if (m_BotBrain.goal == GOAL_WONDERS) {
      int n, kinds[5][9];

      for (i = 0; i < 5; i++) {
         for (n = 0; n < 9; n++) {
            kinds[i][n] = 0;
         }
      }

      for (i = 0; i < m_Hand.m_iNumTiles; i++) {
         CTile &t = m_Hand.m_Tiles[i].tile;
         kinds[t.index1()][t.index2()]++;
      }

      for (i = 0; i < m_Hand.m_iNumTiles; i++) {
         CTile &t = m_Hand.m_Tiles[i].tile;
         if (t.IsMajor()) {
            if (kinds[t.index1()][t.index2()] == 1) {
               m_BotBrain.tilevalue[i] = 2000.0f;
            } else {
               m_BotBrain.tilevalue[i] = (m_BotBrain.handresult_rest < rests[i]) ? 1000 : 500;
            }
         } else {
            m_BotBrain.tilevalue[i] = (m_BotBrain.handresult_rest < rests[i]) ? 250 : 0;
         }
      }
   } else if (m_BotBrain.goal == GOAL_PAIRS) {
      int n, kinds[5][9];

      for (i = 0; i < 5; i++) {
         for (n = 0; n < 9; n++) {
            kinds[i][n] = 0;
         }
      }

      for (i = 0; i < m_Hand.m_iNumTiles; i++) {
         CTile &t = m_Hand.m_Tiles[i].tile;
         kinds[t.index1()][t.index2()]++;
      }

      for (i = 0; i < n; i++) {
         CTile &t = m_Hand.m_Tiles[i].tile;
         if (kinds[t.index1()][t.index2()] == 2) {
            m_BotBrain.tilevalue[i] = 2000.0f;
         } else if (m_BotBrain.handresult_rest < rests[i]) {
            m_BotBrain.tilevalue[i] = 1000.0f;
         } else {
            int rem = CTile::RemainingTile(t);
            m_BotBrain.tilevalue[i] = rem * rem * 60.0f;
         }
      }
   } else {
      CalcTileValue(rests);
   }

   float max = 0;
   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_BotBrain.tilevalue[i] > max) {
         max = m_BotBrain.tilevalue[i];
      }
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (max != 0) {
         m_BotBrain.tilevalue[i] *= 1000 + m_iNumDiscarded * 20;
         m_BotBrain.tilevalue[i] /= max;
      }
   }

   AnalyzeDora();
   AnalyzeColor();
   AnalyzeZone();
   AnalyzeNum();
   AnalyzeDanger();

#ifdef BOT_DEBUG
   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }
      int loc = IndexToLoc(i), x, y;
      x = DRAW_LOCATION(loc);
      y = (loc % 2) ? 5 : 32;
      gpGeneral->DrawUTF8Text(va("%3.1f", m_BotBrain.tilevalue[i]), x, y, 0, 255, 255, 0);
   }
   gpGeneral->UpdateScreen();
   gpGeneral->WaitForAnyKey();
#endif

   // discard the tile with the lowest value
   float min = 9999;
   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }
      if (m_BotBrain.tilevalue[i] < min) { 
         min = m_BotBrain.tilevalue[i];
         m_BotBrain.act_index = i;
      }
   }
}

void CBot::CalcTileValue(int rests[])
{
   int i, j, x, v2_num;
   bool fanpai_fixed = false;

   struct pnt_s {
      int num;
      int point[MAX_HANDTILE];
      CTile tile[MAX_HANDTILE];
   };

   pnt_s p;
   pnt_s result_pnt;

   p.num = result_pnt.num = 0;

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type & (HT_OPENKONG | HT_OPENPUNG | HT_CLOSEDKONG)) {
         const CTile &t = m_Hand.m_TileSets[i].first;
         if (t.GetSuit() == TILESUIT_DRAGON ||
            t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW")))
            fanpai_fixed = true;

         if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW")))
            fanpai_fixed = true;
      }
   }

   for (x = 0; x < m_BotBrain.num_handresult; x++) {
      CHand &r = m_BotBrain.handresult[x];
      p.num = 0;
      v2_num = 0;
      for (i = 0; i < r.m_iNumTileSets; i++) {
         if (r.m_TileSets[i].type & (HT_PAIR | HT_TOCHOW)) {
            v2_num++;
         }
      }
      for (i = 0; i < r.m_iNumTileSets; i++) {
         if (r.m_TileSets[i].type & (HT_OPENCHOW | HT_CLOSEDCHOW)) {
            for (j = 0; j < 3; j++) {
               p.tile[p.num] = r.m_TileSets[i].first() + j;
               p.point[p.num] = 1000;
               p.num++;
               assert(p.num <= MAX_HANDTILE);
            }
         } else if (r.m_TileSets[i].type & (HT_OPENPUNG | HT_CLOSEDPUNG | HT_OPENKONG | HT_CLOSEDKONG)) {
            for (j = 0; j < 3; j++) {
               p.tile[p.num] = r.m_TileSets[i].first;
               p.point[p.num] = 1000;
               p.num++;
               assert(p.num <= MAX_HANDTILE);
            }
         } else if (r.m_TileSets[i].type & HT_PAIR) {
            int tile_rest = CTile::RemainingTile(r.m_TileSets[i].first);
            for (j = 0; j < 2; j++) {
               p.tile[p.num] = r.m_TileSets[i].first;
               if (v2_num == 1) {
                  p.point[p.num] = 500;
               } else {
                  p.point[p.num] = 250 + 30 * tile_rest;
               }
               p.num++;
               assert(p.num <= MAX_HANDTILE);
            }
         } else if (r.m_TileSets[i].type & HT_TOCHOW) {
            int tile_rest = 0;

            CTile t = r.m_TileSets[i].first() + 1;
            if (r.m_TileSets[i].type & HT_TOCHOWMIDDLE) {
               t = r.m_TileSets[i].first() + 2;
            }

            if (r.m_TileSets[i].type & HT_TOCHOWLOWER) {
               tile_rest += CTile::RemainingTile(r.m_TileSets[i].first() - 1);
            }
            if (r.m_TileSets[i].type & HT_TOCHOWMIDDLE) {
               tile_rest += CTile::RemainingTile(r.m_TileSets[i].first() + 1);
            }
            if (r.m_TileSets[i].type & HT_TOCHOWUPPER) {
               tile_rest += CTile::RemainingTile(r.m_TileSets[i].first() + 2);
            }

            for (j = 0; j < 2; j++) {
               p.tile[p.num] = (j ? r.m_TileSets[i].first : t);
               p.point[p.num] = 250 + 30 * tile_rest;
               p.num++;
               assert(p.num <= MAX_HANDTILE);
            }
         }
      }

      for (i = 0; i < r.m_iNumTiles; i++) {
         if (r.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
            continue;
         }

         int pnt = 0;

         CTile &t = r.m_Tiles[i].tile;
         CTile prev = t() - 1, prev2 = prev() - 1;
         CTile next = t() + 1, next2 = next() + 1;

         if (t.GetValue() <= 1) {
            prev = prev2 = 0;
         }

         if (t.GetValue() <= 2) {
            prev2 = 0;
         }

         if (t.GetValue() >= 9) {
            next = next2 = 0;
         }

         if (t.GetValue() >= 8) {
            next2 = 0;
         }

         for (j = 0; j < r.m_iNumTileSets; j++) {
            if (r.m_TileSets[j].type & HT_PAIR) {
               if (r.m_TileSets[j].first == prev ||
                  r.m_TileSets[j].first == prev2 ||
                  r.m_TileSets[j].first == next ||
                  r.m_TileSets[j].first == next2) {
                  pnt = 250;
               }
            } else if (r.m_TileSets[j].type & HT_TOCHOW) {
               CTile a1 = r.m_TileSets[j].first;
               CTile a2 = a1() + 1;
               if (r.m_TileSets[j].type & HT_TOCHOWMIDDLE) {
                  a2 = r.m_TileSets[j].first() + 2;
               }
               if (a1 == t || a2 == t || a1 == next ||
                  a1 == next2 || a2 == prev || a2 == prev2) {
                  pnt = 250;
               }
            }
         }
         p.tile[p.num] = t;
         if (pnt > 0) {
            p.point[p.num] = pnt;
         } else {
            pnt = CTile::RemainingTile(t) * 2;
            pnt += CTile::RemainingTile(next);
            pnt += CTile::RemainingTile(prev);
            p.point[p.num] = pnt * 30;
         }
         p.num++;
         assert(p.num <= MAX_HANDTILE);
      }

      for (i = 0; i < p.num; i++) {
         for (j = i; j < p.num; j++) {
            if (p.tile[i] > p.tile[j] ||
               p.tile[i] == p.tile[j] && p.point[i] > p.point[j]) {
               CTile t = p.tile[i];
               p.tile[i] = p.tile[j];
               p.tile[j] = t;

               int t1 = p.point[i];
               p.point[i] = p.point[j];
               p.point[j] = t1;
            }
         }
      }

      if (x == 0) {

         result_pnt = p;
      } else {
         assert(p.num == result_pnt.num);
         for (i = 0; i < p.num; i++) {
            assert(p.tile[i] == result_pnt.tile[i]);
            result_pnt.point[i] += p.point[i];
         }
      }
   }
#ifdef BOT_DEBUG
   fprintf(stdout, "result_pnt.num = %d\n", result_pnt.num);
   for (j = 0; j < result_pnt.num; j++) {
      fprintf(stdout, "result_pnt.tile[%d] = %s, point[%d] = %d\n",
         j, result_pnt.tile[j].GetCode(), j, result_pnt.point[j]);
   }
#endif
   if (result_pnt.num > 0) {
      for (i = 0; i < m_Hand.m_iNumTiles; i++) {
         if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
            continue;
         }
         for (j = 0; j < result_pnt.num; j++) {
            if (m_Hand.m_Tiles[i].tile == result_pnt.tile[j]) {
               break;
            }
         }
         assert(j < result_pnt.num);
         m_BotBrain.tilevalue[i] = result_pnt.point[j] / m_BotBrain.num_handresult;
      }
   }

   bool open = !m_Hand.IsConcealed();

   switch (m_BotBrain.goal) {
      case GOAL_ALLMINOR:
         for (i = 0; i < m_Hand.m_iNumTiles; i++) {
            if (m_Hand.m_Tiles[i].tile.IsMajor()) {
               m_BotBrain.tilevalue[i] += (open ? 0 : 100);
            } else {
               m_BotBrain.tilevalue[i] += (open ? 500 : 300);
            }
         }
         break;

      case GOAL_ALLPUNG:
         {
            int kinds[5][9];
            for (i = 0; i < 5; i++) {
               for (j = 0; j < 9; j++) {
                  kinds[i][j] = 0;
               }
            }
            for (i = 0; i < m_Hand.m_iNumTiles; i++) {
               if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
                  continue;
               }
               CTile &t = m_Hand.m_Tiles[i].tile;
               kinds[t.index1()][t.index2()]++;
            }
            for (i = 0; i < m_Hand.m_iNumTiles; i++) {
               if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
                  continue;
               }
               CTile &t = m_Hand.m_Tiles[i].tile;
               switch (kinds[t.index1()][t.index2()]) {
                  case 1:
                     m_BotBrain.tilevalue[i] += (CTile::RemainingTile(t) > 2) ? 100 : 0;
                     break;

                  case 2:
                     m_BotBrain.tilevalue[i] += CTile::RemainingTile(t) ? 300 : 100;
                     break;

                  case 3:
                  case 4:
                     m_BotBrain.tilevalue[i] += 500;
                     break;
               }
            }
         }
         break;

      case GOAL_ALLCHAR:
         for (i = 0; i < m_Hand.m_iNumTiles; i++) {
            if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
               continue;
            }
            CTile &t = m_Hand.m_Tiles[i].tile;
            int suit = t.GetSuit();
            if (suit & (TILESUIT_CHAR | TILESUIT_DRAGON | TILESUIT_WIND)) {
               m_BotBrain.tilevalue[i] += 500;
            } else {
               m_BotBrain.tilevalue[i] += (open ? 0 : 250);
            }
         }
         break;

      case GOAL_ALLBAMBOO:
         for (i = 0; i < m_Hand.m_iNumTiles; i++) {
            if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
               continue;
            }
            CTile &t = m_Hand.m_Tiles[i].tile;
            int suit = t.GetSuit();
            if (suit & (TILESUIT_BAMBOO | TILESUIT_DRAGON | TILESUIT_WIND)) {
               m_BotBrain.tilevalue[i] += 500;
            } else {
               m_BotBrain.tilevalue[i] += (open ? 0 : 250);
            }
         }
         break;

      case GOAL_ALLCIRCLE:
         for (i = 0; i < m_Hand.m_iNumTiles; i++) {
            if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
               continue;
            }
            CTile &t = m_Hand.m_Tiles[i].tile;
            int suit = t.GetSuit();
            if (suit & (TILESUIT_CIRCLE | TILESUIT_DRAGON | TILESUIT_WIND)) {
               m_BotBrain.tilevalue[i] += 500;
            } else {
               m_BotBrain.tilevalue[i] += (open ? 0 : 250);
            }
         }
         break;

      case GOAL_FANPAI:
         if (!fanpai_fixed) {
            for (i = 0; i < m_Hand.m_iNumTiles; i++) {
               if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
                  continue;
               }
               const CTile &t = m_Hand.m_Tiles[i].tile;
               if (t.GetSuit() == TILESUIT_DRAGON ||
                  t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
                  m_BotBrain.tilevalue[i] += 500;
               } else if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW"))) {
                  m_BotBrain.tilevalue[i] += 500;
               }
            }
         }
         break;
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (rests[i] > m_BotBrain.handresult_rest) {

         if (m_BotBrain.handresult_rest == 1) {
            m_BotBrain.tilevalue[i] += 500;
         } else if (m_BotBrain.handresult_rest == 2) {
            m_BotBrain.tilevalue[i] += 300;
         } else if (m_BotBrain.handresult_rest == 3) {
            m_BotBrain.tilevalue[i] += 100;
         }
      }
   }
}

void CBot::AnalyzeDora()
{
   int i;
   float r[MAX_HANDTILE], r_max = 0;

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }
      r[i] = 0;
      CTile &t = m_Hand.m_Tiles[i].tile;
      if (gpGame->IsDora(t)) {
         r[i] += 8;
      }
      if (t.GetValue() < 9 && gpGame->IsDora(t() + 1)) {
         r[i] += 4;
      }
      if (t.GetValue() > 1 && gpGame->IsDora(t() - 1)) {
         r[i] += 4;
      }
      if (t.GetValue() < 8 && gpGame->IsDora(t() + 2)) {
         r[i] += 2;
      }
      if (t.GetValue() > 2 && gpGame->IsDora(t() - 2)) {
         r[i] += 2;
      }
      r[i] *= m_BotBrain.tilevalue[i];
      if (r_max < r[i]) {
         r_max = r[i];
      }
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }
      float x;
      if (r_max > 0) {
         x = r[i] * 100 / r_max;
      } else {
         x = 0;
      }
      m_BotBrain.tilevalue[i] += x;
   }
}

void CBot::AnalyzeColor()
{
   const float param = 100.f;

   int i;
   float r[MAX_HANDTILE], r_max = param / 3, b = param / 3 / 200, d = param / 3 / 100;
   float ch = b, cir = b, bam = b, wd = 0;

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      r[i] = 0;
      if (m_Hand.m_Tiles[i].flags & (HT_OPEN | HT_TOCHOW)) {
         continue;
      }
      if (m_Hand.m_Tiles[i].tile.GetSuit() == TILESUIT_CHAR) {
         ch += d;
      } else if (m_Hand.m_Tiles[i].tile.GetSuit() == TILESUIT_CIRCLE) {
         cir += d;
      } else if (m_Hand.m_Tiles[i].tile.GetSuit() == TILESUIT_BAMBOO) {
         bam += d;
      } else {

         wd += d;
      }
   }

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type & HT_OPEN) {
         if (m_Hand.m_TileSets[i].first.GetSuit() == TILESUIT_CHAR) {
            cir = bam = b;
         } else if (m_Hand.m_TileSets[i].first.GetSuit() == TILESUIT_CIRCLE) {
            ch = bam = b;
         } else if (m_Hand.m_TileSets[i].first.GetSuit() == TILESUIT_BAMBOO) {
            ch = cir = b;
         }
      }
   }

   if (ch > cir && ch > bam) {
      ch += wd / 2;
      wd += ch / 2;
   } else if (cir > ch && cir > bam) {
      cir += wd / 2;
      wd += cir / 2;
   } else if (bam > ch && bam > cir) {
      bam += wd / 2;
      wd += bam / 2;
   }

   i = (m_Hand.IsConcealed() ? 1 : 2);
   ch *= ch * i;
   cir *= cir * i;
   bam *= bam * i;
   wd *= wd * i;

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_OPEN | HT_TOCHOW)) {
         continue;
      }
      if (m_Hand.m_Tiles[i].tile.GetSuit() == TILESUIT_CHAR) {
         r[i] = ch / 3;
      } else if (m_Hand.m_Tiles[i].tile.GetSuit() == TILESUIT_CIRCLE) {
         r[i] = cir / 3;
      } else if (m_Hand.m_Tiles[i].tile.GetSuit() == TILESUIT_BAMBOO) {
         r[i] = bam / 3;
      } else {
         r[i] = wd / 3;
      }
      if (r[i] > r_max) {
         r_max = r[i];
      }
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (r_max != 0) {
         b = r[i] * param / 3 / r_max;
      } else {
         b = 0;
      }
      m_BotBrain.tilevalue[i] += b;
   }
#ifdef BOT_DEBUG
   fprintf(stdout, "-=-=-=-=-=-=-=-= AnalyzeColor() =-=-=-=-=-=-=-=-\n");
   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_OPEN | HT_TOCHOW)) {
         continue;
      }
      fprintf(stdout, "%s - %f (%f)\n", m_Hand.m_Tiles[i].tile.GetCode(), r[i], r[i] * 700.0 / 3 / r_max);
   }
   fprintf(stdout, "-=-=-=-=-=-=-=-= AnalyzeColor() END =-=-=-=-=-=-\n");
#endif
}

void CBot::AnalyzeZone()
{
   const float param = 100.0f;

   int i, n;
   float r[14], r_max = param, b = param / 200, d = param / 100, zone[7];

   if (m_BotBrain.goal == GOAL_ALLPUNG || m_BotBrain.goal == GOAL_PAIRS) {
      d *= 2;
   }

   for (i = 0; i < 7; i++) {
      zone[i] = b;
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }

      if (m_Hand.m_Tiles[i].tile.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)) {
         continue;
      }

      n = m_Hand.m_Tiles[i].tile.GetValue();

      if (n >= 1 && n <= 3) {
         zone[0] += d;
      }

      if (n >= 2 && n <= 4) {
         zone[1] += d;
      }

      if (n >= 3 && n <= 5) {
         zone[2] += d;
      }

      if (n >= 4 && n <= 6) {
         zone[3] += d;
      }

      if (n >= 5 && n <= 7) {
         zone[4] += d;
      }

      if (n >= 6 && n <= 8) {
         zone[5] += d;
      }

      if (n >= 7) {
         zone[6] += d;
      }
   }

   for (i = 0; i < 7; i++) {
      zone[i] *= zone[i] / 3;
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      r[i] = 0;

      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }

      if (m_Hand.m_Tiles[i].tile.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)) {
         r[i] = zone[0] + zone[6];
         continue;
      }

      n = m_Hand.m_Tiles[i].tile.GetValue();

      switch (n) {
         case 1:
            r[i] = zone[0];
            break;

         case 2:
            r[i] = zone[0] + zone[1];
            break;

         case 3:
            r[i] = zone[0] + zone[1] + zone[2];
            break;

         case 4:
            r[i] = zone[1] + zone[2] + zone[3];
            break;

         case 5:
            r[i] = zone[2] + zone[3] + zone[4];
            break;

         case 6:
            r[i] = zone[3] + zone[4] + zone[5];
            break;

         case 7:
            r[i] = zone[4] + zone[5] + zone[6];
            break;

         case 8:
            r[i] = zone[5] + zone[6];
            break;

         case 9:
            r[i] = zone[6];
            break;

         default:
            r[i] = zone[0] + zone[6];
            break;
      }

      if (r[i] > r_max) {
         r_max = r[i];
      }
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }

      float x = 0;
      if (r_max > 0) {
         x = r[i] * param / r_max;
      }

      m_BotBrain.tilevalue[i] += x;
   }
#ifdef BOT_DEBUG
   fprintf(stdout, "-=-=-=-=-=-=-=-= AnalyzeZone() =-=-=-=-=-=-=-=-\n");
   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_OPEN | HT_TOCHOW)) {
         continue;
      }
      fprintf(stdout, "%s - %f (%f)\n", m_Hand.m_Tiles[i].tile.GetCode(), r[i], r[i] * 300 / r_max);
   }
   fprintf(stdout, "-=-=-=-=-=-=-=-= AnalyzeZone() END =-=-=-=-=-=-\n");
#endif
}

void CBot::AnalyzeNum()
{
   const float param = 100.f;

   int i, n;
   float r[14], r_max = param, b = param / 200, d = param / 100, number[6];

   if (m_BotBrain.goal == GOAL_ALLPUNG || m_BotBrain.goal == GOAL_PAIRS) {
      d *= 2;
   }

   for (i = 0; i < 6; i++) {
      number[i] = b;
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }

      if (m_Hand.m_Tiles[i].tile.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)) {
         continue;
      }

      n = m_Hand.m_Tiles[i].tile.GetValue();

      if (n <= 6) {
         number[n - 1] += d;
      }

      if (n >= 4) {
         number[n - 4] += d;
      }
   }

   for (i = 0; i < 6; i++) {
      number[i] *= number[i];
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      r[i] = 0;

      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }

      if (m_Hand.m_Tiles[i].tile.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)) {
         r[i] = 10;
         continue;
      }

      n = m_Hand.m_Tiles[i].tile.GetValue();

      switch (n) {
         case 1:
            r[i] = number[0];
            break;

         case 2:
            r[i] = number[1];
            break;

         case 3:
            r[i] = number[2];
            break;

         case 4:
            r[i] = number[3] + number[0];
            break;

         case 5:
            r[i] = number[4] + number[1];
            break;

         case 6:
            r[i] = number[5] + number[2];
            break;

         case 7:
            r[i] = number[3];
            break;

         case 8:
            r[i] = number[4];
            break;

         case 9:
            r[i] = number[5];
            break;

         default:
            r[i] = 10;
            break;
      }

      if (r[i] > r_max) {
         r_max = r[i];
      }
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_LOCKED | HT_TOCHOW)) {
         continue;
      }

      float x = 0;
      if (r_max > 0) {
         x = r[i] * param / r_max;
      }

      m_BotBrain.tilevalue[i] += x;
   }
#ifdef BOT_DEBUG
   fprintf(stdout, "-=-=-=-=-=-=-=-= AnalyzeNum() =-=-=-=-=-=-=-=-\n");
   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & (HT_OPEN | HT_TOCHOW)) {
         continue;
      }
      fprintf(stdout, "%s - %f (%f)\n", m_Hand.m_Tiles[i].tile.GetCode(), r[i], r[i] * 100 / r_max);
   }
   fprintf(stdout, "-=-=-=-=-=-=-=-= AnalyzeNum() END =-=-=-=-=-=-\n");
#endif
}

void CBot::AnalyzeDanger()
{
}

