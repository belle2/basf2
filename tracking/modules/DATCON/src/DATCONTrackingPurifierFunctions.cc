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
  vector<unsigned int> idList, last_idList, merged_idList;
  unsigned int cand_cnt, found_tracks;
  double x, y, last_x;
  double x_tolerance = 1.0;
  vector<DATCONHoughCand> cpyCand;
  bool useMerger = false;

  //m_countStrips = true;

  /** Start Clustering for v-side */

  found_tracks = 0;
  cpyCand = vHoughCand;
  sort(cpyCand.begin(), cpyCand.end());
  cand_cnt = 0;
  x = 0.0;
  last_x = 0;
  y = 0.0;
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    if (it == cpyCand.begin()) {
      x += hc.first.X();
      x += hc.second.X();
      y += hc.first.Y();
      y += hc.second.Y();
      merged_idList = idList;
      ++cand_cnt;
    } else if (compareList(idList, last_idList) && fabs(last_x - hc.first.X()) < x_tolerance
               && it != cpyCand.end()) {
      x += hc.first.X();
      x += hc.second.X();
      y += hc.first.Y();
      y += hc.second.Y();
      ++cand_cnt;
      if (useMerger) {
        mergeIdList(merged_idList, idList, last_idList);
      } else {
        merged_idList = idList;
      }
    } else {
      if (it != cpyCand.begin()) {
        x /= (2.0 * ((double) cand_cnt));
        y /= (2.0 * ((double) cand_cnt));
        ++found_tracks;
        vTrackCand.push_back(DATCONTrackCand(merged_idList, TVector2(x, y)));
      }
      x = hc.first.X();
      x += hc.second.X();
      y = hc.first.Y();
      y += hc.second.Y();
      cand_cnt = 1;
      merged_idList = idList;
    }
    last_idList = idList;
    last_x = hc.first.X();
  }

  if (cpyCand.size() > 0) {
    x /= (2.0 * ((double) cand_cnt));
    y /= (2.0 * ((double) cand_cnt));
    ++found_tracks;
    vTrackCand.push_back(DATCONTrackCand(merged_idList, TVector2(x, y)));
  }

  /** End Clustering v-side */

  /** Start Clustering u-side */

  found_tracks = 0;
  cpyCand = uHoughCand;
  sort(cpyCand.begin(), cpyCand.end());
  cand_cnt = 0;
  x = 0.0;
  last_x = 0;
  y = 0.0;
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    if (it == cpyCand.begin()) {
      x += hc.first.X();
      x += hc.second.X();
      y += hc.first.Y();
      y += hc.second.Y();
      ++cand_cnt;
      merged_idList = idList;
    } else if (compareList(idList, last_idList) && fabs(last_x - hc.first.X()) < x_tolerance
               && it != cpyCand.end()) {
      x += hc.first.X();
      x += hc.second.X();
      y += hc.first.Y();
      y += hc.second.Y();
      ++cand_cnt;
      if (useMerger) {
        mergeIdList(merged_idList, idList, last_idList);
      } else {
        merged_idList = idList;
      }
    } else {
      if (it != cpyCand.begin()) {
        x /= (2.0 * ((double) cand_cnt));
        y /= (2.0 * ((double) cand_cnt));
        ++found_tracks;
        uTrackCand.push_back(DATCONTrackCand(merged_idList, TVector2(x, y)));
      }
      x = hc.first.X();
      x += hc.second.X();
      y = hc.first.Y();
      y += hc.second.Y();
      cand_cnt = 1;
      merged_idList = idList;
    }
    last_idList = idList;
    last_x = hc.first.X();
  }

  if (cpyCand.size() > 0) {
    x /= (2.0 * ((double) cand_cnt));
    y /= (2.0 * ((double) cand_cnt));
    ++found_tracks;
    uTrackCand.push_back(DATCONTrackCand(merged_idList, TVector2(x, y)));
  }

  /** End Clustering u-side */

  //m_countStrips = false;

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
  int strip_a = 0, strip_b = 0;
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
        strip_a    = (int)(*ita) - (((int)(*ita) / 1000) * 1000);
        strip_b    = (int)(*itb) - (((int)(*itb) / 1000) * 1000);
        sensorID_a = (int)(*ita) - strip_a;
        sensorID_b = (int)(*itb) - strip_b;
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
DATCONTrackingModule::mergeIdList(std::vector<unsigned int>& merged, std::vector<unsigned int>& a,
                                  std::vector<unsigned int>& b)
{
  bool found;

  for (auto it = a.begin(); it != a.end(); ++it) {
    found = false;
    for (auto it_in = merged.begin(); it_in != merged.end(); ++it_in) {
      if (*it_in == *it) {
        found = true;
      }
    }
    if (!found) {
      merged.push_back(*it);
    }
  }

  for (auto it = b.begin(); it != b.end(); ++it) {
    found = false;
    for (auto it_in = merged.begin(); it_in != merged.end(); ++it_in) {
      if (*it_in == *it) {
        found = true;
      }
    }
    if (!found) {
      merged.push_back(*it);
    }
  }
}

/*
* Merge Id lists.
*/
void
DATCONTrackingModule::mergeIdList(std::vector<unsigned int>& mergedList, std::vector<unsigned int>& mergeme)
{
  bool found;

  for (auto it = mergeme.begin(); it != mergeme.end(); ++it) {
    found = false;
    for (auto it_in = mergedList.begin(); it_in != mergedList.end(); ++it_in) {
      if (*it_in == *it) {
        found = true;
        break;
      }
    }
    if (!found) {
      mergedList.push_back(*it);
    }
  }
}
