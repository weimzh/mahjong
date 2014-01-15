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

CGame *gpGame = NULL;

CGame::CGame():
m_iCurGirl(-1),
m_iLevelClearFlag(0)
{
   m_pPlayer = new CPlayer;
   m_pBot = new CBot;

   if (m_pPlayer == NULL || m_pBot == NULL) {
      TerminateOnError("CGame::CGame(): Not enough memory!");
   }

   m_pPlayer->m_pOpponent = m_pBot;
   m_pBot->m_pOpponent = m_pPlayer;

   m_iFirstChancePercent = atoi(cfg.Get("GAME", "FirstChancePercent", "20"));
   m_iLastChancePercent = atoi(cfg.Get("GAME", "LastChancePercent", "20"));
   m_iReachBonusPercent = atoi(cfg.Get("GAME", "ReachBonusPercent", "5"));
   m_iBonusGamePercent = atoi(cfg.Get("GAME", "BonusGamePercent", "2"));
   m_iSanGenRushPercent = atoi(cfg.Get("GAME", "SanGenRushPercent", "5"));
   m_iTotalDonDen = atoi(cfg.Get("GAME", "DonDen", "5"));
   if (m_iTotalDonDen > 8) {
      m_iTotalDonDen = 8;
   }
}

CGame::~CGame()
{
   if (m_pPlayer != NULL)
      delete m_pPlayer;

   if (m_pBot != NULL)
      delete m_pBot;
}

void CGame::RunGame()
{
   m_iTotalScore = 0;

   while (1) {
      if (m_iCurGirl == -1) {
         SelectCurGirl();
      }

      GameInit();

      while (1) {
         NewRound();
         PlayRound();

         if (m_pPlayer->m_iScore <= 0) {
            gpGeneral->GameOver();
            m_iTotalScore = 0;
            break;
         } else if (m_pBot->m_iScore <= 0) {
            m_iTotalScore += m_pPlayer->m_iScore;
            StageClear();
            break;
         }
      }
   }
}

void CGame::DonDen()
{
   if (m_iDonDen >= m_iTotalDonDen ||
      m_pBot->m_iNumDiscarded >= 20 ||
      m_pPlayer->m_iNumDiscarded >= 20) {
      return;
   }

   m_iDonDen++;

   int i;
   CTile justgot = 0;

   for (i = 0; i < m_pPlayer->m_Hand.m_iNumTiles; i++) {
      if (m_pPlayer->m_Hand.m_Tiles[i].flags & HT_JUSTGOT) {
         justgot = m_pPlayer->m_Hand.m_Tiles[i].tile;
         m_pPlayer->m_Hand.RemoveTile(i);
         break;
      }
   }

   discarded_t d[20];
   int dis = m_pPlayer->m_iNumDiscarded;
   for (i = 0; i < dis; i++) {
      d[i] = m_pPlayer->m_Discarded[i];
   }

   m_pPlayer->m_iNumDiscarded = m_pBot->m_iNumDiscarded;
   for (i = 0; i < m_pBot->m_iNumDiscarded; i++) {
      m_pPlayer->m_Discarded[i] = m_pBot->m_Discarded[i];
   }
   m_pBot->m_iNumDiscarded = dis;
   for (i = 0; i < dis; i++) {
      m_pBot->m_Discarded[i] = d[i];
   }

   CHand b = m_pPlayer->m_Hand;
   m_pPlayer->m_Hand = m_pBot->m_Hand;
   m_pBot->m_Hand = b;

   if (justgot() != 0) {
      m_pPlayer->m_Hand.AddTile(justgot, HT_JUSTGOT);
   }

   bool r = m_pBot->m_fReach;
   m_pBot->m_fReach = m_pPlayer->m_fReach;
   m_pPlayer->m_fReach = r;

   gpGeneral->EraseArea(0, 0, 640, 200);
   gpGeneral->UpdateScreen(0, 0, 640, 200);
   gpGeneral->EraseArea(0, 265, 640, 480 - 265);
   gpGeneral->UpdateScreen(0, 265, 640, 480 - 265);
   gpGeneral->PlaySound(SND_DISCARD1);
   UTIL_Delay(300);

   gpGeneral->PlaySound(SND_DISCARD2);
   m_pPlayer->DrawHand();
   m_pPlayer->DrawDiscarded();
   m_pBot->DrawHand();
   m_pBot->DrawDiscarded();

   DrawDonDen();
}

void CGame::NewRound()
{
   if (m_pBot->m_iScore < 0 || m_pPlayer->m_iScore < 0) {
      GameInit();
   }

   CTile::NewRound();
   m_fBonusGame = false;
   if (RandomLong(0, 100) < m_iBonusGamePercent) {
      gpGeneral->BonusGame();
      m_fBonusGame = true;
   }

   for (int i = 0; i < 6; i++) {
      m_Dora[i] = CTile::RandomTile();
   }

   m_pPlayer->NewRound();
   m_pBot->NewRound();

   m_fSanGenRush = false;
   if (RandomLong(1, 100) < m_iSanGenRushPercent && !m_fBonusGame) {
      m_fSanGenRush = true;
      ((CPlayer *)m_pPlayer)->SanGenInit();
   }

   if (m_fBonusGame) {
      m_pPlayer->GetBonusHand();
      m_pBot->GetHand();
      gpGeneral->PlayBGMusic(6);
   } else {
      m_pPlayer->GetHand();
      m_pBot->GetHand();
      gpGeneral->PlayBGMusic(RandomLong(0, 1000) & 1);
   }

   gpGeneral->DrawBGFade(m_iCurGirl, 1);

   gpGeneral->UpdateScreen();

   UTIL_Delay(1000);
   DealTiles();

   gpGeneral->PlaySound(SND_DISCARD1);

   m_pPlayer->m_Hand.Sort();
   m_pBot->m_Hand.Sort();

   gpGeneral->EraseArea(0, 0, 640, 18 + TILE_HEIGHT_CONCEALED);
   gpGeneral->EraseArea(0, 380, 640, TILE_HEIGHT_CONCEALED);

   gpGeneral->DrawTiles(NULL, 13, 72, 18, WALL_CONCEALED);
   gpGeneral->DrawTiles(NULL, 13, 20, 396, WALL_CONCEALED);

   gpGeneral->UpdateScreen(0, 0, 640, 18 + TILE_HEIGHT_CONCEALED);
   gpGeneral->UpdateScreen(0, 380, 640, 100);

   UTIL_Delay(1000);

   gpGeneral->PlaySound(SND_DISCARD1);

   m_pPlayer->DrawHand();
   m_pBot->DrawHand();

   UTIL_Delay(1000);

   gpGeneral->DrawBG(m_iCurGirl, 0);
   m_pPlayer->DrawHand();
   m_pBot->DrawHand();

   gpGeneral->DrawTiles(NULL, 7, 415, 215, WALL_CONCEALED, 0);
   gpGeneral->DrawTiles(NULL, 7, 415, 200, WALL_CONCEALED, 0);
   gpGeneral->DrawTile(m_Dora[0], (int)(415 + TILE_WIDTH * 0.7 * 2),
      200, WALL_SHOWN, 0);

   gpGeneral->DrawTurn(m_iRound, 387, 202);

   m_iDonDen = 0;

   DrawScore();
   DrawDonDen();

   gpGeneral->UpdateScreen();

   m_iKongs = 0;

   m_fReachBonus = false;

   if (RandomLong(0, 100) < m_iReachBonusPercent && !m_fBonusGame && !m_fSanGenRush) {
      m_fReachBonus = true;
      gpGeneral->DrawMessage(msg("hint_reachbonus"));
      gpGeneral->WaitForAnyKey();
      gpGeneral->DrawMessage(NULL);
   }
}

void CGame::DealTiles()
{
   gpGeneral->DrawTiles(NULL, 10, 100, 145, WALL_CONCEALED);
   gpGeneral->DrawTiles(NULL, 10, 100, 125, WALL_CONCEALED);

   gpGeneral->DrawTiles(NULL, 10, 100, 250, WALL_CONCEALED);
   gpGeneral->DrawTiles(NULL, 10, 100, 230, WALL_CONCEALED);

   gpGeneral->UpdateScreen(100, 125, TILE_WIDTH * 10, TILE_HEIGHT_WALL + 20);
   gpGeneral->UpdateScreen(100, 230, TILE_WIDTH * 10, TILE_HEIGHT_WALL + 20);
   SDL_Delay(500);

   CBasePlayer *pCurrent = m_pPlayer;
   if (pCurrent->m_iTurn != TURN_EAST) {
      pCurrent = pCurrent->m_pOpponent;
   }

   int i;

   for (i = 0; i < 8; i++) {
      int t = (i / 2 + 1) * 4;
      if (t > 13) {
         t = 13;
      }

      if (i == 0) {
         gpGeneral->EraseArea(100 + TILE_WIDTH * 8, 230, TILE_WIDTH * 2, TILE_HEIGHT_WALL + 20);
      } else if (i == 1) {
         gpGeneral->EraseArea(100 + TILE_WIDTH * 8, 125, TILE_WIDTH * 2, TILE_HEIGHT_WALL + 20);

      } else if (i == 6) {
         gpGeneral->EraseArea(100 + TILE_WIDTH * 7, 125, TILE_WIDTH, TILE_HEIGHT_WALL + 20);

         gpGeneral->DrawTiles(NULL, 1, 100 + TILE_WIDTH * 7, 150, WALL_CONCEALED);
      } else if (i == 7) {
         gpGeneral->EraseArea(100 + TILE_WIDTH * 7, 125, TILE_WIDTH, TILE_HEIGHT_WALL + 20);
      } else {
         gpGeneral->EraseArea(100 + TILE_WIDTH * (8 - i * 2 + 2), 230, TILE_WIDTH * 2, TILE_HEIGHT_WALL + 20);
      }

      gpGeneral->UpdateScreen(100, 125, TILE_WIDTH * 10, TILE_HEIGHT_WALL + 20);
      gpGeneral->UpdateScreen(100, 230, TILE_WIDTH * 10, TILE_HEIGHT_WALL + 20);

      pCurrent->m_Hand.m_iNumTiles = t;
      pCurrent->DrawHand();
      pCurrent = pCurrent->m_pOpponent;
      gpGeneral->PlaySound(SND_BOOM);

      UTIL_Delay(250);
   }

   m_pBot->m_Hand.m_iNumTiles = m_pPlayer->m_Hand.m_iNumTiles = 13;
   UTIL_Delay(750);

   gpGeneral->EraseArea(100, 125, TILE_WIDTH * 7, TILE_HEIGHT_WALL + 20);
   gpGeneral->DrawTiles(NULL, 7, 415, 215, WALL_CONCEALED, 0);
   gpGeneral->DrawTiles(NULL, 7, 415, 200, WALL_CONCEALED, 0);
   gpGeneral->PlaySound(SND_BOOM);
   gpGeneral->UpdateScreen();

   UTIL_Delay(750);
}

void CGame::SelectCurGirl()
{
   gpGeneral->DrawBG(4, 0);

   if (m_iLevelClearFlag == ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3))) {
      m_iLevelClearFlag = 0; // If all level are clear, restart from the beginning
   }

   if (m_iLevelClearFlag & (1 << 0))
      gpGeneral->DrawGirl(20, 20, 280, 200, 0, 0);
   else
      gpGeneral->DrawGirl(20, 20, 280, 200, 0, 1);

   if (m_iLevelClearFlag & (1 << 1))
      gpGeneral->DrawGirl(340, 20, 280, 200, 1, 0);
   else
      gpGeneral->DrawGirl(340, 20, 280, 200, 1, 1);

   if (m_iLevelClearFlag & (1 << 2))
      gpGeneral->DrawGirl(20, 260, 280, 200, 2, 0);
   else
      gpGeneral->DrawGirl(20, 260, 280, 200, 2, 1);

   if (m_iLevelClearFlag & (1 << 3))
      gpGeneral->DrawGirl(340, 260, 280, 200, 3, 0);
   else
      gpGeneral->DrawGirl(340, 260, 280, 200, 3, 1);

   gpGeneral->DrawText("A", 40, 30, 1);
   gpGeneral->DrawText("B", 360, 30, 1);
   gpGeneral->DrawText("C", 40, 270, 1);
   gpGeneral->DrawText("D", 360, 270, 1);

   gpGeneral->DrawUTF8Text(msg("girlselmsg"), 55, 225, 0, 255, 255, 0);

   gpGeneral->UpdateScreen();
   gpGeneral->PlayBGMusic(4);

   while (1) {
      SDLKey key = gpGeneral->ReadKey();
      if (key == SDLK_a && !(m_iLevelClearFlag & (1 << 0))) {
         gpGeneral->PlaySound(SND_DING);
         gpGeneral->DrawGirl(20, 20, 280, 200, 0, 0);
         gpGeneral->DrawText("A", 40, 30, 1, 250, 250, 0);
         gpGeneral->UpdateScreen(20, 20, 280, 200);
         m_iCurGirl = 0;
         UTIL_Delay(800);
         break;
      } else if (key == SDLK_b && !(m_iLevelClearFlag & (1 << 1))) {
         gpGeneral->PlaySound(SND_DING);
         gpGeneral->DrawGirl(340, 20, 280, 200, 1, 0);
         gpGeneral->DrawText("B", 360, 30, 1, 250, 250, 0);
         gpGeneral->UpdateScreen(340, 20, 280, 200);
         m_iCurGirl = 1;
         UTIL_Delay(800);
         break;
      } else if (key == SDLK_c && !(m_iLevelClearFlag & (1 << 2))) {
         gpGeneral->PlaySound(SND_DING);
         gpGeneral->DrawGirl(20, 260, 280, 200, 2, 0);
         gpGeneral->DrawText("C", 40, 270, 1, 250, 250, 0);
         gpGeneral->UpdateScreen(20, 260, 280, 200);
         m_iCurGirl = 2;
         UTIL_Delay(800);
         break;
      } else if (key == SDLK_d && !(m_iLevelClearFlag & (1 << 3))) {
         gpGeneral->PlaySound(SND_DING);
         gpGeneral->DrawGirl(340, 260, 280, 200, 3, 0);
         gpGeneral->DrawText("D", 360, 270, 1, 250, 250, 0);
         gpGeneral->UpdateScreen(340, 260, 280, 200);
         m_iCurGirl = 3;
         UTIL_Delay(800);
         break;
      }
   }

   GameInit();
   gpGeneral->ScreenFade(200);
}

void CGame::GameInit()
{
   int scores[4] = {16000, 24000, 48000, 100000}, i, cleared = 0;

   for (i = 0; i < 4; i++) {
      if (m_iLevelClearFlag & (1 << i)) {
         cleared++;
      }
   }

   m_pPlayer->m_iScore = 1000;
   m_pBot->m_iScore = scores[cleared];

   m_iRound = TURN_EAST;
   m_iNumDeal = 0;
   m_iPendingPoint = 0;

   if (RandomLong(1, 100) > 50) {
      m_pPlayer->m_iTurn = TURN_EAST;
      m_pBot->m_iTurn = TURN_SOUTH;
   } else {
      m_pBot->m_iTurn = TURN_EAST;
      m_pPlayer->m_iTurn = TURN_SOUTH;
   }

   m_fTurned = false;
}

void CGame::PlayRound()
{
   CBasePlayer *pWinner = NULL, *pCurrent;

   if (RandomLong(1, 100) < m_iFirstChancePercent && !m_fSanGenRush && !m_fBonusGame) {
      m_pPlayer->FirstChance();
   }

   // This should be backward
   if (m_pPlayer->m_iTurn == TURN_EAST) {
      pCurrent = m_pBot;
   } else {
      pCurrent = m_pPlayer;
   }

   while (pWinner == NULL) {
      pCurrent = pCurrent->m_pOpponent;
      playeraction act;

      pCurrent->DrawHand();

      if (pCurrent->m_iNumDiscarded >= 20 || !CanDeclareKong()) {
         if (pCurrent->Action(AS_DRAW) == PA_MAHJONG) {
            pWinner = pCurrent;
            break;
         }
         gpGeneral->PlayEndRoundMusic();
         break;
      }

      while (1) {
         act = pCurrent->Action(AS_DRAW);
         if (act == PA_DRAW) {
            gpGeneral->PlaySound(SND_DISCARD1);
            break;
         } else if (act == PA_KONG || act == PA_PUNG || act == PA_CHOW) {
            gpGeneral->PlaySound(SND_SOUND3);
            pCurrent->m_pOpponent->DrawDiscarded();
            if (act == PA_KONG) {
               KongDeclared();
               continue;
            }
            break;
         } else if (act == PA_MAHJONG) {
            pWinner = pCurrent;
            break;
         }
      }

      if (pWinner != NULL) {
         break; // someone won; don't continue the game
      }

      pCurrent->DrawHand();

      while (1) {
         act = pCurrent->Action(AS_DISCARD);
         if (act == PA_DISCARD || act == PA_REACH) {
            gpGeneral->PlaySound(SND_DISCARD2);
            if (act == PA_REACH) {
               if (pCurrent->IsBot()) {
                  gpGeneral->PlayBGMusic(2);
               } else {
                  pCurrent->DrawHand();
                  pCurrent->DrawDiscarded();
                  gpGeneral->PlayBGMusic(-1);
                  gpGeneral->Fire();
                  gpGeneral->PlayBGMusic(3);
               }
               m_iPendingPoint += 1000;
               pCurrent->m_iScore -= 1000;
               DrawScore();
               pCurrent->m_pOpponent->DrawHand();
            }
            break;
         } else if (act == PA_KONG) {
            KongDeclared();
            gpGeneral->PlaySound(SND_SOUND3);

            pCurrent->Action(AS_DRAW);
            gpGeneral->PlaySound(SND_DISCARD1);

            pCurrent->DrawHand();
         } else if (act == PA_MAHJONG) {
            pWinner = pCurrent;
            break;
         }
      }

      pCurrent->DrawHand();
      pCurrent->DrawDiscarded();
   }

   if (pWinner == NULL && RandomLong(1, 100) < m_iLastChancePercent) {
      if (m_pPlayer->LastChance()) {
         pWinner = m_pPlayer;
      }
   }

   if (pWinner == NULL) {
      // This round is a draw
      bool ready[2] = {false, false};

      UTIL_Delay(1000);
      m_pPlayer->DrawHand(true);
      m_pBot->DrawHand(true);
      gpGeneral->PlaySound(SND_DISCARD2);

      ready[0] = m_pPlayer->m_Hand.IsReady();
      ready[1] = m_pBot->m_Hand.IsReady();

      gpGeneral->DrawUTF8Text(msg("round_draw"), 220, 190, 2, 255, 100, 255);
      gpGeneral->UpdateScreen(220, 190, 420, 75);

      UTIL_Delay(1000);
      if (ready[1]) {
         gpGeneral->DrawUTF8Text(msg("ready_hand"), 200, 105, 2, 255, 255, 0);
      } else {
         gpGeneral->DrawUTF8Text(msg("unready_hand"), 200, 105, 2, 0, 255, 255);
      }
      gpGeneral->UpdateScreen(200, 105, 440, 100);

      if (ready[0]) {
         gpGeneral->DrawUTF8Text(msg("ready_hand"), 200, 275, 2, 255, 255, 0);
      } else {
         gpGeneral->DrawUTF8Text(msg("unready_hand"), 200, 275, 2, 0, 255, 255);
      }
      gpGeneral->UpdateScreen(200, 275, 440, 100);

      if (ready[0] != ready[1] && CanDeclareKong()) {
         if (ready[0]) {
            m_pPlayer->m_iScore += 1000;
            m_pBot->m_iScore -= 1000;
         } else {
            m_pPlayer->m_iScore -= 1000;
            m_pBot->m_iScore += 1000;
         }
         UTIL_Delay(1000);
         gpGeneral->PlaySound(SND_SOUND3);
         DrawScore();
      }

      if (m_pPlayer->m_iTurn == TURN_EAST && !ready[0]) {
         m_pPlayer->m_iTurn = TURN_SOUTH;
         m_pBot->m_iTurn = TURN_EAST;
         if (m_fTurned) {
            m_iRound ^= 1;
            m_fTurned = false;
         } else {
            m_fTurned = true;
         }
      } else if (m_pBot->m_iTurn == TURN_EAST && !ready[1]) {
         m_pPlayer->m_iTurn = TURN_EAST;
         m_pBot->m_iTurn = TURN_SOUTH;
         if (m_fTurned) {
            m_iRound ^= 1;
            m_fTurned = false;
         } else {
            m_fTurned = true;
         }
      }

      m_iNumDeal++;
      UTIL_Delay(3000);
   } else {
      // someone won the round
      if (!pWinner->IsBot()) {
         // Play the "win" music if player won
         int i;
         bool selfdrawn = pWinner->m_Result.selfdrawn;
         CTile t = pWinner->m_Hand.m_Tiles[pWinner->m_Hand.m_iNumTiles - 1].tile;

         if (selfdrawn) {
            pWinner->m_Hand.m_iNumTiles--;
            pWinner->DrawHand();
            gpGeneral->AnimSelfDrawn(t);
            pWinner->m_Hand.m_iNumTiles++;
         } else {
            gpGeneral->AnimOut(t);
         }

         gpGeneral->PlayBGMusic(-1);
         UTIL_Delay(500);
         gpGeneral->PlayWinMusic();
         UTIL_Delay(300);
      }
      gpGeneral->PlaySound(SND_DISCARD2);
      pWinner->DrawHand(true);
      if (pWinner->m_fReach) {
         gpGeneral->DrawTile(m_Dora[5], (int)(415 + TILE_WIDTH * 0.7 * 2),
            235, WALL_SHOWN, 0);
         gpGeneral->UpdateScreen((int)(415 + TILE_WIDTH * 0.7 * 2), 235,
            (int)(TILE_WIDTH * 0.7), (int)(TILE_HEIGHT_SHOWN * 0.7));
      }
      UTIL_Delay(3500);

      gpGeneral->DrawBG(m_iCurGirl, 1);
      pWinner->DrawHand(true);
      gpGeneral->UpdateScreen();
      UTIL_Delay(500);
      gpGeneral->PlaySound(SND_SOUND3);
      pWinner->DrawResult();
      UTIL_Delay(500);
      DrawScore(435, 300);

      UTIL_Delay(1500);
      pWinner->m_iScore += pWinner->m_Result.score;
      pWinner->m_pOpponent->m_iScore -= pWinner->m_Result.score;
      gpGeneral->PlaySound(SND_SOUND1);
      DrawScore(435, 300);
      UTIL_Delay(1500);

      if (m_iPendingPoint > 0) {
         pWinner->m_iScore += m_iPendingPoint;
         gpGeneral->PlaySound(SND_SOUND4);
         DrawScore(435, 300);
         m_iPendingPoint = 0;
      }

      UTIL_Delay(4000);

      if (pWinner->m_iTurn != TURN_EAST) {
         pWinner->m_iTurn = TURN_EAST;
         pWinner->m_pOpponent->m_iTurn = TURN_SOUTH;
         m_iNumDeal = 0;
         if (m_fTurned) {
            m_fTurned = false;
            m_iRound ^= 1;
         } else {
            m_fTurned = true;
         }
      } else {
         m_iNumDeal++;
      }
   }

   gpGeneral->ScreenFade();
}

void CGame::StageClear()
{
   m_iLevelClearFlag |= (1 << m_iCurGirl);
   HighScore();
   m_iCurGirl = -1;
}

void CGame::HighScore()
{
   gpGeneral->DrawBGFade(m_iCurGirl, 2);
   gpGeneral->PlayBGMusic(5);
   gpGeneral->WaitForAnyKey();
   // TODO: high score
   gpGeneral->ScreenFade();
}

void CGame::KongDeclared()
{
   m_iKongs++;
   if (m_iKongs > 4)
      m_iKongs = 4;

   gpGeneral->DrawTiles(m_Dora, m_iKongs + 1, (int)(415 + TILE_WIDTH * 0.7 * 2),
      200, WALL_SHOWN, 0);

   int n = (m_iKongs - 1)/ 2 + 1;
   gpGeneral->EraseArea(415, 200, (int)(TILE_WIDTH * 0.7 * n), 60);

   if (m_iKongs % 2) {
      gpGeneral->DrawTile(CTile(0), (int)(415 + (n - 1) * TILE_WIDTH * 0.7), 215, WALL_CONCEALED, 0);
   }

   gpGeneral->UpdateScreen(415, 200, 225, 60);
}

void CGame::DrawScore(int x, int y)
{
   gpGeneral->EraseArea(x, y, 155, 50);

   gpGeneral->DrawUTF8Text(msg("computer"), x, y, 0, 255, 255, 0);
   gpGeneral->DrawUTF8Text(va("%10d", m_pBot->m_iScore), x + 20, y);
   y += 24;

   gpGeneral->DrawUTF8Text(msg("player"), x, y, 0, 255, 255, 0);
   gpGeneral->DrawUTF8Text(va("%10d", m_pPlayer->m_iScore), x + 20, y);

   gpGeneral->UpdateScreen(x, y - 24, 155, 50);
}

void CGame::DrawDonDen()
{
   gpGeneral->EraseArea(450, 300, 160, 50);

   if (m_iTotalDonDen - m_iDonDen <= 0) {
      gpGeneral->UpdateScreen(470, 270, 160, 50);
      return;
   }

   gpGeneral->DrawText("DON", 470, 270, 0, 255, 255, 0);
   gpGeneral->DrawText("DEN", 480, 290, 0, 255, 255, 0);

   gpGeneral->DrawText(va("%d", m_iTotalDonDen - m_iDonDen), 537, 275,
      1, 255, 255, 255);
   gpGeneral->DrawUTF8Text("TIMES", 560, 290, 0, 255, 255, 0);
   gpGeneral->UpdateScreen(470, 270, 160, 50);
}

int CGame::IsDora(const CTile &t, bool reach)
{
   int i, d = 0;
   for (i = 0; i < m_iKongs + 1; i++) {
      if (m_Dora[i].Dora() == t) {
         d++;
      }
   }

   if (reach && t == m_Dora[5].Dora()) {
      d++;
   }

   return d;
}

