/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>

using namespace std;
using namespace Belle2;


/*
* TODO
*/
void
DATCONTrackingModule::purifyTrackCandsList()
{
  coord2dPair hc, hc2;
  vector<unsigned int> idList, merged_idList;
  unsigned int cand_cnt = 0;
  double x = 0, y = 0;
  double x_tolerance = 0.2;
  vector<DATCONHoughCand> cpyCand;
  bool useMerger = false;

  /** Start merging for v-side */

  cpyCand = vHoughCand;
  sort(cpyCand.begin(), cpyCand.end());
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    if (it == cpyCand.begin()) {
      x  = hc.first.X() + hc.second.X();
      y  = hc.first.Y() + hc.second.Y();
      merged_idList = idList;
      cand_cnt = 1;
    } else if (compareList(idList, merged_idList) && fabs(x / (2.0 * cand_cnt) - hc.first.X()) < x_tolerance
               && it != cpyCand.end()) {
      x += hc.first.X() + hc.second.X();
      y += hc.first.Y() + hc.second.Y();
      ++cand_cnt;
      if (useMerger) {
        mergeIdList(merged_idList, idList);
      } else {
        merged_idList = idList;
      }
    } else {
      if (it != cpyCand.begin()) {
        x /= (2.0 * ((double) cand_cnt));
        y /= (2.0 * ((double) cand_cnt));
        vTrackCand.push_back(DATCONTrackCand(merged_idList, TVector2(x, y)));
      }
      x = hc.first.X() + hc.second.X();
      y = hc.first.Y() + hc.second.Y();
      cand_cnt = 1;
      merged_idList = idList;
    }
  }

  if (cpyCand.size() > 0) {
    x /= (2.0 * ((double) cand_cnt));
    y /= (2.0 * ((double) cand_cnt));
    vTrackCand.push_back(DATCONTrackCand(merged_idList, TVector2(x, y)));
  }

  /** End merging v-side */

  /** Start merging u-side */

  cpyCand = uHoughCand;
  sort(cpyCand.begin(), cpyCand.end());
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    if (it == cpyCand.begin()) {
      x = hc.first.X() + hc.second.X();
      y = hc.first.Y() + hc.second.Y();
      cand_cnt = 1;
      merged_idList = idList;
    } else if (compareList(idList, merged_idList) && fabs(x / (2.0 * cand_cnt) - hc.first.X()) < x_tolerance
               && it != cpyCand.end()) {
      x += hc.first.X() + hc.second.X();
      y += hc.first.Y() + hc.second.Y();
      ++cand_cnt;
      if (useMerger) {
        mergeIdList(merged_idList, idList);
      } else {
        merged_idList = idList;
      }
    } else {
      if (it != cpyCand.begin()) {
        x /= (2.0 * ((double) cand_cnt));
        y /= (2.0 * ((double) cand_cnt));
        uTrackCand.push_back(DATCONTrackCand(merged_idList, TVector2(x, y)));
      }
      x = hc.first.X() + hc.second.X();
      y = hc.first.Y() + hc.second.Y();
      cand_cnt = 1;
      merged_idList = idList;
    }
  }

  if (cpyCand.size() > 0) {
    x /= (2.0 * ((double) cand_cnt));
    y /= (2.0 * ((double) cand_cnt));
    uTrackCand.push_back(DATCONTrackCand(merged_idList, TVector2(x, y)));
  }

  /** End merging u-side */

}

/*
* Function to compare the hit list.
* Returns "true", if the IDs of two hits in the two lists are equal and if a hit on at least m_minimumLines is found.
*/
bool
DATCONTrackingModule::compareList(std::vector<unsigned int>& aList, std::vector<unsigned int>& bList)
{
  unsigned int countLayer = 0;
  int layera = 0;
  int hitIDa = 0, hitIDb = 0;
  bool foundLayer[4] = {false};

  for (auto ita = aList.begin(); ita != aList.end(); ++ita) {
    for (auto itb = bList.begin(); itb != bList.end(); ++itb) {
      hitIDa      = (int)(*ita) - (((int)(*ita) / 10000) * 10000);
      hitIDb      = (int)(*itb) - (((int)(*itb) / 10000) * 10000);
      layera    = (int)(*ita) / 10000000;
      if (hitIDa == hitIDb && !foundLayer[layera - 3]) {
        foundLayer[layera - 3] = true;
        ++countLayer;
      }
    }
  }

  if (countLayer >= m_minimumLines) {
    return (true);
  } else {
    return (false);
  }
  return (true);
}


/*
* Merge Id lists.
*/
void
DATCONTrackingModule::mergeIdList(std::vector<unsigned int>& mergedList, std::vector<unsigned int>& mergeme)
{
  bool found;

  for (auto mergemeit = mergeme.begin(); mergemeit != mergeme.end(); ++mergemeit) {
    found = false;
    for (auto mergedit = mergedList.begin(); mergedit != mergedList.end(); ++mergedit) {
      if (*mergedit == *mergemeit) {
        found = true;
        break;
      }
    }
    if (!found) {
      mergedList.push_back(*mergemeit);
    }
  }
}
