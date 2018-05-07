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
* Function to compare the hit list. Different options to return true
* possible: exact match, contains etc...
*/
bool
DATCONTrackingModule::compareList(std::vector<unsigned int>& aList, std::vector<unsigned int>& bList)
{
  unsigned int cnt = 0;
  bool exact_match = false;
  bool contains = true;
  bool found;
  int sensorID_a = 0, sensorID_b = 0;
  int layer_a = 0;
  int hitIDa = 0, hitIDb = 0;
  bool foundLayer[4] = {false};

  /* If exact match and size not equal return false */
  if (m_countStrips) {
    if (exact_match && aList.size() != bList.size()) {
      return (false);
    }

    for (auto ita = aList.begin(); ita != aList.end(); ++ita) {
      found = false;
      for (auto itb = bList.begin(); itb != bList.end(); ++itb) {
        if (*itb == *ita) {
          found = true;
          ++cnt;
        }
      }
      if (!contains && !found) {
        return (false);
      }
    }
  } else {    // belongs to if (m_countStrips)

    if (exact_match && aList.size() != bList.size()) {
      return (false);
    }

    for (auto ita = aList.begin(); ita != aList.end(); ++ita) {
      found = false;
      for (auto itb = bList.begin(); itb != bList.end(); ++itb) {
        hitIDa    = (int)(*ita) - (((int)(*ita) / 1000) * 1000);
        hitIDb    = (int)(*itb) - (((int)(*itb) / 1000) * 1000);
        sensorID_a = (int)(*ita) - hitIDa;
        sensorID_b = (int)(*itb) - hitIDb;
        layer_a    = (int)(*ita) / 1000000;
        if (sensorID_a == sensorID_b) {
          if (!(foundLayer[layer_a - 3])) {
            found = true;
            foundLayer[layer_a - 3] = true;
            ++cnt;
          }
        }
      }
      if (!contains && !found) {
        return (false);
      }
    }
  }

  if (contains && cnt >= m_minimumLines) {
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
