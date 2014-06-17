/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/svdHoughtracking/SVDHoughtrackingModule.h>
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/GeoVXDPosition.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/SVDHoughCluster.h>
#include <tracking/dataobjects/SVDHoughTrack.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <root/TMath.h>
#include <root/TGeoMatrix.h>
#include <root/TRandom.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

/* Hack for re-plot */
int gplot_first = 1;

/* Debug options */
bool createCoordList = false;

/*
 * Transform to Hough space
 */
void
SVDHoughtrackingModule::houghTrafo2d(svdClusterMap& mapClusters, bool n_side, bool hess = false)
{
  int cnt;
  double r;
  svdClusterMapIter iter;
  svdClusterPair clusterInfo;
  TVector3 pos;
  TVector2 hough, center;
  VxdID sensorID;

  if (n_side) {
    B2DEBUG(200, "Hough Transformation: N-Side");
  } else {
    B2DEBUG(200, "Hough Transformation: P-Side");
  }

  cnt = 0;
  center.Set(0.0, 0.0);
  for (iter = mapClusters.begin(); iter != mapClusters.end(); ++iter) {
    clusterInfo = iter->second;
    pos = clusterInfo.second;
    sensorID = clusterInfo.first;
    if (n_side) {
      if (hess) {
        r = ((pow(pos.X() - center.X(), 2.0) + pow(pos.Z() - center.Y(), 2.0)));
        hough.Set(pos.X() / r, pos.Z() / r);
      } else {
        hough.Set(pos.Y(), pos.Z());
      }
      n_hough.insert(make_pair(cnt, make_pair(sensorID, hough)));
      B2DEBUG(200, "  Sensor: " << clusterInfo.first << " Position: "
              << hough.X() << " " << hough.Y());
    } else {
      if (hess) {
        r = ((pow(pos.X() - center.X(), 2.0) + pow(pos.Y() - center.Y(), 2.0)));
        hough.Set(pos.X() / r, pos.Y() / r);
      } else {
        hough.Set(pos.X(), pos.Y());
      }
      p_hough.insert(make_pair(cnt, make_pair(sensorID, hough)));
      B2DEBUG(200, "  Sensor: " << clusterInfo.first << " Position: "
              << hough.X() << " " << hough.Y());
    }
    ++cnt;
  }

  /* Write hough space into gnuplot file */
  if (m_writeHoughSpace) {
    houghTrafoPlot(n_side);
  }
}

void
SVDHoughtrackingModule::houghTrafoPlot(bool n_side)
{
  FILE* fp;
  TVector2 pos;
  ostringstream fmt_s;
  string n_path = "dbg/n_hough.plot";
  string p_path = "dbg/p_hough.plot";
  string n_load = "dbg/n_rect.plot";
  string p_load = "dbg/p_rect.plot";
  string fmt;
  houghPair hp;

  if (n_side) {
    if ((fp = gplotCreate(n_path.c_str(), NULL, GPLOT_TMPL1)) == NULL) {
      B2ERROR("Cannot open hough debug file!");
      return;
    }
    gplotSetLoad(fp, n_load.c_str());
    for (auto it = n_hough.begin(); it != n_hough.end(); ++it) {
      hp = it->second;
      pos = hp.second;
      fmt_s.str("");
      fmt_s.clear();
      fmt_s << "plot " << pos.X() << " * cos(x) + " << pos.Y() << " * sin(x) notitle linestyle 1" << endl;
      fmt = fmt_s.str();
      gplotInsert(fp, fmt.c_str(), GPLOT_LINE);
    }
  } else {
    if ((fp = gplotCreate(p_path.c_str(), NULL, GPLOT_TMPL1)) == NULL) {
      B2ERROR("Cannot open hough debug file!");
      return;
    }
    gplotSetLoad(fp, p_load.c_str());
    for (auto it = p_hough.begin(); it != p_hough.end(); ++it) {
      hp = it->second;
      pos = hp.second;
      fmt_s.str("");
      fmt_s.clear();
      fmt_s << "plot " << pos.X() << " * cos(x) + " << pos.Y() << " * sin(x) notitle linestyle 1" << endl;
      fmt = fmt_s.str();
      gplotInsert(fp, fmt.c_str(), GPLOT_LINE);
    }
  }
  gplotClose(fp, GPLOT_INPLOT);
}

/*
 * Very simple sensor filter.
 */
bool
SVDHoughtrackingModule::layerFilter(bool* layer)
{
  if (layer[0] == true && layer[1] == true && layer[2] == true && layer[3] == true) {
    return (true);
  }
  //B2DEBUG(1, " !!!!$$$$$!!!! Test: " << layer[0] << " " << layer[1] << " " << layer[2] << " " << layer[3]);

  return (false);
}

/*
 * Find the intercept in hough, or hess hough space.
 * Return zero on success.
 * x = m
 * y = a
 */
int
SVDHoughtrackingModule::fastInterceptFinder2d(houghMap& hits, bool n_side, TVector2 v1_s,
                                              TVector2 v2_s, TVector2 v3_s, TVector2 v4_s, unsigned int iterations, unsigned int critIterations,
                                              unsigned int maxIterations, vector<houghDbgPair>& dbg_rect)
{
  int i, j, idx;
  int br = 0;
  unsigned int cnt_tracks;
  unsigned int dbg_start_iteration = 0;
  double unitx, unity;
  double y1, y2;
  double m, a;
  houghPair hp;
  VxdID sensor;
  vector<unsigned int> cand_dx;

  TVector2 v1, v2, v3, v4;

  unitx = ((v2_s.X() - v1_s.X()) / 2.0);
  unity = ((v1_s.Y() - v4_s.Y()) / 2.0);

  cnt_tracks = 0;
  for (i = 0; i < 2 ; ++i) {
    for (j = 0; j < 2; ++j) {
      v1.Set((v1_s.X() + (double) i * unitx), (v1_s.Y() - (double) j * unity));
      v2.Set((v1_s.X() + (double) i * unitx + unitx), (v1_s.Y() - (double) j * unity));
      v3.Set((v1_s.X() + (double) i * unitx + unitx), (v1_s.Y() - (double) j * unity - unity));
      v4.Set((v1_s.X() + (double) i * unitx), (v1_s.Y() - (double) j * unity - unity));

      cand_dx.clear();
      bool layerHit[4] = {false}; /* For layer filter */
      //for (k = 0; k < hits.size(); ++k) {
      for (auto it = hits.begin(); it != hits.end(); ++it) {
        idx = it->first;
        hp = it->second;
        sensor = hp.first;
        m = hp.second.X();
        a = hp.second.Y();
        y1 = m * cos(v1.X()) + a * sin(v1.X());
        y2 = m * cos(v2.X()) + a * sin(v2.X());

        //if (y1 > v1.y && y1 < v3.y || y2 > v1.y && y2 < v3.y ||
        //    y1 > v3.y && y2 < v1.y || y1 < v3.y && y2 > v1.y) {
        if (!((y1 > v1.Y() && y2 > v2.Y()) || (y1 < v4.Y() && y2 < v3.Y()))) {
          if (/*cnt_tracks < 4 &&*/ iterations == critIterations) {
            cand_dx.push_back(idx);
          }

          layerHit[sensor.getLayerNumber() - 3] = true; /* layer filter */
          ++cnt_tracks;

          if (iterations > 4) {
            B2DEBUG(250, "    Hit id: " << idx << " Layer: " << sensor.getLayerNumber());
          }
        }
      }
      if (cnt_tracks >= m_minimumLines) {
        if (m_useSensorFilter && !layerFilter(layerHit)) {
          B2DEBUG(250, "  Kicked out by sensor filter");
        } else {
          if (iterations >= dbg_start_iteration) {
            dbg_rect.push_back(make_pair(iterations, make_pair(v1, v3)));
          }

          if (iterations != critIterations && !br) {
            fastInterceptFinder2d(hits, n_side, v1, v2, v3, v4, iterations + 1, critIterations, maxIterations,
                                  dbg_rect);
          } else {
            if (n_side) {
              n_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v2)));
            } else {
              p_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v2)));
            }
            B2DEBUG(250, "  Add candidate: (" << v1.X() << ", " << v1.Y() << ") (" << v3.X() << ", "
                    << v3.Y() << ") V3_s: " << v3_s.Y());
          }
        }
      }

      cnt_tracks = 0;
    }
  }

  return 0;
}

/*
 * Create rectangulars for debugging in gnuplot
 */
void
SVDHoughtrackingModule::gplotRect(const string name, vector<houghDbgPair>& hp)
{
  unsigned int iteration;
  ofstream os;
  TVector2 v1, v2;
  coord2dPair hc;

  os.open(name.c_str(), ofstream::out);

  for (auto it = hp.begin(); it != hp.end(); ++it) {
    iteration = it->first;
    hc = it->second;
    v1 = hc.first;
    v2 = hc.second;
    os << "set object rect from " << v1.X() << ", " << v1.Y() << " to " << v2.X() << ", " << v2.Y();
    os << " fc rgb \"" << rectColor[iteration % 8] << "\" fs solid 0.5 behind" << endl;
  }

  os.close();
}

FILE*
SVDHoughtrackingModule::gplotCreate(const char* name, char* dat, int what)
{
  FILE* fp;

  if (name == NULL) {
    fp = stdout;
  } else {
    if ((fp = fopen(name, "w")) == NULL) {
      perror("Error: fopen()");

      return NULL;
    }
  }

  if (dat == NULL) {
    B2DEBUG(400, "gplotCreate: Dat is zero");
  }

  switch (what) {
    case GPLOT_INPLOT:
      fprintf(fp, "%s", gplot_text2.c_str());
      break;
    case GPLOT_TMPL1:
      fprintf(fp, "%s", gplot_tmpl1.c_str());
      break;
  }

  return fp;
}

/*
 * List Hough Candidates.
 */
void
SVDHoughtrackingModule::printHoughCandidates()
{
  coord2dPair hc;
  vector<unsigned int> idList;

  B2DEBUG(2, "List of Hough Candidates on N-Side:");
  for (unsigned int i = 0; i < n_houghCand.size(); ++i) {
    idList = n_houghCand[i].getIdList();
    hc = n_houghCand[i].getCoord();
    B2DEBUG(2, "  Coordinates: (" << hc.first.X() << ", " << hc.first.Y() << ") (" << hc.second.X() << ", "
            << hc.second.Y() << ") Hit size: " << n_houghCand[i].getHitSize()
            << " Hash: " <<  n_houghCand[i].getHash());
    for (unsigned int j = 0; j < idList.size(); ++j) {
      B2DEBUG(2, "    ID: " << idList[j]);
    }
  }

  B2DEBUG(2, "List of Hough Candidates on P-Side:");
  for (unsigned int i = 0; i != p_houghCand.size(); ++i) {
    idList = p_houghCand[i].getIdList();
    hc = p_houghCand[i].getCoord();
    B2DEBUG(2, "  Coordinates: (" << hc.first.X() << ", " << hc.first.Y() << ") (" << hc.second.X() << ", "
            << hc.second.Y() << ") Hit size: " << p_houghCand[i].getHitSize()
            << " Hash: " << p_houghCand[i].getHash());
    for (unsigned int j = 0; j < idList.size(); ++j) {
      B2DEBUG(2, "    ID: " << idList[j]);
    }
  }
}

/*
 * List Track Candidates.
 */
void
SVDHoughtrackingModule::printTrackCandidates()
{
  TVector2 coord;
  vector<unsigned int> idList;

  B2DEBUG(2, "List of Track Candidates on N-Side:");
  for (unsigned int i = 0; i < n_houghTrackCand.size(); ++i) {
    idList = n_houghTrackCand[i].getIdList();
    coord = n_houghTrackCand[i].getCoord();
    B2DEBUG(2, "  Coordinates: (" << coord.X() << ", " << coord.Y() << ") "
            << "Hit size: " << n_houghTrackCand[i].getHitSize()
            << " Hash: " << n_houghTrackCand[i].getHash());
    for (unsigned int j = 0; j < idList.size(); ++j) {
      B2DEBUG(2, "    ID: " << idList[j]);
    }
  }

  B2DEBUG(2, "List of Track Candidates on P-Side:");
  for (unsigned int i = 0; i != p_houghTrackCand.size(); ++i) {
    idList = p_houghTrackCand[i].getIdList();
    coord = p_houghTrackCand[i].getCoord();
    B2DEBUG(2, "  Coordinates: (" << coord.X() << ", " << coord.Y() << ") "
            << "Hit size: " << p_houghTrackCand[i].getHitSize()
            << " Hash: " << p_houghTrackCand[i].getHash());
    for (unsigned int j = 0; j < idList.size(); ++j) {
      B2DEBUG(2, "    ID: " << idList[j]);
    }
  }
}

/*
 * Wrapper for the purify tracks. Here you can select what kind of
 * method you want to use.
 */
void
SVDHoughtrackingModule::purifyTrackCands()
{
  if (m_useHashPurify) {
    /* Use hash method */
    purifyTrackCandsHash();
  } else {
    /* Compare the whole list */
    purifyTrackCandsList();
    //oldpurifyTrackCands();
  }
}

/*
 * Hash purify tracks function for both sides. Using the complete
 * track ID list for comparison.
 */
void
SVDHoughtrackingModule::purifyTrackCandsList()
{
  coord2dPair hc, hc2;
  vector<unsigned int> idList, last_idList, merged_idList;
  unsigned int cand_cnt, found_tracks;
  double x, y, last_x;
  double x_tolerance = 1.0;
  vector<SVDHoughCand> cpyCand;
  bool useMerger = false;

  found_tracks = 0;
  cpyCand = n_houghCand;
  sort(cpyCand.begin(), cpyCand.end());
  cand_cnt = 0;
  x = 0.0;
  last_x = 0;
  y = 0.0;
  B2DEBUG(200, "Tracks found in N-Side: " << cpyCand.size());
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
        B2DEBUG(200, "  Track [ " << found_tracks << " ]: " << x << " " << y);
        n_houghTrackCand.push_back(SVDHoughTrackCand(merged_idList, TVector2(x, y)));
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
    B2DEBUG(200, "  Track [ " << found_tracks << " ]: " << x << " " << y);
    n_houghTrackCand.push_back(SVDHoughTrackCand(merged_idList, TVector2(x, y)));
  }

  found_tracks = 0;
  cpyCand = p_houghCand;
  sort(cpyCand.begin(), cpyCand.end());
  cand_cnt = 0;
  x = 0.0;
  last_x = 0;
  y = 0.0;
  B2DEBUG(200, "Tracks found in P-Side: " << cpyCand.size());
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    if (it == cpyCand.begin()) {
      B2DEBUG(200, "    Begin");
      x += hc.first.X();
      x += hc.second.X();
      y += hc.first.Y();
      y += hc.second.Y();
      ++cand_cnt;
      merged_idList = idList;
    } else if (compareList(idList, last_idList) && fabs(last_x - hc.first.X()) < x_tolerance
               && it != cpyCand.end()) {
      B2DEBUG(200, "    Compare successfull... merge");
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
        B2DEBUG(200, "  Track [ " << found_tracks << " ]: " << x << " " << y);
        p_houghTrackCand.push_back(SVDHoughTrackCand(merged_idList, TVector2(x, y)));
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
    B2DEBUG(200, "  Track [ " << found_tracks << " ]: " << x << " " << y);
    p_houghTrackCand.push_back(SVDHoughTrackCand(merged_idList, TVector2(x, y)));
  }
}

/*
 * Function to compare the hit list. Different options to return true
 * possible: exact match, contains etc...
 */
bool
SVDHoughtrackingModule::compareList(std::vector<unsigned int>& a, std::vector<unsigned int>& b)
{
  unsigned int cnt = 0;
  bool exact_match = false;
  bool contains = true;
  bool found;

  /* If exact match and size not equal return false */
  if (exact_match && a.size() != b.size()) {
    return (false);
  }

  for (auto it = a.begin(); it != a.end(); ++it) {
    found = false;
    for (auto it_in = b.begin(); it_in != b.end(); ++it_in) {
      if (*it_in == *it) {
        found = true;
        ++cnt;
      }
    }
    if (!contains && !found) {
      return (false);
    }
  }

  if (contains && cnt > 3) {
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
SVDHoughtrackingModule::mergeIdList(std::vector<unsigned int>& merged, std::vector<unsigned int>& a,
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
 * Hash purify tracks function for both sides.
 */
void
SVDHoughtrackingModule::purifyTrackCandsHash()
{
  coord2dPair hc, hc2;
  vector<unsigned int> idList, last_idList;
  unsigned int hash, size, cand_cnt, found_tracks;
  unsigned int last_hash, last_size;
  double x, y;
  vector<SVDHoughCand> cpyCand;

  found_tracks = 0;
  cpyCand = n_houghCand;
  sort(cpyCand.begin(), cpyCand.end());
  last_hash = 0;
  last_size = 0;
  cand_cnt = 0;
  x = 0.0;
  y = 0.0;
  B2DEBUG(200, "Tracks found in N-Side: " << cpyCand.size());
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    hash = it->getHash();
    size = it->getHitSize();
    if (((last_hash == hash && last_size == size)) && it != cpyCand.end()) {
      x += hc.first.X();
      x += hc.second.X();
      y += hc.first.Y();
      y += hc.second.Y();
      ++cand_cnt;
    } else {
      if (it != cpyCand.begin()) {
        x /= (2.0 * ((double) cand_cnt));
        y /= (2.0 * ((double) cand_cnt));
        ++found_tracks;
        B2DEBUG(200, "  Track [ " << found_tracks << " ]: " << x << " " << y);
        n_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(x, y)));
      }
      x = hc.first.X();
      x += hc.second.X();
      y = hc.first.Y();
      y += hc.second.Y();
      cand_cnt = 1;
    }
    last_hash = hash;
    last_size = size;
    last_idList = idList;
  }

  if (cpyCand.size() > 0) {
    x /= (2.0 * ((double) cand_cnt));
    y /= (2.0 * ((double) cand_cnt));
    ++found_tracks;
    B2DEBUG(200, "  Track [ " << found_tracks << " ]: " << x << " " << y);
    n_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(x, y)));
  }

  found_tracks = 0;
  cpyCand = p_houghCand;
  sort(cpyCand.begin(), cpyCand.end());
  last_hash = 0;
  last_size = 0;
  cand_cnt = 0;
  x = 0.0;
  y = 0.0;
  B2DEBUG(200, "Tracks found in P-Side: " << cpyCand.size());
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    hash = it->getHash();
    size = it->getHitSize();
    if (((last_hash == hash && last_size == size)) && it != cpyCand.end()) {
      x += hc.first.X();
      x += hc.second.X();
      y += hc.first.Y();
      y += hc.second.Y();
      ++cand_cnt;
    } else {
      if (it != cpyCand.begin()) {
        x /= (2.0 * ((double) cand_cnt));
        y /= (2.0 * ((double) cand_cnt));
        ++found_tracks;
        B2DEBUG(200, "  -> Track [ " << found_tracks << " ]: " << x << " " << y);
        p_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(x, y)));
      }
      x = hc.first.X();
      x += hc.second.X();
      y = hc.first.Y();
      y += hc.second.Y();
      cand_cnt = 1;
    }
    B2DEBUG(200, "  Track Cand [ " << cand_cnt << " ]: " << x << " " << y << " Current value: " << (x / (2.0 * ((double) cand_cnt)))
            << " " << (y / (2.0 * ((double) cand_cnt))));
    last_hash = hash;
    last_size = size;
  }

  if (cpyCand.size() > 0) {
    x /= (2.0 * ((double) cand_cnt));
    y /= (2.0 * ((double) cand_cnt));
    ++found_tracks;
    B2DEBUG(200, "  Track [ " << found_tracks << " ]: " << x << " " << y);
    p_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(x, y)));
  }
}

/*
 * Purify tracks for both sides.
 */
void
SVDHoughtrackingModule::oldpurifyTrackCands()
{
  coord2dPair hc, hc2;
  vector<unsigned int> idList;
  unsigned int hash, size, cand_cnt, found_tracks;
  double x, y;
  vector<SVDHoughCand> cpyCand;

  found_tracks = 0;
  cpyCand = n_houghCand;
  B2DEBUG(2, "Tracks found in N-Side: " << cpyCand.size());
  //for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
  while (1) {
    auto it = cpyCand.begin() + found_tracks;
    B2DEBUG(2, " Size: " << cpyCand.size());
    if (cpyCand.size() == 0) {
      break;
    }
    idList = it->getIdList();
    hc = it->getCoord();
    hash = it->getHash();
    size = it->getHitSize();
    cand_cnt = 1;
    x = hc.first.X();
    x += hc.second.X();
    y = hc.first.Y();
    y += hc.second.Y();
    it = cpyCand.erase(it);
    B2DEBUG(2, "     $ " << cpyCand.size());
    auto end_it = cpyCand.end();
    for (auto it_in = it; it_in != cpyCand.end(); ++it_in) {
      B2DEBUG(2, "  *** Size: " << cpyCand.size() << " Compare Hash: " << it_in->getHash() << " with " << hash
              << " Compare size Hash: " << it_in->getHitSize() << " with " << size);
      if (hash == it_in->getHash() && size == it_in->getHitSize()) {
        B2DEBUG(2, "     $$ " << cpyCand.size());
        hc2 = it_in->getCoord();
        x += hc2.first.X();
        x += hc2.second.X();
        y += hc2.first.Y();
        y += hc2.second.Y();
        ++cand_cnt;
        if (cpyCand.size() > 0) {
          it_in = cpyCand.erase(it_in);
          B2DEBUG(2, "    Delete at " << cpyCand.size());
          if (it_in == cpyCand.end()) {
            B2DEBUG(2, "        !!! Would break at " << cpyCand.size());
            //break;
          }
        } else {
          B2DEBUG(2, "     $$$ " << cpyCand.size());
          break;
        }
      }
      if (it_in == cpyCand.end()) {
        B2DEBUG(2, "     $$$$end " << cpyCand.size());
      }
      if (it_in == end_it) {
        B2DEBUG(2, "     $$$$$foo " << cpyCand.size());
      }
    }
    B2DEBUG(2, "     $$$$$$ " << cpyCand.size());
    x /= (2.0 * ((double) cand_cnt));
    y /= (2.0 * ((double) cand_cnt));
    ++found_tracks;
    B2DEBUG(2, "  Track [ " << found_tracks << " ]: " << x << " " << y);
    n_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(x, y)));
  }

  found_tracks = 0;
  cpyCand = p_houghCand;
  B2DEBUG(2, "Tracks found in P-Side:");
  //for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
  while (1) {
    auto it = cpyCand.begin() + found_tracks;
    B2DEBUG(2, " Size: " << cpyCand.size());
    if (cpyCand.size() == 0) {
      break;
    }
    idList = it->getIdList();
    hc = it->getCoord();
    hash = it->getHash();
    size = it->getHitSize();
    cand_cnt = 1;
    x = hc.first.X();
    x += hc.second.X();
    y = hc.first.Y();
    y += hc.second.Y();
    it = cpyCand.erase(it);
    for (auto it_in = cpyCand.begin(); it_in != cpyCand.end() && cpyCand.size() > 0; ++it_in) {
      B2DEBUG(2, "  *** Size: " << cpyCand.size() << " Compare Hash: " << it_in->getHash() << " with " << hash
              << " Compare size Hash: " << it_in->getHitSize() << " with " << size);
      if (hash == it_in->getHash() && size == it_in->getHitSize()) {
        hc2 = it_in->getCoord();
        x += hc2.first.X();
        x += hc2.second.X();
        y += hc2.first.Y();
        y += hc2.second.Y();
        ++cand_cnt;
        if (cpyCand.size() > 0) {
          it_in = cpyCand.erase(it_in);
          if (it_in == cpyCand.end()) {
            B2DEBUG(2, "        !!! Would Break at " << cpyCand.size());
            //break;
          }
        } else {
          break;
        }
      }
    }
    x /= (2.0 * ((double) cand_cnt));
    y /= (2.0 * ((double) cand_cnt));
    ++found_tracks;
    B2DEBUG(2, "  Track [ " << found_tracks << " ]: " << x << " " << y);
    p_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(x, y)));
  }
}

void
SVDHoughtrackingModule::gplotSetLoad(FILE* fp, const char* option)
{
  if (fp == NULL) {
    fp = stdout;
  }

  fprintf(fp, "load '%s'\n", option);
}

void
SVDHoughtrackingModule::gplotSetOpt(FILE* fp, const char* option)
{
  if (fp == NULL) {
    fp = stdout;
  }

  fprintf(fp, "set %s\n", option);
}


void
SVDHoughtrackingModule::gplotInsert(FILE* fp, const char* dat, int what)
{
  switch (what) {
    case GPLOT_LINE:
      if (!gplot_first) {
        fprintf(fp, "%s", "re");
      }
      fprintf(fp, "%s", dat);
      gplot_first = 0;
      break;
    case GPLOT_INPLOT:
      if (gplot_first) {
        fprintf(stdout, "%s", "plot \"-\" notitle\n");

        gplot_first = 0;
      }

      if (dat != NULL) {
        fprintf(fp, "%s", dat);
      }
      break;
    case GPLOT_PLOTGEO:
      if (dat != NULL) {
        fprintf(fp, "%s", gplot_geo.c_str());
      }
      break;
  }
}

void
SVDHoughtrackingModule::gplotClose(FILE* fp, int what)
{
  if (fp == NULL) {
    fp = stdout;
  }

  switch (what) {
    case GPLOT_INPLOT:
      fprintf(fp, "%s%s", "\n", "pause -1\n");
      break;
  }

  if (fp != NULL) {
    fclose(fp);
  }

  gplot_first = 1;
}

/* vim: set tabstop=2:softtabstop=2:shiftwidth=2:noexpandtab */

