/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/SVDHoughTracking/SVDHoughTrackingModule.h>
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


/*
 * Wrapper for the purify tracks. Here you can select what kind of method you want to use.
 */
void
SVDHoughTrackingModule::purifyTrackCands()
{
  if (m_useClusteringPurify) {
    /* Use FPGA like Clustering of HS */
    purifyTrackCandsClustering();
  } else if (m_useEmptyPurify) {
    convertHoughCandToHoughTrackCand();
  } else {
    /* Compare the whole list */
    purifyTrackCandsList();
  }
}

/*
 * Clustering of HS to merge the sectors.
 */
void
SVDHoughTrackingModule::purifyTrackCandsClusteringCW()
{
  coord2dPair hc, hc2;
  vector<unsigned int> idList, last_idList, merged_idList;
  unsigned int cnt;
  vector<SVDHoughCand> cpyCand;
  TVector2 mid, mid2, merged;

  cpyCand = p_houghCand;
  //sort(cpyCand.begin(), cpyCand.end());
  cnt = 0;                                  // 25012016
//  x = 0.0;
//  last_x = 0;
//  y = 0.0;
//  auto it = cpyCand.begin();
//  hc = it->getCoord();
//  step_size_x = hc.second.X() - hc.first.X();
//  step_size_y = hc.second.Y() - hc.first.Y();

  //B2INFO("--> Debug Clustering: " << cpyCand.size() << " <--");
  //B2INFO("  Step size x: " << step_size_x << " y: " << step_size_y);


  /* Merging process for p-side */

  merged.Set(0.0, 0.0);                     //25012016

redo_p:
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    mid.Set((hc.first.X() + hc.second.X()) / 2.0, (hc.first.Y() + hc.second.Y()) / 2.0);
    //B2INFO("    v1: (" << hc.first.X() << ", " << hc.first.Y() << ")  v2: (" << hc.second.X() << ", " << hc.second.Y() << ")");
    if (it != cpyCand.end()) {
      for (auto it_in = it + 1; it_in != cpyCand.end(); ++it_in) {
        hc2 = it_in->getCoord();
        mid2.Set((hc2.first.X() + hc2.second.X()) / 2.0, (hc2.first.Y() + hc2.second.Y()) / 2.0);
        //B2INFO("      Compare with v1: (" << hc2.first.X() << ", " << hc2.first.Y() << ")  v2: (" << hc2.second.X() << ", " << hc2.second.Y() << ")");
        if (hc.first.X() == hc2.first.X() || hc.first.X() == hc2.second.X() || hc.second.X() == hc2.first.X()
            || hc.second.X() == hc2.second.X()) {
          //B2INFO("        --> Merging...");
          //merged.Set((mid.X() + mid2.X()) / 2.0, (mid.Y() + mid2.Y()) / 2.0);                                                                                                         // 25012016
          merged.Set(((double)cnt * merged.X() + (mid.X() + mid2.X()) / 2.0) / ((double)cnt + 1.0) ,
                     ((double)cnt * merged.Y() + (mid.Y() + mid2.Y()) / 2.0)  / ((double)cnt + 1.0));      // 25012016
          cnt++;
          cpyCand.erase(it_in);
          cpyCand.erase(it);
          cnt = 0;          // Zum Testen eingefügt, keine Ahnung ob das hier Sinn macht, 09032016
          goto redo_p;
        }
      }
      /**
       * For me it doesn't make much sense to put an empty "merged_idList" here,
       * so I put the original idList into it (16.12.2015)
       */
      //p_houghTrackCand.push_back(SVDHoughTrackCand(merged_idList, merged));
      p_houghTrackCand.push_back(SVDHoughTrackCand(idList, merged));
      cpyCand.erase(it);
      cnt = 0;          // Zum Testen eingefügt, keine Ahnung ob das hier Sinn macht, 09032016
      goto redo_p;
    } else {
    }
  }

  /* Merging process for n-side */

  cpyCand = n_houghCand;
  //sort(cpyCand.begin(), cpyCand.end());
  cnt = 0;

  merged.Set(0.0, 0.0);                     //25012016

redo_n:
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    mid.Set((hc.first.X() + hc.second.X()) / 2.0, (hc.first.Y() + hc.second.Y()) / 2.0);
    //B2INFO("    v1: (" << hc.first.X() << ", " << hc.first.Y() << ")  v2: (" << hc.second.X() << ", " << hc.second.Y() << ")");
    if (it != cpyCand.end()) {
      for (auto it_in = it + 1; it_in != cpyCand.end(); ++it_in) {
        hc2 = it_in->getCoord();
        mid2.Set((hc2.first.X() + hc2.second.X()) / 2.0, (hc2.first.Y() + hc2.second.Y()) / 2.0);
        //B2INFO("      Compare with v1: (" << hc2.first.X() << ", " << hc2.first.Y() << ")  v2: (" << hc2.second.X() << ", " << hc2.second.Y() << ")");
        if (hc.first.X() == hc2.first.X() || hc.first.X() == hc2.second.X() || hc.second.X() == hc2.first.X()
            || hc.second.X() == hc2.second.X()) {
          //B2INFO("        --> Merging...");
          //merged.Set((mid.X() + mid2.X()) / 2.0, (mid.Y() + mid2.Y()) / 2.0);                                                                                                         // 25012016
          merged.Set(((double)cnt * merged.X() + (mid.X() + mid2.X()) / 2.0) / ((double)cnt + 1.0) ,
                     ((double)cnt * merged.Y() + (mid.Y() + mid2.Y()) / 2.0)  / ((double)cnt + 1.0));      // 25012016
          cnt++;
          cpyCand.erase(it_in);
          cpyCand.erase(it);
          cnt = 0;          // Zum Testen eingefügt, keine Ahnung ob das hier Sinn macht, 09032016
          goto redo_n;
        }
      }
      /**
       * For me it doesn't make much sense to put an empty "merged_idList" here,
       * so I put the original idList into it (16.12.2015)
       */
      //n_houghTrackCand.push_back(SVDHoughTrackCand(merged_idList, merged));
      n_houghTrackCand.push_back(SVDHoughTrackCand(idList, merged));
      cpyCand.erase(it);
      cnt = 0;          // Zum Testen eingefügt, keine Ahnung ob das hier Sinn macht, 09032016
      goto redo_n;
    } else {
    }
  }


}


/*
 * Clustering of HS to merge the sectors.
 */
void
SVDHoughTrackingModule::purifyTrackCandsClustering()
{
  coord2dPair hc, hc2;
  vector<unsigned int> idList, last_idList, merged_idList;
  double step_size_x, step_size_y;
  vector<SVDHoughCand> cpyCand;
  TVector2 mid, mid2, merged;
  bool debug = true;

  ofstream os, os_edges;


  /** Begin of Clustering Process for p-side */

  if (debug) {
    os.open("dbg/clusters_p.dat", ofstream::out);
    os_edges.open("dbg/edges_p.dat", ofstream::out);
  }

  cpyCand = p_houghCand;
  //sort(cpyCand.begin(), cpyCand.end());
  if (cpyCand.size() > 0) {
    auto it = cpyCand.begin();
    hc = it->getCoord();
    step_size_x = hc.second.X() - hc.first.X();
    //step_size_y = hc.second.Y() - hc.first.Y();
    step_size_y = hc.first.Y() - hc.second.Y();
  } else {
    return;
  }

  //B2INFO(200, "--> Debug Clustering: " << cpyCand.size() << " <--");
  //B2INFO(200, "  Step size x: " << step_size_x << " y: " << step_size_y);
  B2INFO("--> Debug Clustering: " << cpyCand.size() << " <--");
  B2INFO("  Step size x: " << step_size_x << " y: " << step_size_y);

  while (cpyCand.size() > 0) {
    SVDHoughClusterCand cur_hcl;
    auto it = cpyCand.begin();
    hc = it->getCoord();
    cur_hcl.setMaxSize(m_maxClusterSizeX, m_maxClusterSizeY);
    cur_hcl.setSize(1);
    cur_hcl.setCenter(TVector2((hc.first.X() + hc.second.X()) / 2.0, (hc.first.Y() + hc.second.Y()) / 2.0));
    cur_hcl.setPitch(TVector2(step_size_x / 2.0, step_size_y / 2.0));
    cpyCand.erase(it);
    iterCluster(&cpyCand, &cur_hcl, hc);
    //cur_hcl.setCenter(TVector2(cur_hcl.getCenter().X() / cur_hcl.getSize(),
    //                          cur_hcl.getCenter().Y() / cur_hcl.getSize()));

    /* Add cluster and filter out <x> cluster sizes */
    if (cur_hcl.getSize() > m_minHoughClusters) {
      unsigned int size = cur_hcl.getSize();
      TVector2 center = cur_hcl.getGravity();
      p_houghTrackCand.push_back(SVDHoughTrackCand(merged_idList, center));
      B2DEBUG(200, "  Cluster: " << size << " Coord: " << center.X() << " " << center.Y() << "  Total cluster size: "
              << cur_hcl.getSizeX() << " " << cur_hcl.getSizeY());
      if (debug) {
        TVector2 left_edge, right_edge;
        left_edge = cur_hcl.getLeftEdge() - cur_hcl.getPitch();
        right_edge = cur_hcl.getRightEdge() + cur_hcl.getPitch();

        /* Print center */
        os <<  center.X() << " " << center.Y() << endl;
        os_edges << "set object rect from " << left_edge.X() << ", " << left_edge.Y() << " to "
                 << right_edge.X() << ", " << right_edge.Y();
        os_edges << " fc rgb \"" << "green" << "\" fs solid 0.5 behind" << endl;
      }
    }
  }

  if (debug) {
    os.close();
    os_edges.close();
  }

  /** End of Clustering Process for p-side */


  /** Begin of Clustering Process for n-side */

  if (debug) {
    os.open("dbg/clusters_n.dat", ofstream::out);
    os_edges.open("dbg/edges_n.dat", ofstream::out);
  }

  cpyCand = n_houghCand;
  //sort(cpyCand.begin(), cpyCand.end());
  if (cpyCand.size() > 0) {
    auto it = cpyCand.begin();
    hc = it->getCoord();
    step_size_x = hc.second.X() - hc.first.X();
    //step_size_y = hc.second.Y() - hc.first.Y();
    step_size_y = hc.first.Y() - hc.second.Y();
  } else {
    return;
  }

  //B2INFO(200, "--> Debug Clustering: " << cpyCand.size() << " <--");
  //B2INFO(200, "  Step size x: " << step_size_x << " y: " << step_size_y);
  B2INFO("--> Debug Clustering: " << cpyCand.size() << " <--");
  B2INFO("  Step size x: " << step_size_x << " y: " << step_size_y);

  while (cpyCand.size() > 0) {
    SVDHoughClusterCand cur_hcl;
    auto it = cpyCand.begin();
    hc = it->getCoord();
    cur_hcl.setMaxSize(m_maxClusterSizeX, m_maxClusterSizeY);
    cur_hcl.setSize(1);
    cur_hcl.setCenter(TVector2((hc.first.X() + hc.second.X()) / 2.0, (hc.first.Y() + hc.second.Y()) / 2.0));
    cur_hcl.setPitch(TVector2(step_size_x / 2.0, step_size_y / 2.0));
    cpyCand.erase(it);
    iterCluster(&cpyCand, &cur_hcl, hc);
    //cur_hcl.setCenter(TVector2(cur_hcl.getCenter().X() / cur_hcl.getSize(),
    //                          cur_hcl.getCenter().Y() / cur_hcl.getSize()));

    // Add cluster and filter out <x> cluster sizes
    if (cur_hcl.getSize() > m_minHoughClusters) {
      unsigned int size = cur_hcl.getSize();
      TVector2 center = cur_hcl.getGravity();
      n_houghTrackCand.push_back(SVDHoughTrackCand(merged_idList, center));
      B2DEBUG(200, "  Cluster: " << size << " Coord: " << center.X() << " " << center.Y() << "  Total cluster size: "
              << cur_hcl.getSizeX() << " " << cur_hcl.getSizeY());
      if (debug) {
        TVector2 left_edge, right_edge;
        left_edge = cur_hcl.getLeftEdge() - cur_hcl.getPitch();
        right_edge = cur_hcl.getRightEdge() + cur_hcl.getPitch();

        // Print center
        os <<  center.X() << " " << center.Y() << endl;
        os_edges << "set object rect from " << left_edge.X() << ", " << left_edge.Y() << " to "
                 << right_edge.X() << ", " << right_edge.Y();
        os_edges << " fc rgb \"" << "green" << "\" fs solid 0.5 behind" << endl;
      }
    }
  }

  if (debug) {
    os.close();
    os_edges.close();
  }


  /** End of Clustering Process for n-side */
}

/*
 * Clustering of HS to merge the sectors.
 */
unsigned int
SVDHoughTrackingModule::iterCluster(vector<SVDHoughCand>* cands, SVDHoughClusterCand* hcl, coord2dPair hc)
{
  vector<unsigned int> idList;
  coord2dPair hc2;
  TVector2 mid;

  if (cands->size() == 0) {
    return 0;
  }

  for (auto it = cands->begin(); it < cands->end(); ++it) {
    idList = it->getIdList();
    hc2 = it->getCoord();
    mid.Set((hc2.first.X() + hc2.second.X()) / 2.0, (hc2.first.Y() + hc2.second.Y()) / 2.0);

    if (hitsEqual(hc, hc2)) {
      if (hcl->checkClusterSize(mid)) {
        B2DEBUG(200, "        --> Merging... middle point " << mid.X() << " " << mid.Y() << " in current cluster "
                << hcl->getCenter().X() << " " << hcl->getCenter().Y());
        hcl->incSize();
        hcl->addCenter(mid);
        hcl->printEdges();
        cands->erase(it);
        iterCluster(cands, hcl, hc2);
        return 1;
      } else {
      }
    }
  }
  return 0;
}

/*
 * Check for equal hits in coordPairs
 */
bool
SVDHoughTrackingModule::hitsEqual(coord2dPair& hc1, coord2dPair& hc2)
{
  TVector2 f1, f2;
  TVector2 s1, s2;

  f1 = hc1.first;
  f2 = hc1.second;
  s1 = hc2.first;
  s2 = hc2.second;

  /*if ((f1.X() - s1.X()) < epsilon || (f1.X() - s2.X()) < epsilon || (f2.X() - s1.X()) < epsilon
        || (f2.X() - s2.X()) < epsilon) {
    if ((f1.Y() - s1.Y()) < epsilon || (f1.Y() - s2.Y()) < epsilon || (f2.Y() - s1.Y()) < epsilon
        || (f2.Y() - s2.Y()) < epsilon) {
      return true;
    }
  }*/

  if (f1.X() == s1.X() || f1.X() == s2.X() || f2.X() == s1.X() || f2.X() == s2.X()) {
    if (f1.Y() == s1.Y() || f1.Y() == s2.Y() || f2.Y() == s1.Y() || f2.Y() == s2.Y()) {
      return true;
    }
  }

  return false;
}


/*
 * Empty purifier function only converting x_houghCand into x_houghTrackCand
 */
void
SVDHoughTrackingModule::convertHoughCandToHoughTrackCand()
{
  coord2dPair hc;
  vector<unsigned int> idList;
  double x, y;
  vector<SVDHoughCand> cpyCand;

  /** Start Clustering for n-side */

  cpyCand = n_houghCand;
  x = 0.0;
  y = 0.0;
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    //x = (hc.first.X() + hc.second.X()) / 2.0;
    //y = (hc.first.Y() + hc.second.Y()) / 2.0;
    //n_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(x, y)));
    n_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(hc.first.X(), hc.first.Y())));
    n_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(hc.second.X(), hc.second.Y())));

  }

  /** End Clustering n-side */

  /** Start Clustering p-side */

  cpyCand = p_houghCand;
  x = 0.0;
  y = 0.0;
  for (auto it = cpyCand.begin(); it != cpyCand.end(); ++it) {
    idList = it->getIdList();
    hc = it->getCoord();
    //x = (hc.first.X() + hc.second.X()) / 2.0;
    //y = (hc.first.Y() + hc.second.Y()) / 2.0;
    //p_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(x, y)));
    p_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(hc.first.X(), hc.first.Y())));
    p_houghTrackCand.push_back(SVDHoughTrackCand(idList, TVector2(hc.second.X(), hc.second.Y())));
  }

  /** End Clustering p-side */
}


/*
 * Hash purify tracks function for both sides. Using the complete
 * track ID list for comparison.
 */
void
SVDHoughTrackingModule::purifyTrackCandsList()
{
  coord2dPair hc, hc2;
  vector<unsigned int> idList, last_idList, merged_idList;
  unsigned int cand_cnt, found_tracks;
  double x, y, last_x;
  double x_tolerance = 1.0;
  vector<SVDHoughCand> cpyCand;
  bool useMerger = false;

  //m_countStrips = true;

  /** Start Clustering for n-side */

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

  /** End Clustering n-side */

  /** Start Clustering p-side */

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

  /** End Clustering p-side */

  //m_countStrips = false;

}

/*
 * Function to compare the hit list. Different options to return true
 * possible: exact match, contains etc...
 */
bool
SVDHoughTrackingModule::compareList(std::vector<unsigned int>& a, std::vector<unsigned int>& b)
{
  unsigned int cnt = 0;
  bool exact_match = false;
  bool contains = true;
  bool found;
  int sensorID_a, sensorID_b;
  int idx_a, idx_b;

  /* If exact match and size not equal return false */
  if (m_countStrips) {
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

    if (contains && cnt >= m_minimumLines) {
      return (true);
    } else {
      return (false);
    }

  } else {
    /*  // code that should work and should be used
    if (a.size() == b.size()) {
      for (auto it = a.begin(); it != a.end(); ++it) {
        found = false;
        for (auto it_in = b.begin(); it_in != b.end(); ++it_in) {
          //sensorID_a = (*it) / 1000;
          //sensorID_b = (*it_in) / 1000;
          sensorID_a = (*it) / 10000000;
          sensorID_b = (*it_in) / 10000000;
          if (sensorID_a == sensorID_b) {
            found = true;
            ++cnt;
          }
        }
      }
    }

    if (!(cnt >= m_minimumLines)) {
      return (false);
    }
    */

    // code for testing purpose
    if (exact_match && a.size() != b.size()) {
      return (false);
    }

    for (auto it = a.begin(); it != a.end(); ++it) {
      found = false;
      for (auto it_in = b.begin(); it_in != b.end(); ++it_in) {
        //if (((*it_in) / 10000000) == ((*it) / 10000000)) {
        idx_a = (unsigned int)(*it)    - (((unsigned int)(*it)    / 10) * 10);
        idx_b = (unsigned int)(*it_in) - (((unsigned int)(*it_in) / 10) * 10);
        if (idx_a == idx_b) {
          found = true;
          ++cnt;
        }
      }
      if (!contains && !found) {
        return (false);
      }
    }

    if (contains && cnt >= m_minimumLines) {
      return (true);
    } else {
      return (false);
    }

  }

  return (true);
}

/*
 * Merge Id lists.
 */
void
SVDHoughTrackingModule::mergeIdList(std::vector<unsigned int>& merged, std::vector<unsigned int>& a,
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
