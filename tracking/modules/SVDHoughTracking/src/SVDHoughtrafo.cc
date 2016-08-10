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

/* Hack for re-plot */
//int gplot_first = 1;

/* Debug options */
bool createCoordList = false;

/*
 * Transform to Hough space
 */
void
SVDHoughTrackingModule::houghTrafo2d(svdClusterMap& mapClusters, bool p_side, bool conformal = false)
{
  int cnt;
  int strip_id;
  double r;
  svdClusterMapIter iter;
  svdClusterPair clusterInfo;
  TVector3 pos;
  TVector2 hough, center;
  VxdID sensorID;

  if (!p_side) {
    B2DEBUG(200, "Hough Transformation: N-Side");
  } else {
    B2DEBUG(200, "Hough Transformation: P-Side");
  }

  cnt = 0;
  center.Set(0.0, 0.0);
  for (iter = mapClusters.begin(); iter != mapClusters.end(); ++iter) {
    strip_id = iter->first;
    clusterInfo = iter->second;
    pos = clusterInfo.second;
    sensorID = clusterInfo.first;
    if (!p_side) {
      if (conformal) {
        r = sqrt(pow(pos.X() - center.X(), 2.0) + pow(pos.Z() - center.Y(), 2.0));
        hough.Set(pos.X() / (r * r), pos.Z() / (r * r));
      } else if (m_tbMapping) {
        hough.Set(pos.X(), pos.Z());
      } else if (m_projectionRecoN) {
        hough.Set(pos.Y(), pos.Z());
      } else {
        r = sqrt(pos.X() * pos.X() + pos.Y() * pos.Y());
        /*
        if (sensorID.getLayerNumber() == 3) {
          r = 3.8990;
        } else if (sensorID.getLayerNumber() == 4) {
          r = 8.0000;
        } else if (sensorID.getLayerNumber() == 5) {
          r = 10.4000;
        } else if (sensorID.getLayerNumber() == 6) {
          r = 13.5150;
        }
        */
        hough.Set(r, pos.Z());
      }
      //n_hough.insert(make_pair(cnt, make_pair(sensorID, hough)));
      //strip_id += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber(); // for testing purpose
      //B2INFO("n_side stripID: " << strip_id);
      n_hough.insert(make_pair(strip_id, make_pair(sensorID, hough)));    // for testing purpose
      B2DEBUG(200, "  Sensor: " << clusterInfo.first << " Position: "
              << hough.X() << " " << hough.Y());
    } else {
      if (conformal) {
        //rsqr = ((pow(pos.X() - center.X(), 2.0) + pow(pos.Y() - center.Y(), 2.0)));
        r = sqrt(pos.X() * pos.X() + pos.Y() * pos.Y());
        if (m_xyHoughPside) {
          //hough.Set(pos.X() / rsqr, pos.Y() / rsqr);
          hough.Set(pos.X() / (r * r), pos.Y() / (r * r));
        } else if (m_rphiHoughPside) {
          //hough.Set(pos.Phi() - M_PI / 2.0, sqrt(rsqr));
          hough.Set(pos.Phi() - M_PI / 2.0, r);
        }
      } else {
        hough.Set(pos.X(), pos.Y());
      }
      //p_hough.insert(make_pair(cnt, make_pair(sensorID, hough)));
      //strip_id += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber(); // for testing purpose
      //B2INFO("p_side stripID: " << strip_id);
      p_hough.insert(make_pair(strip_id, make_pair(sensorID, hough)));     // for testing purpose
      B2DEBUG(200, "  Sensor: " << clusterInfo.first << " Position: "
              << hough.X() << " " << hough.Y());
    }
    ++cnt;
  }

  /* Write hough space into gnuplot file */
  if (m_writeHoughSpace) {
    houghTrafoPlot(p_side);
  }
}



/*
 * Very simple sensor filter / layer filter / sensor layer filter.
 */
bool
SVDHoughTrackingModule::layerFilter(bool* layer, unsigned int minLines)
{
  unsigned int lcnt = 0;

  /* Count number of found layers */
  for (int i = 0; i < 4; ++i) {
    if (layer[i] == true) {
//    if (layer[i] == true && i != 3) {
      ++lcnt;
    }
  }
  // return true, if lines in sector are from at least minLines different layers
  if (lcnt >= minLines) {
    return (true);
  }

  return (false);
}


/*
 * Find the intercept in hough, or hess hough space.  // does "hess hough space" here mean "conformal mapped hough space"?
 * Return zero on success.
 * Parameter for p_side:
 * x = phi0
 * y = r
 * Parameter for n_side:
 * x = m
 * y = a
 */
int
SVDHoughTrackingModule::fastInterceptFinder2d(houghMap& hits, bool p_side, TVector2 v1_s,
                                              TVector2 v2_s, TVector2 v3_s, TVector2 v4_s, unsigned int iterations,
                                              unsigned int critIterations, unsigned int maxIterations,
                                              vector<houghDbgPair>& dbg_rect, unsigned int min_lines)
{
  int i, j, idx;
  int strip_id;
  int br = 0;
  unsigned int cnt_tracks;
  unsigned int dbg_start_iteration = 0;
  double unitx, unity;
  double y1, y2;
  double m, a;              // for n_side
  double r, phi0;           // for p_side
  houghPair hp;
  VxdID sensor;
  vector<unsigned int> cand_dx;

  // The vectors v1...v4 (and v1_s...v4_s) contain phi (rsp. theta) in first (meaning X), and d (or rho) (rsp. matching variable for z-direction) in second (meaning Y) coordinate
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
        strip_id = it->first;      // for testing purpose
        //if (p_side && i==0 && j==0 && it==hits.begin())  {B2INFO("p_side stripID: " << strip_id);}
        //if (!p_side && i==0 && j==0 && it==hits.begin()) {B2INFO("n_side stripID: " << strip_id);}
        hp = it->second;
        sensor = hp.first;

        if (!p_side) {
          m = hp.second.X();
          a = hp.second.Y();
          y1 = m * cos(v1.X()) + a * sin(v1.X());
          y2 = m * cos(v2.X()) + a * sin(v2.X());
        } else {
          if (m_xyHoughPside) {
            m = hp.second.X();
            a = hp.second.Y();
            y1 = 2 * (m * cos(v1.X()) + a * sin(v1.X()));
            y2 = 2 * (m * cos(v2.X()) + a * sin(v2.X()));
          } else if (m_rphiHoughPside) {
            phi0 = hp.second.X();
            r = hp.second.Y();
            y1 = 2.0 / r * sin(v1.X() - phi0);
            y2 = 2.0 / r * sin(v2.X() - phi0);
          }
        }

        //if (y1 > v1.y && y1 < v3.y || y2 > v1.y && y2 < v3.y ||
        //    y1 > v3.y && y2 < v1.y || y1 < v3.y && y2 > v1.y) {

        /* Check if HS-parameter curve is inside (or outside) actual sub-HS */
        if (!((y1 > v1.Y() && y2 > v2.Y()) || (y1 < v4.Y() && y2 < v3.Y()))) {
          if (/*cnt_tracks < 4 &&*/ iterations == critIterations) {
            //cand_dx.push_back(idx);
            cand_dx.push_back(strip_id);      // for testing purpose
          }

          //if (sensor.getLayerNumber() != 6) {                                   // testing of effects of 3-layer-tracking with different layer combinations
          layerHit[sensor.getLayerNumber() - 3] = true; /* layer filter */
          ++cnt_tracks;                                                       // testing of effects of 3-layer-tracking with different layer combinations
          //}

          if (iterations > 4) {
            //B2DEBUG(250, "    Hit id: " << idx << " Layer: " << sensor.getLayerNumber());
            B2DEBUG(250, "    Hit id: " << strip_id << " Layer: " << sensor.getLayerNumber());
          }
        }
      }
      if (cnt_tracks >= min_lines) {
        if (m_useSensorFilter
            && !layerFilter(layerHit,
                            min_lines)) {   // m_useSensorFilter corresponds to layerFilter, m_useSensorFilter=True means layerFilter is used
          B2DEBUG(250, "  Kicked out by sensor filter");
        } else {
          if (iterations >= dbg_start_iteration) {
            dbg_rect.push_back(make_pair(iterations, make_pair(v1, v3)));
          }

          // recursive / iterative call of fastInterceptFinder2d, until iterations = critIterations (critIterations-1), actual values for v1...v4 are new startingpoints
          if (iterations != critIterations && !br) {
            fastInterceptFinder2d(hits, p_side, v1, v2, v3, v4, iterations + 1, critIterations, maxIterations,
                                  dbg_rect, min_lines);
          } else {
            if (!p_side) {
              n_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v3)));      // v2 ersetzt durch v3, CWessel 05012016
              //for(int k = 0; k != cand_dx.size(); k++) { B2INFO("stripIDs of n_houghCand: " << cand_dx[k]); }

            } else {
              /* // my version
              p_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v3)));      // v2 ersetzt durch v3, CWessel 05012016
              */
              /* XXX: Curling detection */
              bool left = curlingDetection(p_clusters, cand_dx);
              if (m_useClusteringPurify) {
                p_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v4, v2), left));
              } else {
                //p_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v2), left));   // Michael's version
                p_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v3), left));
                //for(int k = 0; k != cand_dx.size(); k++) { B2INFO("stripIDs of p_houghCand: " << cand_dx[k]); }
              }
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
 * Find the intercept in hough, or hess hough space.  // does "hess hough space" here mean "conformal mapped hough space"?
 * Return zero on success.
 * Parameter for p_side:
 * x = phi0
 * y = r
 * Parameter for n_side:
 * x = m
 * y = a
 */
int
SVDHoughTrackingModule::slowInterceptFinder2d(houghMap& hits, bool p_side, vector<houghDbgPair>& dbg_rect, unsigned int min_lines)
{
  int i, j, idx;
  int strip_id;
  unsigned int cnt_tracks;
  double unitx, unity;
  double y1, y2;
  double m, a;              // for n_side
  double r, phi0;           // for p_side
  houghPair hp;
  VxdID sensor;
  vector<unsigned int> cand_dx;
  unsigned int angleSectors, vertSectors;
  double angleRange, vertRange;
  double left, right, up, down;

  // The vectors v1...v4 (and v1_s...v4_s) contain phi (rsp. theta) in first (meaning X), and d (or rho) (rsp. matching variable for z-direction) in second (meaning Y) coordinate
  TVector2 v1, v2, v3, v4;

  if (!p_side) {
    angleSectors = m_angleSectorsN;
    vertSectors = m_vertSectorsN;
    left = m_rectXN1;
    right = m_rectXN2;
    up = m_rectSizeN;
    down = -1.0 * m_rectSizeN;
  } else {
    angleSectors = m_angleSectorsP;
    vertSectors = m_vertSectorsP;
    left = m_rectXP1;
    right = m_rectXP2;
    up = m_rectSizeP;
    down = -1.0 * m_rectSizeP;
  }

  angleRange = (right - left);
  vertRange = (up - down);
  unitx = angleRange / (double)(angleSectors);
  unity = vertRange / (double)(vertSectors);

  cnt_tracks = 0;
  // outer loop for angular range
  for (i = 0; i < (int)angleSectors ; ++i) {
    // inner loop for vertical range
    for (j = 0; j < (int)vertSectors; ++j) {
      v1.Set((left + (double) i * unitx), (up - (double) j * unity));
      v2.Set((left + (double) i * unitx + unitx), (up - (double) j * unity));
      v3.Set((left + (double) i * unitx + unitx), (up - (double) j * unity - unity));
      v4.Set((left + (double) i * unitx), (up - (double) j * unity - unity));

      cand_dx.clear();
      bool layerHit[4] = {false}; /* For layer filter */
      //for (k = 0; k < hits.size(); ++k) {
      for (auto it = hits.begin(); it != hits.end(); ++it) {
        idx = it->first;
        strip_id = it->first;      // for testing purpose
        hp = it->second;
        sensor = hp.first;

        if (!p_side) {
          m = hp.second.X();
          a = hp.second.Y();
          y1 = m * cos(v1.X()) + a * sin(v1.X());
          y2 = m * cos(v2.X()) + a * sin(v2.X());
        } else {
          if (m_xyHoughPside) {
            m = hp.second.X();
            a = hp.second.Y();
            y1 = 2 * (m * cos(v1.X()) + a * sin(v1.X()));
            y2 = 2 * (m * cos(v2.X()) + a * sin(v2.X()));
          } else if (m_rphiHoughPside) {
            phi0 = hp.second.X();
            r = hp.second.Y();
            y1 = 2.0 / r * sin(v1.X() - phi0);
            y2 = 2.0 / r * sin(v2.X() - phi0);
          }
        }

        /* Check if HS-parameter curve is inside (or outside) actual sub-HS */
        if (!((y1 > v1.Y() && y2 > v2.Y()) || (y1 < v4.Y() && y2 < v3.Y()))) {
          //      if (cnt_tracks >= min_lines) {
          cand_dx.push_back(idx);
          //cand_dx.push_back(strip_id);      // for testing purpose
          //      }

          layerHit[sensor.getLayerNumber() - 3] = true; /* layer filter */
          ++cnt_tracks;
        }
      }
      if (cnt_tracks >= min_lines) {
        if (m_useSensorFilter
            && !layerFilter(layerHit,
                            min_lines)) {               // m_useSensorFilter corresponds to layerFilter, m_useSensorFilter=True means layerFilter is used
          B2DEBUG(250, "  Kicked out by sensor filter");
        } else {
          /*
          if (iterations >= dbg_start_iteration) {
            dbg_rect.push_back(make_pair(iterations, make_pair(v1, v3)));
          }
          */
          dbg_rect.push_back(make_pair(5, make_pair(v1, v3)));

          if (!p_side) {
            n_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v3)));      // v2 ersetzt durch v3, CWessel 05012016
          } else {
            /* // my version
            p_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v3)));      // v2 ersetzt durch v3, CWessel 05012016
            */
            /* XXX: Curling detection */
            bool left = curlingDetection(p_clusters, cand_dx);
            if (m_useClusteringPurify) {
              p_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v4, v2), left));
            } else {
              p_houghCand.push_back(SVDHoughCand(cand_dx, make_pair(v1, v2), left));
            }
          }
          //B2DEBUG(250, "  Add candidate: (" << v1.X() << ", " << v1.Y() << ") (" << v3.X() << ", " << v3.Y() << ") V3_s: " << v3_s.Y());
        }
      }

      cnt_tracks = 0;
    }
  }

  return 0;
}

/*
 * Curling detection.
 */
bool
SVDHoughTrackingModule::curlingDetection(svdClusterMap& hits, vector<unsigned int>& ids)
{
  svdClusterPair cp;
  VxdID sensor;
  TVector3 c1, c2;
  bool found_l3 = false;
  bool found_l6 = false;

  for (auto iter = ids.begin(); iter != ids.end(); ++iter) {
    svdClusterMap::iterator hit = hits.find(*iter);
    cp = hit->second;
    sensor = cp.first;
    if (sensor.getLayerNumber() == 3) {
      c1 = cp.second;
      found_l3 = true;
      if (found_l6) {
        break;
      }
    }
    if (sensor.getLayerNumber() == 6) {
      c2 = cp.second;
      found_l6 = true;
      if (found_l3) {
        break;
      }
    }
    if (sensor.getLayerNumber() == 4 && !found_l3) {
      c1 = cp.second;
    }
    if (sensor.getLayerNumber() == 5 && !found_l6) {
      c2 = cp.second;
    }
  }

  if (c1.DeltaPhi(c2) >= 0.0) {
    //B2INFO("  ----> Bla true: " << c1.DeltaPhi(c2));
    return true;
  } else {
    //B2INFO("  ----> Bla false: " << c1.DeltaPhi(c2));
    return false;
  }

}



/*
 * List Hough Candidates.
 */
void
SVDHoughTrackingModule::printHoughCandidates()
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
SVDHoughTrackingModule::printTrackCandidates()
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
