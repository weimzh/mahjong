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

CBasePlayer::CBasePlayer():
m_iNumDiscarded(0),
m_iScore(0),
m_iState(0)
{
   memset(&m_Result, 0, sizeof(m_Result));

   m_fTuiDaoHu = (atoi(cfg.Get("GAME", "TuiDaoHu", "0")) > 0);
}

CBasePlayer::~CBasePlayer()
{
}

// Return true if player has discarded the specified tile.
bool CBasePlayer::HasDiscarded(const CTile &t)
{
   for (int i = 0; i < m_iNumDiscarded; i++) {
      if (m_Discarded[i].tile == t) {
         return true;
      }
   }
   return false;
}

void CBasePlayer::NewRound()
{
   m_iNumDiscarded = 0;
   m_Hand.NewHand();
   m_fReach = false;
   m_iState = PS_NOTDISCARDED;

   memset(&m_Result, 0, sizeof(m_Result));
}

void CBasePlayer::GetHand()
{
   m_Hand.NewHand();
   for (int i = 0; i < 13; i++) {
      m_Hand.AddRandomTile(0);
   }
}

void CBasePlayer::GetBonusHand()
{
   int count = 0, i, num = RandomLong(9, 12);
   CTile t[13], tmp;

   CTile maj[13] = {"1C", "9C", "1B", "9B", "1D", "9D", "EW", "SW",
      "WW", "NW", "WD", "GD", "RD"};
   CTile wind[4] = {"EW", "SW", "WW", "NW"};
   CTile elem[3] = {"RD", "GD", "WD"};

   i = RandomLong(1, 7);

   switch (i) {
      case 1: // one suit
         {
            t[0] = CTile::RandomTile();
            count = 1;
            int suit = t[0].GetSuit();
            for (i = 0; i < num; i++) {
               int v = RandomLong(1, 9);
               tmp = CTile(suit | v);
               if (CTile::FetchTile(tmp)) {
                  t[count++] = tmp;
               }
            }
            break;
         }

      case 2: // all majors
         while (count < num) {
            tmp = maj[RandomLong(0, 13 - 1)];
            for (int j = 0, c = RandomLong(1, 100) > 75 ? 2 : 3; j < c; j++) {
               int a = 0;
               if (CTile::RemainingTile(tmp) > 1) {
                  CTile::FetchTile(tmp);
                  t[count++] = tmp;
                  a++;
               }
               if (count >= num) {
                  break;
               }
            }
         }
         break;

      case 3: // all num majors
         while (count < num) {
            tmp = maj[RandomLong(0, 6 - 1)];
            for (int j = 0, c = RandomLong(1, 100) > 75 ? 2 : 3; j < c; j++) {
               int a = 0;
               if (CTile::RemainingTile(tmp) > 1) {
                  CTile::FetchTile(tmp);
                  t[count++] = tmp;
                  a++;
               }
               if (count >= num) {
                  break;
               }
            }
         }
         break;

      case 4: // thirteen wonders
         for (i = 0; i < num; i++) {
            if (RandomLong(0, 100) > 90) {
               continue;
            }
            if (CTile::FetchTile(maj[i])) {
               t[count++] = maj[i];
            }
         }
         break;

      case 5: // four winds
         for (i = 0; i < 12; i++) {
            if (RandomLong(0, 100) > 90) {
               continue;
            }
            if (CTile::FetchTile(wind[i / 3])) {
               t[count++] = wind[i / 3];
            }
         }
         break;

      case 6: // three elements
         for (i = 0; i < 9; i++) {
            if (RandomLong(0, 100) > 80) {
               continue;
            }
            if (CTile::FetchTile(elem[i / 3])) {
               t[count++] = elem[i / 3];
            }
         }
         break;

      case 7: // nine lanteens
         num = (TILESUIT_CHAR << RandomLong(0, 2));
         for (i = 0; i < 13; i++) {
            if (RandomLong(0, 100) > 90) {
               continue;
            }
            int v = ((i < 3) ? 1 : ((i > 9) ? 9 : i - 1));
            tmp = CTile(num | v);
            if (CTile::FetchTile(tmp)) {
               t[count++] = tmp;
            }
         }
         break;
   }

   // fill the remaining places with random tiles
   for (i = count; i < 13; i++) {
      t[i] = CTile::RandomTile();
   }

   // shuffle the tiles
   for (i = 0; i < 13; i++) {
      int b = RandomLong(0, 13 - 1);
      tmp = t[i];
      t[i] = t[b];
      t[b] = tmp;
   }

   // add tiles to hand
   for (i = 0; i < 13; i++) {
      m_Hand.AddTile(t[i], 0);
   }
}

bool CBasePlayer::HasYakuman()
{
   return (m_Result.tenhou || m_Result.renhou || m_Result.chihou ||
      m_Result.tuisou || m_Result.chinraotou || m_Result.ryuisou ||
      m_Result.daisangen || m_Result.daisusi || m_Result.syosusi ||
      m_Result.suankou || m_Result.surenkou || m_Result.kokusi ||
      m_Result.cyurenpotou || m_Result.sukantu);
}

int CBasePlayer::HasFan()
{
   if (HasYakuman())
      return 13;

   int fan = 0;
   bool concealed = m_Hand.IsConcealed();

   fan += m_Result.reach;
   fan += m_Result.pinfu;
   fan += m_Result.ippatu;
   fan += m_Result.double_reach;
   fan += m_Result.tanyao;
   fan += m_Result.tumo;
   fan += m_Result.ryanpeikou * 3;
   fan += m_Result.iipeikou;
   fan += m_Result.sansiki * (concealed ? 2 : 1);
   fan += m_Result.sansiki_doukoku * 2;
   fan += m_Result.toitoi * 2;
   fan += m_Result.sanankou * 2;
   fan += m_Result.sanrenkou * 2;
   fan += m_Result.junchan * (concealed ? 3 : 2);
   fan += m_Result.chanta * (concealed ? 2 : 1);
   fan += m_Result.ittu * (concealed ? 2 : 1);
   fan += m_Result.chinitu * (concealed ? 6 : 5);
   fan += m_Result.honitu * (concealed ? 3 : 2);
   fan += m_Result.chitoitu * 2;

   fan += m_Result.honraotou * 2;
   fan += m_Result.syosangen * 2;
   fan += m_Result.haitei;
   fan += m_Result.houtei;
   fan += m_Result.rinshan;
   fan += m_Result.fanpai;
   fan += m_Result.dora;
   fan += m_Result.sankantu * 2;

   fan += m_Result.nofan;

   return fan;
}

void CBasePlayer::CalcResult()
{
   memset(&m_Result, 0, sizeof(m_Result));

   m_Result.concealed = m_Hand.IsConcealed();
   CalcFu();

   CalcResult_tenhou();
   CalcResult_renhou();
   CalcResult_tuisou();
   CalcResult_chinraotou();
   CalcResult_ryuisou();
   CalcResult_daisangen();
   CalcResult_daisusi();
   CalcResult_suankou();
   CalcResult_surenkou();
   CalcResult_kokusi();
   CalcResult_cyurenpotou();
   CalcResult_sukantu();

   CalcResult_chitoitu();

   if (!HasYakuman()) {
      CalcResult_reach();
      CalcResult_ippatu();
      CalcResult_double_reach();
      CalcResult_tanyao();
      CalcResult_ryanpeikou();
      CalcResult_sansiki();
      CalcResult_sansiki_doukoku();
      CalcResult_toitoi();
      CalcResult_honraotou();
      CalcResult_junchan();
      CalcResult_ittu();
      CalcResult_chinitu();
      CalcResult_honitu();
      CalcResult_haitei();
      CalcResult_rinshan();
      CalcResult_fanpai();

      CalcResult_dora();
   }

   if (atoi(cfg.Get("GAME", "TuiDaoHu", "0")) > 1 && !HasFan()) {
      m_Result.nofan = 1;
   }

   CalcScore();
}

void CBasePlayer::CalcResult_tenhou()
{
   if ((m_iState & PS_NOTDISCARDED) && m_iTurn == TURN_EAST) {
      m_Result.tenhou = 1;
   }
}

void CBasePlayer::CalcResult_renhou()
{
   if ((m_iState & PS_NOTDISCARDED) && m_iTurn != TURN_EAST) {
      if (m_Result.selfdrawn) {
         m_Result.chihou = 1;
         return;
      }
      m_Result.renhou = 1;
   }
}

void CBasePlayer::CalcResult_tuisou()
{
   for (int i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (!(m_Hand.m_Tiles[i].tile.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)))
         return; // fail!
   }
   m_Result.tuisou = 1; // success!
}

void CBasePlayer::CalcResult_chinraotou()
{
   for (int i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (!m_Hand.m_Tiles[i].tile.IsNumMajor())
         return; // fail!
   }
   m_Result.chinraotou = 1; // success!
}

void CBasePlayer::CalcResult_ryuisou()
{
   for (int i = 0; i < m_Hand.m_iNumTiles; i++) {
      const CTile &t = m_Hand.m_Tiles[i].tile;
      if (t == CTile("GD"))
         continue;
      if (t.GetSuit() != TILESUIT_BAMBOO)
         return; // fail!
      if (t.GetValue() != 2 || t.GetValue() != 3 ||
         t.GetValue() != 4 || t.GetValue() != 6 ||
         t.GetValue() != 8)
         return; // fail!
   }

   m_Result.ryuisou = 1; // success!
}

void CBasePlayer::CalcResult_daisangen()
{
   int a[3] = {0, 0, 0}, i;
   bool pair = false;

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].first.GetSuit() == TILESUIT_DRAGON) {
         if (m_Hand.m_TileSets[i].type & HT_PAIR) {
            if (pair)
               return; // fail; two pairs is not possible
            pair = true;
         }
         assert(m_Hand.m_TileSets[i].first.GetValue() - 1 <= 2);
         a[m_Hand.m_TileSets[i].first.GetValue() - 1]++;
      }
   }

   if (a[0] && a[1] && a[2]) {
      if (!pair)
         m_Result.daisangen = 1;
      else
         m_Result.syosangen = 1;
   }
}

void CBasePlayer::CalcResult_daisusi()
{
   int a[4] = {0, 0, 0, 0}, i;
   bool pair = false;

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].first.GetSuit() == TILESUIT_WIND) {
         if (m_Hand.m_TileSets[i].type & HT_PAIR) {
            if (pair)
               return; // fail; two pairs are not possible
            pair = true;
         }
         assert(m_Hand.m_TileSets[i].first.GetValue() - 1 <= 3);
         a[m_Hand.m_TileSets[i].first.GetValue() - 1]++;
      }
   }

   if (a[0] && a[1] && a[2] && a[3]) {
      if (!pair) {
         m_Result.daisusi = 1;
      } else {
         m_Result.syosusi = 1;
      }
   }
}

void CBasePlayer::CalcResult_suankou()
{
   int count = 0, i;
   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type & (HT_CLOSEDKONG | HT_CLOSEDPUNG))
         count++;
   }

   if (count >= 4)
      m_Result.suankou = 1;
   else if (count >= 3)
      m_Result.sanankou = 1;
}

void CBasePlayer::CalcResult_surenkou()
{
   int i, j;
   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type &
         (HT_CLOSEDKONG | HT_CLOSEDPUNG | HT_OPENKONG | HT_OPENPUNG))
      {
         int a[3] = {0, 0, 0};
         const CTile &t1 = m_Hand.m_TileSets[i].first;
         if (t1.GetValue() >= 7 || (t1.GetSuit() & (TILESUIT_WIND | TILESUIT_DRAGON)))
            continue;
         for (j = 0; j < m_Hand.m_iNumTileSets; j++) {
            if (m_Hand.m_TileSets[j].type &
               (HT_CLOSEDKONG | HT_CLOSEDPUNG | HT_OPENKONG |
               HT_OPENPUNG))
            {
               const CTile &t2 = m_Hand.m_TileSets[j].first;
               if (t1.GetSuit() != t2.GetSuit())
                  continue;
               int d = t2.GetValue() - t1.GetValue();
               if (d < 1 || d > 3)
                  continue;
               a[d - 1]++;
            }
         }
         if (a[0] && a[1] && a[2]) {
            m_Result.surenkou = 1; // success!
            return;
         } else if (a[0] && a[1]) {
            m_Result.sanrenkou = 1;
            return;
         }
      }
   }
}

void CBasePlayer::CalcResult_kokusi()
{
   if (m_Hand.m_Tiles[0].flags & HT_THIRTEENWONDERS) {
      m_Result.kokusi = 1;
      for (int i = 0; i < m_Hand.m_iNumTiles; i++) {
         if (m_Hand.m_Tiles[i].flags & HT_JUSTGOT) {
            m_Result.selfdrawn = true;
         }
      }
   }
}

void CBasePlayer::CalcResult_cyurenpotou()
{
   if (!m_Result.concealed) {
      return; // fail!
   }
   int count[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}, i;
   int suit = m_Hand.m_Tiles[0].tile.GetSuit();
   bool b = false;
   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].tile.GetSuit() != suit) {
         return; // fail
      }
      assert(m_Hand.m_Tiles[i].tile.GetValue() >= 1);
      assert(m_Hand.m_Tiles[i].tile.GetValue() <= 9);
      count[m_Hand.m_Tiles[i].tile.GetValue() - 1]++;
   }
   for (i = 0; i < 9; i++) {

      int s = ((i == 0 || i == 8) ? 3 : 1);
      if (count[i] == s + 1) {
         if (b) {
            return; // fail
         }
         b = true;
      } else if (count[i] != s) {
         return; // fail
      }
   }
   m_Result.cyurenpotou = 1; // success
}

void CBasePlayer::CalcResult_sukantu()
{
   int count = 0, i;
   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type &
         (HT_OPENKONG | HT_CLOSEDKONG))
         count++;
   }
   if (count >= 4)
      m_Result.sukantu = 1; // success!
   else if (count >= 3)
      m_Result.sankantu = 1;
}

void CBasePlayer::CalcResult_reach()
{
   if (m_fReach)
      m_Result.reach = 1;
}

void CBasePlayer::CalcResult_ippatu()
{
   if ((m_iState & PS_CALLREACH) &&
      m_iNumDiscarded > 0 &&
      (m_Discarded[m_iNumDiscarded - 1].flags & DT_REACH))
      m_Result.ippatu = 1;
}

void CBasePlayer::CalcResult_double_reach()
{
   if (m_fReach && m_iNumDiscarded > 0 &&
      (m_Discarded[0].flags & DT_REACH))
      m_Result.double_reach = 1;
}

void CBasePlayer::CalcResult_tanyao()
{
   for (int i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].tile.IsMajor())
         return; // fail!
   }

   m_Result.tanyao = 1;
}

void CBasePlayer::CalcResult_ryanpeikou()
{
   if (!m_Result.concealed) {
      return; // fail
   }
   int i, j, count = 0;
   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type & (HT_CLOSEDCHOW | HT_OPENCHOW)) {
         for (j = i + 1; j < m_Hand.m_iNumTileSets; j++) {
            if (m_Hand.m_TileSets[j].type & (HT_CLOSEDCHOW | HT_OPENCHOW)) {
               if (m_Hand.m_TileSets[j].first == m_Hand.m_TileSets[i].first) {
                  count++;
               }
            }
         }
      }
   }

   if (count >= 2) {
      m_Result.ryanpeikou = 1;
   } else if (count >= 1) {
      m_Result.iipeikou = 1;
   }
}

void CBasePlayer::CalcResult_sansiki()
{
   int i, j;
   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (!(m_Hand.m_TileSets[i].type & (HT_OPENCHOW | HT_CLOSEDCHOW))) {
         continue;
      }
      CTile t = m_Hand.m_TileSets[i].first;
      if (t.GetSuit() != TILESUIT_CHAR) {
         continue;
      }
      bool b[2] = {false, false};
      for (j = 0; j < m_Hand.m_iNumTileSets; j++) {
         if (!(m_Hand.m_TileSets[j].type & (HT_OPENCHOW | HT_CLOSEDCHOW))) {
            continue;
         }
         CTile s = m_Hand.m_TileSets[j].first;
         if (s.GetValue() == t.GetValue()) {
            if (s.GetSuit() == TILESUIT_BAMBOO) {
               b[0] = true;
            } else if (s.GetSuit() == TILESUIT_CIRCLE) {
               b[1] = true;
            }
         }
      }
      if (b[0] && b[1]) {

         m_Result.sansiki = 1;
         return; // success!
      }
   }
}

void CBasePlayer::CalcResult_sansiki_doukoku()
{
   int i, j;
   const int flags = (HT_OPENPUNG | HT_CLOSEDPUNG | HT_OPENKONG | HT_CLOSEDKONG);
   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (!(m_Hand.m_TileSets[i].type & flags)) {
         continue;
      }
      CTile t = m_Hand.m_TileSets[i].first;
      if (t.GetSuit() != TILESUIT_CHAR) {
         continue;
      }
      bool b[2] = {false, false};

      for (j = 0; j < m_Hand.m_iNumTileSets; j++) {
         if (!(m_Hand.m_TileSets[j].type & flags)) {
            continue;
         }
         CTile s = m_Hand.m_TileSets[j].first;
         if (s.GetValue() == t.GetValue()) {
            if (s.GetSuit() == TILESUIT_BAMBOO) {
               b[0] = true;
            } else if (s.GetSuit() == TILESUIT_CIRCLE) {
               b[1] = true;
            }
         }
      }
      if (b[0] && b[1]) {
         m_Result.sansiki_doukoku = 1;
         return; // success!
      }
   }
}

void CBasePlayer::CalcResult_toitoi()
{
   if (m_Result.chitoitu) {
      return;
   }
   for (int i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type & (HT_OPENCHOW | HT_CLOSEDCHOW))
         return; // found a chew, fail!
   }
   m_Result.toitoi = 1; // all are pungs, success!
}

void CBasePlayer::CalcResult_junchan()
{
   if (m_Result.honraotou) {
      return;
   }

   bool mixed = false;

   if (m_Hand.m_iNumTileSets <= 0) {
      return; // fail
   }

   for (int i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type & (HT_OPENCHOW | HT_CLOSEDCHOW)) {
         int v = m_Hand.m_TileSets[i].first.GetValue();
         if (v != 1 && v != 7) {
            return; // fail
         }
      } else {
         const CTile &t = m_Hand.m_TileSets[i].first;
         if (!t.IsMajor()) {
            return; // fail
         }
         if (!t.IsNumMajor()) {
            mixed = true;
         }
      }
   }
   if (mixed) {
      m_Result.chanta = 1;
   } else {
      m_Result.junchan = 1;
   }
}

void CBasePlayer::CalcResult_ittu()
{
   for (int i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type & (HT_OPENCHOW | HT_CLOSEDCHOW)) {
         const CTile &t = m_Hand.m_TileSets[i].first;
         if (t.GetValue() == 1) {
            bool h[2] = {false, false};
            for (int j = 0; j < m_Hand.m_iNumTileSets; j++) {
               if (m_Hand.m_TileSets[j].type & (HT_OPENCHOW | HT_CLOSEDCHOW)) {
                  const CTile &s = m_Hand.m_TileSets[j].first;
                  if (s.GetSuit() != t.GetSuit()) {
                     continue;
                  } else if (s.GetValue() == 4) {
                     h[0] = true;
                  } else if (s.GetValue() == 7) {
                     h[1] = true;
                  }
               }
            }
            if (h[0] && h[1]) {
               m_Result.ittu = 1;
               return; // success!
            }
         }
      }
   }
}

void CBasePlayer::CalcResult_chinitu()
{
   int suit = m_Hand.m_Tiles[0].tile.GetSuit();
   for (int i = 1; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].tile.GetSuit() != suit)
         return; // fail!
   }
   m_Result.chinitu = 1; // success!

}

void CBasePlayer::CalcResult_honitu()
{
   if (m_Result.chinitu)
      return;

   int i, suit = -1;

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (!(m_Hand.m_Tiles[i].tile.GetSuit() &
         (TILESUIT_DRAGON | TILESUIT_WIND)))
      {
         suit = m_Hand.m_Tiles[i].tile.GetSuit();
         break;
      }
   }

   if (suit == -1)
      return; // fail!

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].tile.GetSuit() != suit &&
         !(m_Hand.m_Tiles[i].tile.GetSuit() &
         (TILESUIT_DRAGON | TILESUIT_WIND)))
      {
         return; // fail!
      }
   }

   m_Result.honitu = 1; // success!
}

void CBasePlayer::CalcResult_chitoitu()
{
   int i;

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (!(m_Hand.m_Tiles[i].flags & HT_PAIR)) {
         return; // fail!
      }
   }

   m_Result.chitoitu = 1; // success!
   m_Result.fu = 25;

   m_Result.selfdrawn = false;
   m_Result.tumo = 0;
   m_Result.pinfu = 0;
   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & HT_JUSTGOT) {
         m_Result.tumo = 1;
         m_Result.selfdrawn = true;
      }
   }
}

void CBasePlayer::CalcResult_honraotou()
{
   for (int i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (!m_Hand.m_Tiles[i].tile.IsMajor())
         return; // fail!
   }
   m_Result.honraotou = 1; // success!

}

void CBasePlayer::CalcResult_haitei()
{
   int i;

   if (m_pOpponent->m_iNumDiscarded >= 20 || m_iNumDiscarded >= 20) {
      for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
         if (m_Hand.m_TileSets[i].type & HT_TUMO) {
            m_Result.haitei = 1;
            break;
         } else if ((m_Hand.m_TileSets[i].type & HT_RON) && m_iNumDiscarded >= 20) {
            m_Result.houtei = 1;
            break;
         }
      }

      if (m_Hand.m_iNumTileSets <= 0) {
         for (i = 0; i < m_Hand.m_iNumTiles; i++) {
            if (m_Hand.m_Tiles[i].flags & HT_JUSTGOT) {
               m_Result.haitei = 1;
               break;
            } else if ((m_Hand.m_Tiles[i].flags & HT_FROMOPPONENT) && m_iNumDiscarded >= 20) {
               m_Result.houtei = 1;
               break;
            }
         }
      }
   }
}

void CBasePlayer::CalcResult_rinshan()
{
   if (m_iState & PS_KONGED) {
      m_Result.rinshan = 1;
   }
}

void CBasePlayer::CalcResult_fanpai()
{
   int count = 0, i;
   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (!(m_Hand.m_TileSets[i].type &
         (HT_OPENPUNG | HT_CLOSEDPUNG | HT_OPENKONG | HT_CLOSEDKONG)))
         continue;

      const CTile &t = m_Hand.m_TileSets[i].first;
      if (t.GetSuit() == TILESUIT_DRAGON ||
         t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW")))
         count++;

      if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW")))
         count++;
   }
   m_Result.fanpai = count;
}

void CBasePlayer::CalcResult_dora()
{
   for (int i = 0; i < m_Hand.m_iNumTiles; i++) {
      m_Result.dora += gpGame->IsDora(m_Hand.m_Tiles[i].tile, m_fReach);
   }
}

void CBasePlayer::CalcFu()
{
   if (m_Result.chitoitu) {
      m_Result.fu = 25;
      return;
   }

   int i, fu = 20, n, ron = 0;

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if (m_Hand.m_TileSets[i].type & HT_OPENKONG) {
         n = 8;
         if (m_Hand.m_TileSets[i].first.IsMajor())
            n *= 2;
         fu += n;
      } else if (m_Hand.m_TileSets[i].type & HT_CLOSEDKONG) {
         n = 16;
         if (m_Hand.m_TileSets[i].first.IsMajor())
            n *= 2;
         fu += n;
      } else if (m_Hand.m_TileSets[i].type & HT_OPENPUNG) {
         n = 2;
         if (m_Hand.m_TileSets[i].first.IsMajor())
            n *= 2;
         fu += n;
      } else if (m_Hand.m_TileSets[i].type & HT_CLOSEDPUNG) {
         n = 4;
         if (m_Hand.m_TileSets[i].first.IsMajor())
            n *= 2;
         fu += n;
      } else if (m_Hand.m_TileSets[i].type & HT_PAIR) {
         const CTile &t = m_Hand.m_TileSets[i].first;
         if (t.GetSuit() == TILESUIT_DRAGON ||
            t == ((gpGame->GetRound() == TURN_EAST) ? CTile("EW") : CTile("SW")))
         {
            fu += 2;
         }
         if (t == ((m_iTurn == TURN_EAST) ? CTile("EW") : CTile("SW")))
         {
            fu += 2;
         }
      }

      if (m_Hand.m_TileSets[i].type & HT_RON) {
         ron = 1;
         handtile_t *p[3] = {NULL, NULL, NULL}, *pt;
         int count = 0, j;
         for (j = 0; j < m_Hand.m_iNumTiles; j++) {
            if (m_Hand.m_Tiles[j].tileset == i) {
               assert(count < 3);
               p[count++] = &m_Hand.m_Tiles[j];
            }
         }
         assert(count >= 2);
         if (count == 2) {
            fu += 2;
         } else if (m_Hand.m_TileSets[i].type & HT_OPENCHOW) {
            for (j = 0; j < count; j++) {
               if (p[j]->flags & HT_FROMOPPONENT) {
                  pt = p[0];
                  p[0] = p[j];
                  p[j] = pt;
                  break;
               }
            }

            if (p[1]->tile > p[2]->tile) {
               pt = p[1];
               p[1] = p[2];
               p[2] = pt;
            }

            if (p[0]->tile() == p[1]->tile() + 1 &&
               p[0]->tile() == p[2]->tile() - 1)
            {
               fu += 2;
            }
            else {
               if ((p[0]->tile.GetValue() == 3 && p[1]->tile.GetValue() == 1) ||
                  (p[0]->tile.GetValue() == 7 && p[1]->tile.GetValue() == 8))
               {
                  fu += 2;
               }
            }
         }
      } else if (m_Hand.m_TileSets[i].type & HT_TUMO) {
         if (m_Result.concealed) {
            m_Result.tumo = 1;
         }

         handtile_t *p[3] = {NULL, NULL, NULL}, *pt;
         int count = 0, j;
         for (j = 0; j < m_Hand.m_iNumTiles; j++) {
            if (m_Hand.m_Tiles[j].tileset == i) {
               assert(count < 3);
               p[count++] = &m_Hand.m_Tiles[j];
            }
         }

         assert(count >= 2);
         if (count == 2) {
            fu += 2;
         } else if (m_Hand.m_TileSets[i].type & HT_CLOSEDCHOW) {
            for (j = 0; j < count; j++) {
               if (p[j]->flags & HT_JUSTGOT) {
                  pt = p[0];
                  p[0] = p[j];
                  p[j] = pt;
                  break;
               }
            }

            if (p[1]->tile > p[2]->tile) {
               pt = p[1];
               p[1] = p[2];
               p[2] = pt;
            }

            if (p[0]->tile() == p[1]->tile() + 1 &&
               p[0]->tile() == p[2]->tile() - 1)
            {
               fu += 2;
            }
            else {
               if ((p[0]->tile.GetValue() == 3 && p[1]->tile.GetValue() == 1) ||
                  (p[0]->tile.GetValue() == 7 && p[1]->tile.GetValue() == 8))
               {
                  fu += 2;
               }
            }
         }
      }
   }

   m_Result.selfdrawn = false;
   if (fu == 20) {
      if (m_Result.concealed) {
         m_Result.pinfu = 1;
         m_Result.fu = (ron ? 30 : 20);
         if (!ron) {
            m_Result.selfdrawn = true;
         }
         return;
      }
   }

   if (!ron) {
      fu += 2;
      m_Result.selfdrawn = true;
   } else if (m_Result.concealed) {
      fu += 10;
   }

   if (fu <= 20 && HasFan() <= 1) {
      fu = 30;
   }

   m_Result.fu = (fu + 9) / 10 * 10;
}

void CBasePlayer::CalcScore()
{
   int fan = HasFan();

   if (HasYakuman() || fan >= 13) {
      m_Result.score = 8000;
      m_Result.level = SL_YAKUMAN;
   } else if (fan == 5) {
      m_Result.score = 2000;
      m_Result.level = SL_MANGAN;
   } else if (fan >= 6 && fan <= 7) {
      m_Result.score = 3000;
      m_Result.level = SL_HANEMAN;
   } else if (fan >= 8 && fan <= 10) {
      m_Result.score = 4000;
      m_Result.level = SL_BAIMAN;
   } else if (fan >= 11) {
      m_Result.score = 6000;
      m_Result.level = SL_SANBAIMAN;
   } else {
      int n = 4, i;
      for (i = 0; i < fan; i++) {
         n *= 2;
      }

      m_Result.score = m_Result.fu * n;

      if (m_Result.score >= 2000) {
         m_Result.score = 2000;
         m_Result.level = SL_MANGAN;
      } else {
         switch (fan) {
            case 0:
            case 1:
               m_Result.level = SL_ONEHAN;
               break;

            case 2:
               m_Result.level = SL_TWOHAN;
               break;

            case 3:
               m_Result.level = SL_THREEHAN;
               break;

            case 4:

               m_Result.level = SL_FOURHAN;
               break;

            default:
               assert(false);
               break;
         }
      }
   }

   if (m_iTurn == TURN_EAST) {
      m_Result.score *= 6;
   } else {
      m_Result.score *= 4;
   }

   m_Result.score += gpGame->GetNumDeal() * 100;

   // round it up to the nearest hundred
   m_Result.score = (m_Result.score + 99) / 100 * 100;
}

bool CBasePlayer::ActionIsValid(enum actionstate state, playeraction act)
{
   int i;
   switch (state) {
      case AS_DRAW:
         switch (act) {
            case PA_DRAW:
               return true;

            case PA_DISCARD:
               return false;

            case PA_PUNG:
               if (m_fReach || (m_iState & PS_ASKED) || m_iNumDiscarded >= 20)
                  return false;
               return m_Hand.CanPung(m_pOpponent->LastDiscardedTile());

            case PA_CHOW:
               if (m_fReach || (m_iState & PS_ASKED) || m_iNumDiscarded >= 20)
                  return false;
               return (m_Hand.CanChow(m_pOpponent->LastDiscardedTile(), CHOW_UPPER) ||
                  m_Hand.CanChow(m_pOpponent->LastDiscardedTile(), CHOW_MIDDLE) ||
                  m_Hand.CanChow(m_pOpponent->LastDiscardedTile(), CHOW_LOWER));

            case PA_KONG:
               if (m_fReach || (m_iState & PS_ASKED) || m_iNumDiscarded >= 20)
                  return false;
               return m_Hand.CanKong(m_pOpponent->LastDiscardedTile(), true);

            case PA_REACH:
               return false;

            case PA_MAHJONG:
               {
                  CTile waiting[13];
                  int i, j, num;
                  num = m_Hand.GetWaitingTile(waiting, 13);

                  if (num <= 0) {
                     return false; // This is not a ready hand at all
                  }

                  if (!m_fTuiDaoHu) {
                     if (num < 13) {
                        for (i = 0; i < m_iNumDiscarded; i++) {
                           for (j = 0; j < num; j++) {
                              if (m_Discarded[i].tile == waiting[j]) {
                                 // player has discarded waiting tile
                                 return false;
                              }
                           }
                        }
                     }

                     if (m_fReach) {
                        for (i = 0; i < m_pOpponent->m_iNumDiscarded; i++) {
                           if (m_pOpponent->m_Discarded[i].flags & DT_OPPONENTREACH) {
                              break;
                           }
                        }
                        if (i >= m_pOpponent->m_iNumDiscarded) {
                           i = -1;
                        }
                        for (i++; i < m_pOpponent->m_iNumDiscarded - 1; i++) {
                           for (j = 0; j < num; j++) {
                              if (m_pOpponent->m_Discarded[i].tile == waiting[j]) {
                                 return false;
                              }
                           }
                        }
                     }
                  }

                  CHand bak = m_Hand;
                  if (m_Hand.GoMahjong(m_pOpponent->LastDiscardedTile())) {
                     CalcResult();
                     m_Hand = bak;
                     if (m_fTuiDaoHu || HasFan() - m_Result.dora > 0) {
                        return true;
                     }
                  }
                  return false;
               }

            default:
               return false;
         }
         break;

      case AS_DISCARD:
         switch (act) {
            case PA_DRAW:
               return false;

            case PA_DISCARD:
               return true;

            case PA_PUNG:
               return false;

            case PA_CHOW:
               return false;

            case PA_KONG:
               for (i = m_Hand.m_iNumTiles - 1; i >= 0; i--) {
                  if (m_Hand.m_Tiles[i].flags & HT_OPEN) {
                     continue;
                  }
                  if (m_Hand.CanKong(m_Hand.m_Tiles[i].tile, false)) {
                     return true;
                  }
                  if (m_fReach) {
                     break;
                  }
               }
               return false;

            case PA_REACH:
               if (m_Hand.IsConcealed() && !m_fReach && m_iNumDiscarded < 19 && m_pOpponent->m_iNumDiscarded < 20) {
                  for (i = 0; i < m_Hand.m_iNumTiles; i++) {
                     if (m_Hand.m_Tiles[i].flags & HT_LOCKED) {
                        continue;
                     }
                     CHand t = m_Hand;
                     t.RemoveTile(i);
                     if (t.IsReady()) {
                        return true;
                     }
                  }
               }
               return false;

            case PA_MAHJONG:
               if (m_Hand.GoMahjong()) {
                  CalcResult();
                  m_Hand.Restore();

                  if (m_fTuiDaoHu || HasFan() - m_Result.dora > 0) {
                     return true;
                  }
               }
               return false;

            default:
               return false;
         }
         break;
   }

   return false;
}

bool CBasePlayer::IsRoundDraw()
{
   return (m_iNumDiscarded >= 20 ||
      (!gpGame->CanDeclareKong() && !(m_iState & PS_KONGED)));
}

void CBasePlayer::DrawResult()
{
   const char *res[] = {
      msg("1han"),
      msg("2han"),
      msg("3han"),
      msg("4han"),
      msg("man"),
      msg("hane"),
      msg("bai"),
      msg("3bai"),
      msg("yaku")
   };

   int top = 10, y;
   if (IsBot()) {

      top += 85;
   }

   y = top;

#define DRAW_FAN(name, fan, fanc)                            \
   {                                                         \
      if (m_Result.name) {                                   \
         gpGeneral->DrawUTF8Text(msg(#name), 20, y, 0, 255, 255, 0); \
         if (fan < 13) {                                     \
            gpGeneral->DrawUTF8Text(va(msg("num_fan"),       \
               (m_Result.concealed ? fanc : fan) * m_Result.name), \
               175, y);                                      \
         }                                                   \
         y += 32;                                            \
      }                                                      \
   }

   DRAW_FAN(tenhou, 13, 13);
   DRAW_FAN(renhou, 13, 13);
   DRAW_FAN(chihou, 13, 13);
   DRAW_FAN(tuisou, 13, 13);
   DRAW_FAN(chinraotou, 13, 13);
   DRAW_FAN(ryuisou, 13, 13);
   DRAW_FAN(daisangen, 13, 13);
   DRAW_FAN(daisusi, 13, 13);
   DRAW_FAN(syosusi, 13, 13);
   DRAW_FAN(suankou, 13, 13);
   DRAW_FAN(surenkou, 13, 13);
   DRAW_FAN(kokusi, 13, 13);
   DRAW_FAN(cyurenpotou, 13, 13);
   DRAW_FAN(sukantu, 13, 13);

   if (!HasYakuman()) {
      DRAW_FAN(reach, 0, 1);
      DRAW_FAN(pinfu, 0, 1);
      DRAW_FAN(ippatu, 0, 1);
      DRAW_FAN(double_reach, 0, 1);
      DRAW_FAN(tanyao, 1, 1);
      DRAW_FAN(tumo, 0, 1);
      DRAW_FAN(ryanpeikou, 3, 3);
      DRAW_FAN(iipeikou, 0, 1);
      DRAW_FAN(sansiki, 1, 2);
      DRAW_FAN(sansiki_doukoku, 2, 2);
      DRAW_FAN(toitoi, 2, 2);
      DRAW_FAN(sanankou, 2, 2);
      DRAW_FAN(sanrenkou, 2, 2);
      DRAW_FAN(junchan, 2, 3);
      DRAW_FAN(chanta, 1, 2);
      DRAW_FAN(ittu, 1, 2);
      DRAW_FAN(chinitu, 5, 6);
      DRAW_FAN(honitu, 2, 3);
      DRAW_FAN(chitoitu, 0, 2);
      DRAW_FAN(honraotou, 2, 2);
      DRAW_FAN(syosangen, 2, 2);
      DRAW_FAN(haitei, 1, 1);
      DRAW_FAN(houtei, 1, 1);
      DRAW_FAN(rinshan, 1, 1);
      DRAW_FAN(fanpai, 1, 1);
      DRAW_FAN(dora, 1, 1);
      DRAW_FAN(sankantu, 2, 2);
      DRAW_FAN(nofan, 1, 1);
   }

   if (!HasFan()) {
      gpGeneral->DrawUTF8Text(msg("nofan"), 20, y, 0, 255, 255, 0);
   }

   y = top;

   if (!HasYakuman()) {
      gpGeneral->DrawUTF8Text(msg("total"), 420, y, 0, 255, 255, 0);
      y += 32;

      gpGeneral->DrawUTF8Text(va(msg("num_fu_fan"), m_Result.fu, HasFan()), 420, y);
      y += 40;
   }

   gpGeneral->DrawUTF8Text(res[m_Result.level], 420, y, 0, 255, 255, 0);
   y += 40;

   gpGeneral->DrawUTF8Text(va(msg("num_score"), m_Result.score), 420, y, 0, 0, 0, 255);

   gpGeneral->UpdateScreen();
}

int CBasePlayer::IndexToLoc(int index)
{
   int loc = 0, i;

   if (index < 0 || index >= m_Hand.m_iNumTiles)
      return -1; // this isn't a valid index

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if ((m_Hand.m_TileSets[i].type & HT_OPEN) &&
         !(m_Hand.m_TileSets[i].type & HT_RON))
         loc += 3;
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {

      if ((m_Hand.m_Tiles[i].flags & HT_OPEN) &&
         !(m_Hand.m_Tiles[i].flags & HT_RON))
         continue;
      if (i == index)
         return loc;
      loc++;
   }

   return loc;
}

//---------------------------------------------------------------

#define DRAW_LOCATION(l) \
   (((l) >= 13) ? (20 + TILE_WIDTH * (l) + 8) : (20 + TILE_WIDTH * (l)))

CPlayer::CPlayer()
{
   m_fAutoDraw = (atoi(cfg.Get("GAME", "AutoDraw", "0")) > 0);
}

CPlayer::~CPlayer()
{
}

void CPlayer::NewRound()
{
   m_iNumSanGenRush = 0;
   CBasePlayer::NewRound();
}

// Draw the player's hand
void CPlayer::DrawHand(bool shown)
{
   int i, j, loc = 0;
   char c[2] = "A";

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if ((m_Hand.m_TileSets[i].type & HT_OPEN) &&
         !(m_Hand.m_TileSets[i].type & HT_RON))
      {
         if (m_Hand.m_TileSets[i].type & HT_OPENKONG) {
            for (j = 0; j < 3; j++) {
               gpGeneral->EraseArea(DRAW_LOCATION(loc), 380,
                  TILE_WIDTH, 100);
               gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
                  DRAW_LOCATION(loc), 410, PLAYER_SHOWN);
               loc++;
            }
            // Draw another tile as it's a kong
            gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
               DRAW_LOCATION(loc - 2), 390, PLAYER_SHOWN);
         } else if (m_Hand.m_TileSets[i].type & HT_CLOSEDKONG) {
            for (j = 0; j < 3; j++) {
               gpGeneral->EraseArea(DRAW_LOCATION(loc), 380,
                  TILE_WIDTH, 100);
               gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
                  DRAW_LOCATION(loc), 410, WALL_CONCEALED);
               loc++;
            }
            // Draw another tile as it's a kong
            gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
               DRAW_LOCATION(loc - 2), 390, PLAYER_SHOWN);
         } else if (m_Hand.m_TileSets[i].type & HT_OPENPUNG) {
            for (j = 0; j < 3; j++) {
               gpGeneral->EraseArea(DRAW_LOCATION(loc), 380,
                  TILE_WIDTH, 100);
               gpGeneral->DrawTile(m_Hand.m_TileSets[i].first,
                  DRAW_LOCATION(loc), 410, PLAYER_SHOWN);
               loc++;
            }
         } else if (m_Hand.m_TileSets[i].type & HT_OPENCHOW) {
            CTile t = m_Hand.m_TileSets[i].first;
            for (j = 0; j < 3; j++) {
               gpGeneral->EraseArea(DRAW_LOCATION(loc), 380,
                  TILE_WIDTH, 100);
               gpGeneral->DrawTile(t, DRAW_LOCATION(loc),
                  410, PLAYER_SHOWN);
               loc++;
               t = t() + 1;
            }
         } else {
            assert(false); // this should NOT happen
         }
      }
   }

   c[0] += loc;

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if ((m_Hand.m_Tiles[i].flags & HT_OPEN) &&
         !(m_Hand.m_Tiles[i].flags & HT_RON))
      {
         continue;
      }
      gpGeneral->EraseArea(DRAW_LOCATION(loc), 380,
         TILE_WIDTH, 100);
      gpGeneral->DrawTile(m_Hand.m_Tiles[i].tile,
         DRAW_LOCATION(loc), shown ? 396 : 380,
         shown ? PLAYER_SHOWN : PLAYER_CONCEALED);
      gpGeneral->DrawUTF8Text(c, DRAW_LOCATION(loc) + 15, 455);
      c[0]++;
      loc++;
   }

   if (c[0] <= 'N') {
      gpGeneral->EraseArea(DRAW_LOCATION(13), 380,
         TILE_WIDTH, 100);
      gpGeneral->DrawUTF8Text("N", DRAW_LOCATION(13) + 15, 455);
   }

   gpGeneral->UpdateScreen(0, 380, 640, 100);
}

int CPlayer::KeyToIndex(SDLKey key)
{
   int d = key - SDLK_a, i, loc = 0;

   if (d < 0 || d > 14)
      return -1; // this isn't a letter key thru A-N

   for (i = 0; i < m_Hand.m_iNumTileSets; i++) {
      if ((m_Hand.m_TileSets[i].type & HT_OPEN) &&
         !(m_Hand.m_TileSets[i].type & HT_RON))
         loc += 3;
   }

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if ((m_Hand.m_Tiles[i].flags & HT_OPEN) &&
         !(m_Hand.m_Tiles[i].flags & HT_RON))
         continue;
      if (loc == d)
         return i;
      loc++;
   }

   return -1;
}

void CPlayer::FirstChance()
{
   SDLKey key;
   int index, changed = 0;

   gpGeneral->DrawMessage(msg("hint_firstchance"));

   do {
      key = gpGeneral->ReadKey();
      index = KeyToIndex(key);
      if (index < 0 || index >= 13 || (changed & (1 << index)))
         continue;

      changed |= (1 << index);

      CTile::ReturnTile(m_Hand.m_Tiles[index].tile);
      m_Hand.m_Tiles[index].tile = CTile::RandomTile();


      gpGeneral->PlaySound(SND_SOUND3);
      gpGeneral->EraseArea(DRAW_LOCATION(index), 380,
         TILE_WIDTH, TILE_HEIGHT_CONCEALED);
      gpGeneral->DrawTile(m_Hand.m_Tiles[index].tile,
         DRAW_LOCATION(index), 396, WALL_CONCEALED);
      gpGeneral->UpdateScreen(DRAW_LOCATION(index), 380,
         TILE_WIDTH, TILE_HEIGHT_CONCEALED);
   } while (key != SDLK_RETURN);

   gpGeneral->DrawMessage(NULL);

   gpGeneral->PlaySound(SND_DISCARD1);
   DrawHand();
   m_Hand.Sort();
   gpGeneral->UpdateScreen(0, 380, 640, 100);
   UTIL_Delay(800);

   gpGeneral->PlaySound(SND_DISCARD1);

   gpGeneral->EraseArea(0, 380, 640, TILE_HEIGHT_CONCEALED);
   gpGeneral->DrawTiles(NULL, 13, 20, 396, WALL_CONCEALED);
   gpGeneral->UpdateScreen(0, 380, 640, 100);
   UTIL_Delay(1000);

   gpGeneral->PlaySound(SND_DISCARD1);
   DrawHand();
   gpGeneral->UpdateScreen(0, 380, 640, 100);
}

// return true if player wins the round with this chance.
bool CPlayer::LastChance()
{
   CTile waiting[13], wall[10];
   int n = m_Hand.GetWaitingTile(waiting), i, t, flags = 0, chance;
   if (n <= 0) {
      return false; // not ready hand
   }

   gpGeneral->PlaySound(SND_DING); // play a hint sound

   t = RandomLong(1, 3);

   for (i = 0; i < t; i++) {
      int s = RandomLong(0, n - 1);
      wall[i] = (CTile::FetchTile(waiting[s]) ? waiting[s] : CTile::RandomTile());
   }

   for (; i < 10; i++) {
      wall[i] = CTile::RandomTile();
   }

   // shuffle the wall
   for (i = 0; i < 10; i++) {
      CTile tmp = wall[i];
      t = RandomLong(0, 9);
      wall[i] = wall[t];
      wall[t] = tmp;
   }

   gpGeneral->EraseArea(0, 265, 640, 363 - 265);
   gpGeneral->DrawTiles(NULL, 10, 20, 265, WALL_CONCEALED);
   for (i = 0; i < 10; i++) {
      char sz[] = "A";
      sz[0] += (char)i;
      gpGeneral->DrawUTF8Text(sz, 20 + TILE_WIDTH * i + 15, 325);
   }
   gpGeneral->UpdateScreen(0, 265, 640, 480 - 265);

   for (chance = 0; chance < 3; chance++) {
      SDLKey k = gpGeneral->ReadKey();
      t = k - SDLK_a;
      if (t < 0 || t >= 10 || (flags & (1 << t))) {
         chance--;
         continue;
      }
      flags |= (1 << t);
      i = 20 + TILE_WIDTH * t;
      gpGeneral->PlaySound(SND_SOUND4);
      gpGeneral->DrawTile(wall[t], i, 265, WALL_SHOWN);
      gpGeneral->UpdateScreen(i, 265, TILE_WIDTH, TILE_HEIGHT_SHOWN);

      CHand tmp = m_Hand;
      m_Hand.AddTile(wall[t]);
      if (m_Hand.GoMahjong()) {
         CalcResult();
         UTIL_Delay(1000);
         return true;
      }
      m_Hand = tmp;
   }

   UTIL_Delay(500);
   gpGeneral->DrawTiles(wall, 10, 20, 265, WALL_SHOWN);
   gpGeneral->UpdateScreen(20, 265, TILE_WIDTH * 10, TILE_HEIGHT_SHOWN);
   UTIL_Delay(500);

   return false;
}

void CPlayer::DrawDiscarded()
{
   int i, x, y;

   for (i = 0; i < 10 && i < m_iNumDiscarded; i++) {
      gpGeneral->DrawTile(m_Discarded[i].tile, DRAW_LOCATION(i),
         265, WALL_SHOWN);
      if (m_Discarded[i].flags & DT_REACH) {
         bool locked = false;
         if (SDL_MUSTLOCK(gpScreen)) {
            SDL_LockSurface(gpScreen);
            locked = true;
         }
         for (x = DRAW_LOCATION(i); x < DRAW_LOCATION(i) + TILE_WIDTH; x++) {
            for (y = 265; y < 265 + TILE_HEIGHT_SHOWN; y++) {
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

   gpGeneral->EraseArea(DRAW_LOCATION(i), 265, TILE_WIDTH * (10 - i),
      TILE_HEIGHT_SHOWN);

   for (i = 0; i < 10 && i < m_iNumDiscarded - 10; i++) {
      gpGeneral->DrawTile(m_Discarded[i + 10].tile, DRAW_LOCATION(i),
         303, WALL_SHOWN);
      if (m_Discarded[i + 10].flags & DT_REACH) {
         bool locked = false;
         if (SDL_MUSTLOCK(gpScreen)) {
            SDL_LockSurface(gpScreen);
            locked = true;
         }
         for (x = DRAW_LOCATION(i); x < DRAW_LOCATION(i) + TILE_WIDTH; x++) {
            for (y = 303; y < 303 + TILE_HEIGHT_SHOWN; y++) {
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

   gpGeneral->EraseArea(DRAW_LOCATION(i), 325, TILE_WIDTH * (10 - i),
      TILE_HEIGHT_SHOWN - 22);

   if (m_fReach) {
      gpGeneral->DrawDotBar(DRAW_LOCATION(10) + 5, 270);
   } else {
      gpGeneral->EraseArea(DRAW_LOCATION(10) + 5, 270, 20, 85);
   }

   gpGeneral->UpdateScreen(DRAW_LOCATION(0), 265, TILE_WIDTH * 11,
      TILE_HEIGHT_SHOWN * 2 - 22);
}

playeraction CPlayer::Action(actionstate state)
{
   switch (state) {
      case AS_DRAW:
         return ActionDraw();
         break;

      case AS_DISCARD:
         return ActionDiscard();
         break;

      default:
         assert(false);
         break;
   }

   return PA_NONE;
}

playeraction CPlayer::ActionDraw()
{
   int act = PA_DRAW, i;

   if (m_Hand.m_iNumTileSets <= 0 && !m_fReach && !(m_iState & PS_KONGED) && m_iNumSanGenRush > 0 && --m_iCountSanGenRush <= 0) {
      if (SanGenRush() && m_iNumSanGenRush > 0) {
         m_iCountSanGenRush = RandomLong(2, 7);
         m_iNumSanGenRush -= 3;
      }
   }

   while (1) {
      SDLKey key;

      DrawAction((playeraction)act);
      if ((m_iState & PS_KONGED) ||
         (m_iNumDiscarded == 0 && m_pOpponent->m_iNumDiscarded == 0) ||
         (m_fAutoDraw && m_fReach && !ActionIsValid(AS_DRAW, PA_MAHJONG)))
      {
         UTIL_Delay(500);
         key = SDLK_n;
      } else {
         key = gpGeneral->ReadKey();
      }

      switch (key) {
         case SDLK_y:
            if ((m_iState & (PS_ASKED | PS_KONGED)) == 0)
               gpGame->DonDen();
            if (m_Hand.m_iNumTileSets <= 0 && !m_fReach && !(m_iState & PS_KONGED) && m_iNumSanGenRush > 0 && --m_iCountSanGenRush <= 0) {
               if (SanGenRush() && m_iNumSanGenRush > 0) {
                  m_iCountSanGenRush = RandomLong(2, 7);
                  m_iNumSanGenRush -= 3;
               }
            }
            continue;

         case SDLK_n:
            if (IsRoundDraw()) {
               return PA_NONE;
            }
            for (i = 0; i < m_Hand.m_iNumTiles; i++) {
               m_Hand.m_Tiles[i].flags &= ~HT_JUSTGOT;
            }
            if (gpGame->m_fReachBonus && m_fReach) {
               gpGame->m_fReachBonus = false;
               CTile t[13];
               int count;
               count = m_Hand.GetWaitingTile(t);
               assert(count > 0);
               i = RandomLong(0, count - 1);
               if (CTile::FetchTile(t[i])) {
                  m_Hand.AddTile(t[i], HT_JUSTGOT);
               } else {
                  m_Hand.AddRandomTile(HT_JUSTGOT);
                  gpGame->m_fReachBonus = true;
               }
            } else {
               m_Hand.AddRandomTile(HT_JUSTGOT);
            }
            m_iState &= (PS_CALLREACH | PS_KONGED | PS_NOTDISCARDED);
            return PA_DRAW;
            break;

         case SDLK_UP:
         case SDLK_KP8:
         case SDLK_RIGHT:
            do {
               act++;
               if (act >= PA_COUNT) {
                  act = PA_NONE + 1;
               }
            } while (!ActionIsValid(AS_DRAW, (playeraction)act));
            break;

         case SDLK_DOWN:
         case SDLK_KP2:
         case SDLK_LEFT:
            do {
               act--;
               if (act <= PA_NONE) {
                  act = PA_COUNT - 1;
               }
            } while (!ActionIsValid(AS_DRAW, (playeraction)act));
            break;

         case SDLK_SPACE:
         case SDLK_RETURN:
         case SDLK_KP_ENTER:
            switch (act) {
               case PA_DRAW:
                  if (IsRoundDraw()) {
                     return PA_NONE;
                  }
                  for (i = 0; i < m_Hand.m_iNumTiles; i++) {
                     m_Hand.m_Tiles[i].flags &= ~HT_JUSTGOT;
                  }
                  m_Hand.AddRandomTile(HT_JUSTGOT);
                  m_iState &= (PS_CALLREACH | PS_KONGED | PS_NOTDISCARDED);
                  return PA_DRAW;

               case PA_PUNG:
                  if (m_Hand.Pung(m_pOpponent->LastDiscardedTile())) {
                     m_pOpponent->m_iNumDiscarded--;
                     m_iState |= PS_ASKED;
                     return PA_PUNG;
                  }
                  break;

               case PA_CHOW:
                  if (ActionChow() != PA_NONE) {
                     m_iState |= PS_ASKED;
                     return PA_CHOW;
                  }
                  break;

               case PA_KONG:
                  if (m_Hand.Kong(m_pOpponent->LastDiscardedTile(), true)) {
                     m_pOpponent->m_iNumDiscarded--;
                     m_iState |= (PS_KONGED | PS_ASKED);
                     return PA_KONG;
                  }
                  break;

               case PA_MAHJONG:
                  m_Hand.AddTile(m_pOpponent->LastDiscardedTile());
                  return PA_MAHJONG;
                  break;
            }
            break;
      }
   }

   return PA_NONE;
}

playeraction CPlayer::ActionDiscard()
{
   int act = PA_DISCARD;
   bool reach = false;

   while (1) {
      DrawAction((playeraction)act);
      SDLKey key;
      if (m_fReach && m_fAutoDraw && !ActionIsValid(AS_DISCARD, PA_MAHJONG) &&
         !ActionIsValid(AS_DISCARD, PA_KONG)) {
         UTIL_Delay(500);
         key = SDLK_n;
      } else {
         key = gpGeneral->ReadKey();
      }
      int index;
      switch (key) {
         case SDLK_y:
            if (!reach && ((m_iState & (PS_ASKED | PS_KONGED)) == 0))
               gpGame->DonDen();
            continue;

         case SDLK_UP:
         case SDLK_KP8:
         case SDLK_RIGHT:
            if (!reach) {
               do {
                  act++;
                  if (act >= PA_COUNT) {
                     act = PA_NONE + 1;
                  }
               } while (!ActionIsValid(AS_DISCARD, (playeraction)act));
            }
            break;

         case SDLK_DOWN:
         case SDLK_KP2:
         case SDLK_LEFT:
            if (!reach) {
               do {
                  act--;
                  if (act <= PA_NONE) {
                     act = PA_COUNT - 1;
                  }
               } while (!ActionIsValid(AS_DISCARD, (playeraction)act));
            }
            break;

         case SDLK_SPACE:
         case SDLK_RETURN:
         case SDLK_KP_ENTER:
            switch (act) {
               case PA_DISCARD:
                  index = KeyToIndex(SDLK_n);

                  m_Discarded[m_iNumDiscarded].tile = m_Hand.m_Tiles[index].tile;
                  m_Discarded[m_iNumDiscarded].flags = 0;
                  m_iNumDiscarded++;
                  m_Hand.RemoveTile(index);
                  m_Hand.Sort();
                  for (index = 0; index < m_Hand.m_iNumTiles; index++) {
                     m_Hand.m_Tiles[index].flags &= ~HT_JUSTGOT;
                  }

                  if (reach && m_Hand.IsReady()) {
                     m_fReach = true;
                     m_Discarded[m_iNumDiscarded - 1].flags |= DT_REACH;
                     if (m_pOpponent->m_iNumDiscarded > 0) {
                        m_pOpponent->m_Discarded[m_pOpponent->m_iNumDiscarded - 1].flags |= DT_OPPONENTREACH;
                     }
                     m_iState = PS_CALLREACH;
                     return PA_REACH;
                  }

                  m_iState = 0;
                  return PA_DISCARD;

               case PA_REACH:
                  reach = true;
                  gpGeneral->PlaySound(SND_REACH);
                  gpGeneral->DrawDotBar(445, 270, true);
                  gpGeneral->UpdateScreen(445, 270, 20, 85);
                  break;

               case PA_KONG:
                  if (!reach) {
                     if (ActionKong() != PA_NONE) {
                        m_iState |= PS_KONGED;
                        return PA_KONG;
                     }
                  }
                  break;

               case PA_MAHJONG:
                  if (!reach) {
                     return PA_MAHJONG;
                  }
                  break;
            }
            break;

         default:
            if (m_fReach && key != SDLK_n)
               continue;
            index = KeyToIndex(key);
            if (index < 0) {
               continue;
            }
            m_Discarded[m_iNumDiscarded].tile = m_Hand.m_Tiles[index].tile;
            m_Discarded[m_iNumDiscarded].flags = 0;
            m_iNumDiscarded++;
            m_Hand.RemoveTile(index);
            m_Hand.Sort();
            for (index = 0; index < m_Hand.m_iNumTiles; index++) {
               m_Hand.m_Tiles[index].flags &= ~HT_JUSTGOT;
            }

            if (reach && m_Hand.IsReady()) {
               m_fReach = true;
               m_Discarded[m_iNumDiscarded - 1].flags |= DT_REACH;
               if (m_pOpponent->m_iNumDiscarded > 0) {
                  m_pOpponent->m_Discarded[m_pOpponent->m_iNumDiscarded - 1].flags |= DT_OPPONENTREACH;
               }
               m_iState = PS_CALLREACH;
               return PA_REACH;
            }

            m_iState = 0;
            return PA_DISCARD;
      }
   }
   return PA_NONE;
}

playeraction CPlayer::ActionChow()
{
   int index[3][2] = {{-1, -1}, {-1, -1}, {-1, -1}}, count = 0, i, j, l = -1;
   CTile t = m_pOpponent->LastDiscardedTile(), to_find[2];

   for (i = CHOW_LOWER; i <= CHOW_UPPER; i++) {
      switch (i) {
         case CHOW_LOWER:
            if (t.GetValue() > 7)
               continue;
            to_find[0] = t() + 1;
            to_find[1] = t() + 2;
            break;

         case CHOW_MIDDLE:
            if (t.GetValue() == 1 || t.GetValue() == 9)
               continue;
            to_find[0] = t() - 1;
            to_find[1] = t() + 1;
            break;

         case CHOW_UPPER:
            if (t.GetValue() < 3)
               continue;
            to_find[0] = t() - 1;
            to_find[1] = t() - 2;
            break;

         default:
            TerminateOnError("CPlayer::ActionChow(): Invalid location");
      }

      for (j = 0; j < m_Hand.m_iNumTiles; j++) {
         if (m_Hand.m_Tiles[j].flags & HT_LOCKED) {
            continue;
         }
         if (m_Hand.m_Tiles[j].tile == to_find[0]) {
            index[i][0] = j;
         }

         if (m_Hand.m_Tiles[j].tile == to_find[1]) {
            index[i][1] = j;
         }
      }

      if (index[i][0] != -1 && index[i][1] != -1) {
         count++;
         l = i;
      } else {
         index[i][0] = -1;
      }
   }

   if (count <= 0) {
      return PA_NONE;
   } else if (count == 1) {
      assert(l != -1);
      if (m_Hand.Chow(t, l)) {
         m_pOpponent->m_iNumDiscarded--;
         m_iState |= PS_ASKED;
         return PA_CHOW;
      }
   } else {
      l = CHOW_UPPER;
      while (1) {
         if (index[l][0] == -1) {
            l++;
            if (l > CHOW_UPPER) {
               l = CHOW_LOWER;

            }
            continue;
         }
         int loc[2] = {IndexToLoc(index[l][0]), IndexToLoc(index[l][1])};
         SDL_Rect dstrect;

         DrawHand();

         dstrect.x = DRAW_LOCATION(loc[0]) + 5;
         dstrect.y = 382;
         dstrect.w = TILE_WIDTH - 10;
         dstrect.h = 10;

         if (SDL_MUSTLOCK(gpScreen)) {
            SDL_LockSurface(gpScreen);
         }

         SDL_FillRect(gpScreen, &dstrect, SDL_MapRGB(gpScreen->format, 255, 0, 0));

         dstrect.x = DRAW_LOCATION(loc[1]) + 5;

         SDL_FillRect(gpScreen, &dstrect, SDL_MapRGB(gpScreen->format, 255, 0, 0));

         if (SDL_MUSTLOCK(gpScreen)) {
            SDL_UnlockSurface(gpScreen);
         }

         gpGeneral->UpdateScreen(0, 380, 640, 100);

         SDLKey key = gpGeneral->ReadKey();

         switch (key) {
            case SDLK_DOWN:
            case SDLK_KP2:
            case SDLK_LEFT:
               do {
                  l++;
                  if (l > CHOW_UPPER) {
                     l = CHOW_LOWER;
                  }
               } while (index[l][0] == -1);

               break;

            case SDLK_UP:
            case SDLK_KP8:
            case SDLK_RIGHT:
               do {
                  l--;
                  if (l < CHOW_LOWER) {
                     l = CHOW_UPPER;
                  }
               } while (index[l][0] == -1);
               break;

            case SDLK_SPACE:
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
               if (m_Hand.Chow(t, l)) {
                  m_pOpponent->m_iNumDiscarded--;
                  m_iState |= PS_ASKED;
                  return PA_CHOW;
               }
               break;

            default:
               i = KeyToIndex(key);
               if (i >= 0 && i < m_Hand.m_iNumTiles) {
                  if (m_Hand.m_Tiles[i].flags & HT_OPEN) {
                     break;
                  }
                  for (j = 0; j < count; j++) {
                     if (m_Hand.m_Tiles[index[j][0]].tile == m_Hand.m_Tiles[i].tile ||
                        m_Hand.m_Tiles[index[j][1]].tile == m_Hand.m_Tiles[i].tile)
                     {
                        l = j;
                        break;
                     }
                  }
               }
               break;
         }
      }
   }

   return PA_NONE;
}

playeraction CPlayer::ActionKong()
{
   int count = 0, i, l = -1;

   for (i = 0; i < m_Hand.m_iNumTiles; i++) {
      if (m_Hand.m_Tiles[i].flags & HT_OPEN) {
         continue;
      }
      if (m_Hand.CanKong(m_Hand.m_Tiles[i].tile, false)) {
         count++;
         l = i;
      }
   }

   if (count <= 0) {
      return PA_NONE;
   } else if (count <= 4) {
      assert(l != -1);
      if (m_Hand.Kong(m_Hand.m_Tiles[l].tile, false)) {
         m_iState |= PS_KONGED;
         return PA_KONG;
      }
   } else {
      assert(l != -1);
      while (1) {
         int loc = IndexToLoc(l);
         SDL_Rect dstrect;

         DrawHand();

         dstrect.x = DRAW_LOCATION(loc) + 5;
         dstrect.y = 382;
         dstrect.w = TILE_WIDTH - 10;
         dstrect.h = 10;

         if (SDL_MUSTLOCK(gpScreen)) {
            SDL_LockSurface(gpScreen);
         }

         SDL_FillRect(gpScreen, &dstrect, SDL_MapRGB(gpScreen->format, 255, 0, 0));

         if (SDL_MUSTLOCK(gpScreen)) {
            SDL_UnlockSurface(gpScreen);
         }
         gpGeneral->UpdateScreen(0, 380, 640, 100);


         SDLKey key = gpGeneral->ReadKey();

         switch (key) {
            case SDLK_UP:
            case SDLK_KP8:
            case SDLK_RIGHT:
               do {
                  l++;
                  if (l >= m_Hand.m_iNumTiles) {
                     l = 0;
                  }
               } while (!m_Hand.CanKong(m_Hand.m_Tiles[l].tile, false));
               break;

            case SDLK_DOWN:
            case SDLK_KP2:
            case SDLK_LEFT:
               do {
                  l--;
                  if (l < 0) {
                     l = m_Hand.m_iNumTiles - 1;
                  }
               } while (!m_Hand.CanKong(m_Hand.m_Tiles[l].tile, false));
               break;

            case SDLK_SPACE:
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
               if (m_Hand.Kong(m_Hand.m_Tiles[l].tile, false)) {
                  m_iState |= PS_KONGED;
                  return PA_KONG;
               }
               break;

            default:
               i = KeyToIndex(key);
               if (i < 0 || i >= m_Hand.m_iNumTiles) {
                  break;
               }
               if (m_Hand.m_Tiles[i].flags & HT_OPEN) {
                  break;
               }
               if (m_Hand.CanKong(m_Hand.m_Tiles[i].tile, false)) {
                  l = i;
               }
               break;
         }
      }
   }

   return PA_NONE;
}

void CPlayer::DrawAction(playeraction act)
{
   gpGeneral->EraseArea(580, 330, 50, 28);

   const char *pStr = NULL;

   switch (act) {
      case PA_DRAW:
         pStr = msg("player_drawtile");
         break;

      case PA_DISCARD:
         pStr = msg("player_discardtile");
         break;

      case PA_PUNG:
         pStr = msg("player_pung");
         break;

      case PA_CHOW:
         pStr = msg("player_chow");
         break;

      case PA_KONG:
         pStr = msg("player_kong");
         break;

      case PA_REACH:
         pStr = msg("player_reach");
         break;

      case PA_MAHJONG:
         pStr = msg("player_gomahjong");
         break;
   }

   if (pStr == NULL) {
      gpGeneral->UpdateScreen(580, 330, 50, 28);
      return; // do nothing
   }

   gpGeneral->DrawUTF8Text(pStr, 580, 330, 0, 255, 255, 100);
   gpGeneral->UpdateScreen(580, 330, 50, 28);
}

void CPlayer::SanGenInit()
{
   int i, j;
   CTile sangen[3] = {"RD", "GD", "WD"};
   m_iNumSanGenRush = RandomLong(2, 3) * 3;
   for (i = 0; i < m_iNumSanGenRush / 3; i++) {
      for (j = 0; j < 3; j++) {
         CTile::FetchTile(sangen[i]);
         m_SanGenRush[i * 3 + j] = sangen[i];
      }
   }
   m_iCountSanGenRush = RandomLong(2, 7);
}

bool CPlayer::SanGenRush()
{
   SDLKey key;
   int index, changed = 0, num = 0;

   gpGeneral->DrawMessage(msg("sangenrush"));
   gpGeneral->PlaySound(SND_SOUND2);

   do {
      key = gpGeneral->ReadKey();
      index = KeyToIndex(key);
      if (index < 0 || index >= 13 || (changed & (1 << index)))
         continue;

      changed |= (1 << index);

      CTile::ReturnTile(m_Hand.m_Tiles[index].tile);
      m_Hand.m_Tiles[index].tile = m_SanGenRush[m_iNumSanGenRush - 1];
      num++;

      gpGeneral->PlaySound(SND_SOUND3);
      gpGeneral->EraseArea(DRAW_LOCATION(index), 380,
         TILE_WIDTH, TILE_HEIGHT_CONCEALED);
      gpGeneral->DrawTile(m_Hand.m_Tiles[index].tile,
         DRAW_LOCATION(index), 396, WALL_CONCEALED);
      gpGeneral->UpdateScreen(DRAW_LOCATION(index), 380,
         TILE_WIDTH, TILE_HEIGHT_CONCEALED);
   } while (key != SDLK_RETURN && num < 3);

   UTIL_Delay(200);
   gpGeneral->DrawMessage(NULL);

   gpGeneral->PlaySound(SND_SOUND2);
   DrawHand();
   gpGeneral->UpdateScreen(0, 380, 640, 100);

   return (num > 0);
}

