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
//#include <tracking/modules/svdHoughtracking/basf2_tracking.h>
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/GeoVXDPosition.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/modules/mcTrackCandClassifier/MCTrackCandClassifierModule.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/IdealCDCGeometryTranslator.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/FieldManager.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/MaterialEffects.h>
#include <genfit/MeasurementFactory.h>
#include <genfit/RKTrackRep.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <geometry/GeometryManager.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/SVDHoughCluster.h>
#include <tracking/dataobjects/SVDHoughTrack.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <root/TMath.h>
#include <root/TGeoMatrix.h>
#include <root/TRandom.h>
#include <time.h>

#include <TH1F.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGeoManager.h>
#include <TDatabasePDG.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


/*
 * Run the full tracking pipeline.
 */
void
SVDHoughTrackingModule::trackingPipeline()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  TVector2 v1_s, v2_s, v3_s, v4_s;
  double rect_size;
  vector<houghDbgPair> n_rect, p_rect;
  vector<houghDbgPair> n_rectX, n_rectY;
  clock_t start, end;
  bool countStrips = m_countStrips;

  if (!storeHoughTrack.isValid()) {
    storeHoughTrack.create();
  } else {
    storeHoughTrack.getPtr()->Clear();
  }

  /* For testbeam only !! */
  /*  if (m_tbMapping) {
      if (n_clusters.size() >= 12 || p_clusters.size() >= 12) {
        return;
      }
    }
  */
  /* Start time measurement */
  start = clock();

  /* Hough transformation */
  houghTrafo2d(n_clusters, false,
               m_conformalTrafoN);     /* in z, no Hess transformation */   // better: no conformal transformation in z
  houghTrafo2d(p_clusters, true, m_conformalTrafoP);    /* in y, with Hess */      // better: with conformal transformation in y

  /*
   * Run hough tracking on P-Side
   */
  if (m_independentSectors) {
    slowInterceptFinder2d(p_hough, true, p_rect, m_minimumLines);
  } else {
    /* Initial rect size parameters for Hough trafo */
    rect_size = m_rectSizeP;
    /* Set start values */
    v1_s.Set(m_rectXP1, (rect_size));
    v2_s.Set(m_rectXP2, (rect_size));
    v3_s.Set(m_rectXP2, (-1.0 * rect_size));
    v4_s.Set(m_rectXP1, (-1.0 * rect_size));
    /* Run intercept finder */
    fastInterceptFinder2d(p_hough, true, v1_s, v2_s, v3_s, v4_s, 0, m_critIterationsP, m_maxIterationsP, p_rect, m_minimumLines);
  }

  /* Debug */
  if (m_writeHoughSectors) {
    gplotRect("dbg/p_rect.plot", p_rect);
  }

  /*
   * Run hough tracking on N-Side
   */
  if (m_independentSectors) {
    slowInterceptFinder2d(n_hough, false, n_rect, m_minimumLines);
  } else {
    /* Initial rect size parameters for Hough trafo */
    rect_size = m_rectSizeN;
    /* Set start values */
    v1_s.Set(m_rectXN1, (rect_size));
    v2_s.Set(m_rectXN2, (rect_size));
    v3_s.Set(m_rectXN2, (-1.0 * rect_size));
    v4_s.Set(m_rectXN1, (-1.0 * rect_size));
    /* Run intercept finder */
    fastInterceptFinder2d(n_hough, false, v1_s, v2_s, v3_s, v4_s, 0, m_critIterationsN, m_maxIterationsN, n_rect, m_minimumLines);
  }

  /* Debug */
  if (m_writeHoughSectors) {
    gplotRect("dbg/n_rect.plot", n_rect);
  }

  /* Print Hough list (when debug level >= 2) */
  printHoughCandidates();

  /* Purify candidates (when debug level >= 2) */
  if (m_usePurifier) {
    m_countStrips = true;
    purifyTrackCands();
    m_countStrips = countStrips;
  }
  printTrackCandidates(); /* Print track candidates */

  /* Run track merger */
  if (m_useTrackMerger) {
    trackMerger();
  }

  /* Find and combine compatible sets and create real tracks */
  fac3d();

  /* End of time measurement */
  end = clock();
  totClockCycles += 1000.0 * (((double)((end - start))) / ((double)((CLOCKS_PER_SEC))));

  /* Print tracks */
  printTracks();
}


/**
 * Estimate Curvature of the track
 */
int SVDHoughTrackingModule::CurvatureEstimation(vector<unsigned int>& p_idList)
{
  VxdID sensorID;
  houghPair hp;
  unsigned int strip_id;
  vector<double> phiarray;
  vector<double> deltaphi;
  //int i = 0;
  double phidiff = 0.;
  int posdiff = 0, negdiff = 0;
  int curvsign = 0;

  // for(int i = p_idList.begin(); i < p_idList.end(); i++) {
  for (auto it_idList = p_idList.begin(); it_idList != p_idList.end(); it_idList++) {
    for (auto iter = p_hough.begin(); iter != p_hough.end(); iter++) {
      //idx = iter->first;
      strip_id = iter->first;
      hp = iter->second;
      sensorID = hp.first;

      if (strip_id == *it_idList) {
        if (m_xyHoughPside) {
          double x = hp.second.X();
          double y = hp.second.Y();
          phiarray.push_back(atan2(y, x));
        } else if (m_rphiHoughPside) {
          phiarray.push_back(hp.second.X());
        }
        break;
      }
    }
  }

  for (auto phiarrayiter = phiarray.begin(); phiarrayiter != phiarray.end(); phiarrayiter++) {
    phidiff = (*(phiarrayiter + 1)) - (*phiarrayiter);
    if (phidiff > M_PI) {
      deltaphi.push_back(phidiff - 2.0 * M_PI);
    } else if (phidiff < -1.0 * M_PI) {
      deltaphi.push_back(phidiff + 2.0 * M_PI);
    } else {
      deltaphi.push_back(phidiff);
    }
  }

  for (auto deltaiter = deltaphi.begin(); deltaiter != deltaphi.end(); deltaiter++) {
    if ((*deltaiter) > 0.) {
      posdiff++;
    } else if ((*deltaiter) < 0.) {
      negdiff++;
    }
  }

  if (posdiff > negdiff) {
    curvsign = +1;      // corresponds to negatively charged particles
  } else if (negdiff > posdiff) {
    curvsign = -1;      // corresponds to positively charged particles
  }

  return curvsign;
}

/**
 * Analytically calculate intersection of HS lines for continuous angular and radial values
 */
void SVDHoughTrackingModule::AnalyticalIntersection(vector<unsigned int>& idList, bool p_side, vector<double>& result)
{
  VxdID sensorID;
  houghMap hough;
  houghPair hp;
  unsigned int strip_id;
  double x1 = 0., x2 = 0.;
  double y1 = 0., y2 = 0.;
  double angle = 0., d = 0.;
  double r = 0., rsum = 0., rsqr = 0.;
  double derivation1 = 0., derivation2 = 0.;
  double sigmaangle = 0., sigmad = 0.;
  double anglesum = 0., dsum = 0.;
  double anglesqr = 0., dsqr = 0.;
  vector<bool> layerFlag(4, false);
  vector<bool> calculatedIntersections(6, false);
  bool used = false;
  int layerCount = 0;
  int intersectionCount = 0;

  if (p_side) {
    hough = p_hough;
  } else {
    hough = n_hough;
  }

  // for(int i = p_idList.begin(); i < p_idList.end(); i++) {
  for (auto it_idList = idList.begin(); it_idList != idList.end() - 1; it_idList++) {

    for (auto it = layerFlag.begin(); it != layerFlag.end(); it++) {
      *it = false;
    }
    layerCount = 0;
    used = false;

    for (auto iter = hough.begin(); iter != hough.end(); iter++) {
      //idx = iter->first;
      strip_id = iter->first;
      hp = iter->second;
      sensorID = hp.first;

      if ((strip_id == (*it_idList)) && layerFlag[sensorID.getLayerNumber() - 3] == false) {
        //B2INFO("sensorID: " << sensorID << " sensorID.getLayerNumber(): " << sensorID.getLayerNumber());
        layerFlag[sensorID.getLayerNumber() - 3] = true;
        layerCount++;
        if (!p_side) {
          x1 = hp.second.X();
          y1 = hp.second.Y();
        } else {
          if (m_xyHoughPside) {
            x1 = hp.second.X();
            y1 = hp.second.Y();
          } else if (m_rphiHoughPside) {
            double phi = hp.second.X();
            double r = hp.second.Y();
            x1 = r * cos(phi);
            y1 = r * sin(phi);
          }
        }
        break;
      }
    }

    for (auto iter = hough.begin(); iter != hough.end(); iter++) {
      //idx = iter->first;
      strip_id = iter->first;
      hp = iter->second;
      sensorID = hp.first;

      if (strip_id == *(it_idList + 1) && !layerFlag[sensorID.getLayerNumber() - 3]) {
        layerFlag[sensorID.getLayerNumber() - 3] = true;
        layerCount++;
        if (!p_side) {
          x2 = hp.second.X();
          y2 = hp.second.Y();
        } else {
          if (m_xyHoughPside) {
            x2 = hp.second.X();
            y2 = hp.second.Y();
          } else if (m_rphiHoughPside) {
            double phi = hp.second.X();
            double r = hp.second.Y();
            x1 = r * cos(phi);
            y1 = r * sin(phi);
          }
        }
        break;
      }
    }


    angle = atan((x2 - x1) / (y1 - y2));
    derivation1 = y1 * cos(angle) - x1 * sin(angle);
    derivation2 = y2 * cos(angle) - x2 * sin(angle);

    /** only compute intersections if slope of HS curve is the same because this is required for "real" possible hits
     * this also should then reduce the number of background hits / tracks while the number of "real" tracks should
     * remain unchanged -> should yield higher DRF in the end because with less background tracks / fake tracks
     * less extrapolated hits are calculated and thus less ROI
     */
    if (!((derivation1 > 0. && derivation2 > 0.) || (derivation1 < 0. && derivation2 < 0.))) {
      continue;
    }

    /** Only take one intersection between two points / lines of two different layers */
    if (layerFlag[0] && layerFlag[1] && !calculatedIntersections[0]) {
      calculatedIntersections[0] = true;
      used = true;
    } else if (layerFlag[0] && layerFlag[2] && !calculatedIntersections[1]) {
      calculatedIntersections[1] = true;
      used = true;
    } else if (layerFlag[0] && layerFlag[3] && !calculatedIntersections[2]) {
      calculatedIntersections[2] = true;
      used = true;
    } else if (layerFlag[1] && layerFlag[2] && !calculatedIntersections[3]) {
      calculatedIntersections[3] = true;
      used = true;
    } else if (layerFlag[1] && layerFlag[3] && !calculatedIntersections[4]) {
      calculatedIntersections[4] = true;
      used = true;
    } else if (layerFlag[2] && layerFlag[3] && !calculatedIntersections[5]) {
      calculatedIntersections[5] = true;
      used = true;
    } else {
      used = false;
    }

    if (layerCount == 2 && used) {
      angle = atan((x2 - x1) / (y1 - y2));
      //angle = 2*atan((sqrt(x1*x1-2*x1*x2+x2*x2+y1*y1-2*y1*y2+y2*y2)+y1-y2)/(x1-x2));
      d = x1 * cos(angle) + y1 * sin(angle);
      //d = x2 * cos(angle) + y2 * sin(angle);
      r = 1.0 / (x1 * cos(angle) + y1 * sin(angle));

      anglesum += angle;
      dsum += d;
      anglesqr += angle * angle;
      dsqr += d * d;
      rsum += r;
      rsqr += r * r;
      intersectionCount++;
    }

    /*
    if (layerFlag[0] && layerFlag[1]) {
      calculatedIntersections[0] = true;
    } else if (layerFlag[0] && layerFlag[2]) {
      calculatedIntersections[1] = true;
    } else if (layerFlag[0] && layerFlag[3]) {
      calculatedIntersections[2] = true;
    } else if (layerFlag[1] && layerFlag[2]) {
      calculatedIntersections[3] = true;
    } else if (layerFlag[1] && layerFlag[3]) {
      calculatedIntersections[4] = true;
    } else if (layerFlag[2] && layerFlag[3]) {
      calculatedIntersections[5] = true;
    }
    */

  }

  angle = anglesum / ((double)intersectionCount);
  // as it should
  //d = dsum / ((double)intersectionCount);
  // how it works best
  d = dsum / ((double)intersectionCount - 1.0);
  sigmaangle = sqrt((anglesqr - (anglesum * anglesum)) / ((double)intersectionCount - 1.0));
  sigmad = sqrt((dsqr - (dsum * dsum)) / ((double)intersectionCount - 1.0));
  // Nutzung von d(mean(angle)) = nächste Zeilen hat keinen guten Effekt und sieht genau so bescheiden aus wie mean(d)(angle)
  //d = x1 * cos(angle) + y1 * sin(angle);
  //sigmad = (x1 * sin(angle) + y1 * cos(angle)) * sigmaangle;
  r = rsum / ((double)intersectionCount - 1.0);
  double sigmaR = sqrt((rsqr - (rsum * rsum)) / ((double)intersectionCount - 1.0));

  result[0] = angle;
  result[2] = sigmaangle;
  //result[1] = d;
  //result[3] = sigmad;
  result[1] = 1.0 / r;
  result[3] = 1.0 / sigmaR;
}


/**
 * Find and combine into 3D tracks.
 */
void
SVDHoughTrackingModule::fac3d()
{
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  //StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);

  //const RecoTrack* recoTrack = nullptr;
  //RecoTrack* recoTrack = nullptr;

  vector<unsigned int> n_idList, p_idList;
  unsigned int tracks;
  TVector2 n_tc, p_tc;
  double r, phi, theta, d;
  bool all = false; /* combine every track in N and P */

  TVector3 houghMomentum;
  vector<double> positionCovariance(9, 0.);
  vector<double> momentumCovariance(9, 0.);

  /* During testbeam it is best to combine every track, because the sensors are very
   * noisy and partly broken.
   */
  if (m_tbMapping) {
    all = true;
  }
  if (m_useAllStripCombinations) {
    all = true;
  }

  if (!storeHoughTrack.isValid()) {
    storeHoughTrack.create();
  } else {
    storeHoughTrack.getPtr()->Clear();
  }

  tracks = 0;

  /* Phi reconstruction only */
  if (m_usePhiOnly) {
    for (auto it = p_houghTrackCand.begin(); it != p_houghTrackCand.end(); ++it) {
      p_idList = it->getIdList();
      ++tracks;
      p_tc = it->getCoord();               // p_tc contains intersection coords in p-HS (phi, rho), thus: p_tc = (phi, rho) of intersection
      //r = 1.0 / (2.0 * p_tc.Y());        // old version, used by Michael
      r = 1.0 / p_tc.Y();                  // new version, with HS curve rho = 2/r * sin(phi - phi0), r and phi0 being coords of hit

      /* Determine if we use tb mapping or not */
      if (m_tbMapping) {
        if (r < 0.0) {
          phi = 1.0 * p_tc.X() + M_PI / 2.0;                  // old version of Michael
          //phi = 1.0 * p_tc.X();
        } else {
          phi = (1.0 * p_tc.X() + M_PI / 2.0) - M_PI;         // old version of Michael
          //phi = (1.0 * p_tc.X()) - M_PI;
        }
        if (phi > M_PI) {
//          phi -= 2.0 * M_PI;
        } else if (phi < -1.0 * M_PI) {
          phi += 2.0 * M_PI;
        }
      } else {
        if (r < 0.0) {
          phi = 1.0 * p_tc.X() + M_PI / 2.0;                  // old version of Michael
          //phi = 1.0 * p_tc.X();
        } else {
          phi = (1.0 * p_tc.X() + M_PI / 2.0) - M_PI;         // old version of Michael
          //phi = (1.0 * p_tc.X()) - M_PI;
        }
        if (phi > M_PI) {
//          phi -= 2.0 * M_PI;
        } else if (phi < -1.0 * M_PI) {
          phi += 2.0 * M_PI;
        }
      }

      /* Radius Filter */
      if (m_useRadiusFilter) {
        if (fabs(r) > m_radiusThreshold) {
          storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, 0.0));
        }
      } else {
        storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, 0.0));
      }
    }

    return;
  }

  /* Theta reconstruction only */
  if (m_useThetaOnly) {

    for (auto it = n_houghTrackCand.begin(); it != n_houghTrackCand.end(); ++it) {
      ++tracks;
      n_tc = it->getCoord();

      /* Determine if we use tb mapping or not */
      if (m_tbMapping) {
        if (n_tc.X() > 0.0) {
          theta = n_tc.X(); // (M_PI) - n_tc.X();
        } else {
          theta = -1.0 * n_tc.X(); //(M_PI / 2) - n_tc.X();
        }
      } else {
        if (n_tc.X() > 0.0) {
          theta = n_tc.X(); // (M_PI) - n_tc.X();
        } else {
          theta = -1.0 * n_tc.X(); //(M_PI / 2) - n_tc.X();
        }
      }

      storeHoughTrack.appendNew(SVDHoughTrack(tracks, 0.0, 0.0, theta));
    }

    return;
  }

  /* Full track combination using phi and theta */
  for (auto it = n_houghTrackCand.begin(); it != n_houghTrackCand.end(); ++it) {
    for (auto it_in = p_houghTrackCand.begin(); it_in != p_houghTrackCand.end(); ++it_in) {
      n_idList = it->getIdList();
      p_idList = it_in->getIdList();

      /*
      int i = 0;
      for (auto iter = n_idList.begin(); iter != n_idList.end(); iter++) {
        B2INFO("n_idList[" << i << "]: " << *iter);
        i++;
      }
      i = 0;
      for (auto iter = p_idList.begin(); iter != p_idList.end(); iter++) {
        B2INFO("p_idList[" << i << "]: " << *iter);
        i++;
      }
      */


      //if ((it->getHash() == it_in->getHash() && it->getHitSize() == it_in->getHitSize()) || all) {
      if (compareList(p_idList, n_idList) || all) {
        //if (!compareList(p_idList, n_idList) || all) {

        /****** Begin of the estimation of the sign of the track curvature ******/
        /*
        VxdID sensorID;
        houghPair hp;
        unsigned int strip_id;
        vector<double> phiarray;
        vector<double> deltaphi;
        //int i = 0;
        double phidiff = 0.;
        int posdiff = 0, negdiff = 0;
        int curvsign = 0, charge = 0;

        // for(int i = p_idList.begin(); i < p_idList.end(); i++) {
        for (auto it_idList = p_idList.begin(); it_idList != p_idList.end();
             it_idList++) {
          for (auto iter = p_hough.begin(); iter != p_hough.end(); iter++) {
            //idx = iter->first;
            strip_id = iter->first;
            hp = iter->second;
            sensorID = hp.first;

            if (strip_id == *it_idList) {
              phiarray.push_back(hp.second.X());
            }
          }
        }

        for (auto phiarrayiter = phiarray.begin(); phiarrayiter != phiarray.end(); phiarrayiter++) {
          phidiff = (*(phiarrayiter + 1)) - (*phiarrayiter);
          if (phidiff > M_PI) {
            deltaphi.push_back(phidiff - 2.0 * M_PI);
          } else if (phidiff < -1.0 * M_PI) {
            deltaphi.push_back(phidiff + 2.0 * M_PI);
          } else {
            deltaphi.push_back(phidiff);
          }
        }

        for (auto deltaiter = deltaphi.begin(); deltaiter != deltaphi.end(); deltaiter++) {
          if ((*deltaiter) > 0.) {
            posdiff++;
          } else if ((*deltaiter) < 0.) {
            negdiff++;
          }
        }

        if (posdiff > negdiff) {
          curvsign = +1;      // corresponds to negatively charged particles
        } else if (negdiff > posdiff) {
          curvsign = -1;      // corresponds to positively charged particles
        }
        */
        int curvsign;
        curvsign = CurvatureEstimation(p_idList);

        int charge = -curvsign;

        /****** End of the estimation of the sign of the track curvature ******/



        /** Begin of "real" finding and combining */
        ++tracks;

        if (m_analyticalIntersection) {
          vector<double> analyticalResult(5, 0.0);
          /** Calculate analytical intersection for rphi plane */
          AnalyticalIntersection(p_idList, true, analyticalResult);
          phi = analyticalResult[0];
          r = 1.0 / analyticalResult[1];

//           positionCovariance[0] = analyticalResult[3];
//           positionCovariance[4] = analyticalResult[4];
//           positionCovariance[1] = 0.;       // analyticalResult[5];
//           positionCovariance[3] = 0.;       // analyticalResult[5];

          /** Calculate analytical intersection for theta / z */
          AnalyticalIntersection(n_idList, false, analyticalResult);
          theta = analyticalResult[0];
          d = analyticalResult[1];

//           positionCovariance[8] = analyticalResult[3];

        } else { /** Use intersections from clustered HS */
          n_tc = it->getCoord();
          p_tc = it_in->getCoord();            //!< p_tc contains intersection coords in p-HS (phi, rho), thus: p_tc = (phi, rho) of intersection
          //r = 1.0 / (2.0 * p_tc.Y());        //!< old version, used by Michael
          r = 1.0 / p_tc.Y();                  //!< new version, with HS curve rho = 2/r * sin(phi - phi0), r and phi0 being coords of hit
          phi = p_tc.X();
          theta = n_tc.X();
        }

        if (m_straightTracks && all) {
          storeHoughTrack.appendNew(SVDHoughTrack(p_tc.X(), p_tc.Y(), n_tc.X(), n_tc.Y()));
          return;
        }

        /* Determine if we use tb mapping or not */
        if (m_tbMapping) {
          if (r < 0.0) {
            phi = phi + M_PI / 2.0;
            //phi = 1.0 * p_tc.X() + M_PI / 2.0;                // old version of Michael, works for non-analytic version
            //phi = 1.0 * p_tc.X();
          } else {
            phi = (phi + M_PI / 2.0) - M_PI;
            //phi = (1.0 * p_tc.X() + M_PI / 2.0) - M_PI;       // old version of Michael, works for non-analytic version
            //phi = (1.0 * p_tc.X()) - M_PI;
          }
          if (phi > M_PI) {
//          phi -= 2.0 * M_PI;
          } else if (phi < -1.0 * M_PI) {
            phi += 2.0 * M_PI;
          }

          if (n_tc.Y() > 0.0) {
            theta = -1.0 * theta;
          } else {
            theta = M_PI - theta;
          }


          /* Radius Filter */
          if (m_useRadiusFilter) {
            if (fabs(r) > m_radiusThreshold && fabs(r) < 5000) {
              storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, theta, charge, curvsign));
            }
          } else {
            if (fabs(r) < 5000) {
              storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, theta, charge, curvsign));
            }
          }
        } else {  // begin of "else" belonging to "if(m_tbMapping)"
          if (r < 0.0) {
            phi = phi + M_PI / 2.0;
          } else {
            phi = (phi + M_PI / 2.0) - M_PI;
          }
          if (phi > M_PI) {
            phi -= 2.0 * M_PI;
          } //else if (phi < -1.0 * M_PI) {
          if (phi < -1.0 * M_PI) {
            phi += 2.0 * M_PI;
          }

          /** old version with theta projection algorithm */
          if (m_projectionRecoN) {
            if (n_tc.Y() > 0.0) {
              theta = -1.0 * theta;
            } else {
              theta = -1.0 * theta;
            }

            if (theta > M_PI) {
              theta -= 2.0 * M_PI;
            } //else if (theta < -1.0 * M_PI) {
            if (theta < -1.0 * M_PI) {
              theta += 2.0 * M_PI;
            }
          } else { /* default */
            // works puurrrrrfectly, most theta are reconstructed correctly, not only in projection! :)
            if (n_tc.Y() > 0.0) {           // n_tc.Y() > 0.0 works quite well
              theta = -1.0 * theta;
            } else {
              theta = M_PI - theta;
            }

            if (theta < 0.0) {
              theta += M_PI;
            } else if (theta > M_PI) {
              theta -= M_PI;
            }

            /*  // should (?) be right, but seems to be wrong
            if (theta > M_PI) {
              theta -= 2.0*M_PI;
              //theta -= M_PI;
            } else if (theta < -1.0*M_PI) {
              theta += 2.0*M_PI;
              //theta += M_PI;
            }
            */

          }

          /* Radius Filter */
          if (m_useRadiusFilter) {
            if (fabs(r) > m_radiusThreshold && fabs(r) < 5000) {
              storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, theta, charge, curvsign));
              //m_histCurvatureSign->Fill(curvsign);
            }
          } else {
            if (fabs(r) < 5000) {
              storeHoughTrack.appendNew(SVDHoughTrack(tracks, r, phi, theta, charge, curvsign));
              //m_histCurvatureSign->Fill(curvsign);
            }
          }

//           TVector3 houghMomentum;
//           vector<double> momentumCovariance(9, 0.);
          calculateMomentum(r, phi, theta, houghMomentum, positionCovariance, momentumCovariance);

          // Since the algorithm assumes the tracks starts in the origin, set the seedPosition to (0, 0, 0)
          TVector3 seedPosition(0.0, 0.0, 0.0);
          //recoTrack.setPositionAndMomentum(seedPosition, houghMomentum);
          //recoTrack->setPositionAndMomentum(seedPosition, houghMomentum);

          //saveHitsToRecoTrack(recoTrack, p_idList, false);
          //saveHitsToRecoTrack(recoTrack, n_idList, true);


        }
      }
    }
  }
}


/**
 * Calculate momentum from Hough algorithm data as well as its covariance matrix
 */
void
SVDHoughTrackingModule::calculateMomentum(double r, double phi, double theta, TVector3& momentum,
                                          vector<double>& positionCovariance, vector<double>& momentumCovariance)
{
  BFieldMap& bfieldMap = BFieldMap::Instance();
  TVector3 magField = bfieldMap.getBField(TVector3(0, 0, 0));
  double BFieldStrength = magField.Mag();

  double pX = 0.299792458 * BFieldStrength * r * cos(phi);
  double pY = 0.299792458 * BFieldStrength * r * sin(phi);
  double pZ = 0.299792458 * BFieldStrength * r / tan(theta);

  momentum.SetXYZ(pX, pY, pZ);

  /**
   * sR2 = sigma_R^2
   * a = cov(R,phi)
   * sphi2 = sigma_phi^2
   * stheta2 = sigma_theta^2
   */

  double sR2      = positionCovariance[0];
  double a        = positionCovariance[1];
  double sphi2    = positionCovariance[4];
  double stheta2  = positionCovariance[8];

  /*
  momentumCovariance[0] = pow(0.299792458 * BFieldStrength, 2.0) * (cos(phi) * (cos(phi)*sR2 - a*r*sin(phi))) - (r*sin(phi) * (a*cos(phi) - r*sphi2*sin(phi)));
  momentumCovariance[1] = pow(0.299792458 * BFieldStrength, 2.0) * (sin(phi) * (cos(phi)*sR2 - a*r*sin(phi))) + (r*cos(phi) * (a*cos(phi) - r*sphi2*sin(phi)));
  momentumCovariance[2] = pow(0.299792458 * BFieldStrength, 2.0) * (cos(phi)*sR2 - a*r*sin(phi)) / tan(theta);
  momentumCovariance[3] = pow(0.299792458 * BFieldStrength, 2.0) * (-r*sin(phi) * (r*cos(phi)*sphi2 + a*sin(phi))) + (cos(phi) * (a*r*cos(phi) + sR2*sin(phi)));
  momentumCovariance[4] = pow(0.299792458 * BFieldStrength, 2.0) * ( r*cos(phi) * (r*cos(phi)*sphi2 + a*sin(phi))) + (sin(phi) * (a*r*cos(phi) + sR2*sin(phi)));
  momentumCovariance[5] = pow(0.299792458 * BFieldStrength, 2.0) * (a*r*cos(phi) + sR2*sin(phi)) / tan(theta);
  momentumCovariance[6] = pow(0.299792458 * BFieldStrength, 2.0) * (cos(phi)*sR2 - a*r*sin(phi)) / tan(theta);
  momentumCovariance[7] = pow(0.299792458 * BFieldStrength, 2.0) * (a*r*cos(phi) + sR2*sin(phi)) / tan(theta);
  momentumCovariance[8] = pow(0.299792458 * BFieldStrength, 2.0) * sR2 / (tan(theta)*tan(theta)) + r*r*stheta2 / (pow(sin(theta), 4.0));
  */

}


/**
 * Save (Track) Hits to RecoTrack
 */
void
SVDHoughTrackingModule::saveHitsToRecoTrack(RecoTrack* recoTrack, vector<unsigned int>& idList, bool p_side)
{
  VxdID sensorID;
  houghMap hough;
  houghPair hp;
  unsigned int stripID, stripID_in;
  unsigned int hitID, hitID_in;
  int layerCount = 0;
  int layer, ladder, sensor;
  double positionOnSensor = 0.;
  double positionSigma = 0.;

  std::vector<SVDHoughTrackCand> houghTrackCand;

  if (p_side) {
    houghTrackCand = p_houghTrackCand;
    hough = p_hough;
  } else {
    houghTrackCand = n_houghTrackCand;
    hough = n_hough;
  }

  for (auto it = houghTrackCand.begin(); it != houghTrackCand.end(); it++) {

  }

  for (auto it = idList.begin(); it != idList.end(); it++) {
    hitID = *it;
    stripID = (hitID) - ((hitID / 10000000) * 10000000) - (((*it) / 10) * 10);
    for (auto iter = hough.begin(); iter != hough.end(); iter++) {

      hitID_in = iter->first;
      stripID_in = (hitID_in) - ((hitID_in / 10000000) * 10000000) - (((*it) / 10) * 10);
      hp = iter->second;
      sensorID = hp.first;
      layer = sensorID.getLayerNumber();
      ladder = sensorID.getLadderNumber();
      sensor = sensorID.getSensorNumber();

      if (hitID == hitID_in && stripID == stripID_in) {
        //static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
        //const VXD::SensorInfoBase& info = geo.get(sensorID);

        const SVD::SensorInfo* currentSensor = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
        if (!(sensor == 1 && layer > 3)) {    /* For rectangular sensors it's easy since the pitch doesn't depend on v */
          if (p_side) {
            positionOnSensor = currentSensor->getUCellPosition(stripID);
            positionSigma = currentSensor->getUPitch() / sqrt(12.);
          } else {
            positionOnSensor = currentSensor->getVCellPosition(stripID);
            positionSigma = currentSensor->getVPitch() / sqrt(12.);
          }
        } else {      /* For the slanted sensors it's a little more complicated though... */
          //TODO
        }

        /* How to give the charges, times and stuff if they are not known? */
        SVDCluster svdCluster(sensorID, p_side, positionOnSensor, positionSigma, -1, -1, -1, -1, -1);

      }

    }
  }

}


/**
 * Merge duplicated tracks (means tracks with difference of m_mergeThreshold).
 */
void
SVDHoughTrackingModule::trackMerger()
{
  int cnt;
  //unsigned int id;
  vector<unsigned int> idList;

  std::vector<SVDHoughTrackCand> n_houghTrackCandCpy;
  std::vector<SVDHoughTrackCand> p_houghTrackCandCpy;
  std::vector<SVDHoughTrackCand> n_houghTrackCandMerged;
  std::vector<SVDHoughTrackCand> p_houghTrackCandMerged;

  n_houghTrackCandCpy = n_houghTrackCand;
  p_houghTrackCandCpy = p_houghTrackCand;

//  if(! (m_useTrackMergerP || m_useTrackMergerN)) {
//      return;
//  }

  /* Begin of p-side track merger */

  if (m_useTrackMerger && m_useTrackMergerP) {

    TVector2 p_tc, p_tc_in;
    double r, phi;

    B2DEBUG(200, " Size of cand list before: " << p_houghTrackCandCpy.size());
    for (auto it = p_houghTrackCandCpy.begin(); it != p_houghTrackCandCpy.end(); ++it) {
      idList = it->getIdList();
      p_tc = it->getCoord();
      r = 1.0 / p_tc.Y();                      // new version, with HS curve rho = 2/r * sin(phi - phi0), r and phi0 being coords of hit
      phi = p_tc.X();
      B2DEBUG(250, "  Phi: " << phi << " R: " << r);
    }

    //id = 0;
    while (p_houghTrackCandCpy.size() > 0) {
      auto it = p_houghTrackCandCpy.begin();
      idList = it->getIdList();
      p_tc = it->getCoord();
      r = 1.0 / p_tc.Y();                      // new version, with HS curve rho = 2/r * sin(phi - phi0), r and phi0 being coords of hit
      phi = p_tc.X();
      cnt = 1;

redo_p:
      for (auto it_in = (p_houghTrackCandCpy.begin() + 1); it_in != p_houghTrackCandCpy.end(); ++it_in) {
        p_tc_in = it_in->getCoord();
        if (fabs(p_tc.X() - p_tc_in.X()) < m_mergeThresholdP) {
          phi += p_tc_in.X();
          ++cnt;
          p_houghTrackCandCpy.erase(it_in);
          goto redo_p;
        }
      }
      if (p_houghTrackCandCpy.size() > 0) {
        p_houghTrackCandCpy.erase(it);
      }

      /* Add to list */
      p_houghTrackCandMerged.push_back(SVDHoughTrackCand(idList, TVector2(phi / ((double) cnt), p_tc.Y())));
    }

    B2DEBUG(200, "Size of cand list after: " << p_houghTrackCandMerged.size());
    for (auto it = p_houghTrackCandMerged.begin(); it != p_houghTrackCandMerged.end(); ++it) {
      idList = it->getIdList();
      p_tc = it->getCoord();
      phi =  p_tc.X();
      B2DEBUG(250, "  " << phi);
    }

    p_houghTrackCand = p_houghTrackCandMerged;
  }

  /* End of p-side track merger */


  /* Begin of n-side track merger */

  if (m_useTrackMerger && m_useTrackMergerN) {

    TVector2 n_tc, n_tc_in;
    double d, theta;

    B2DEBUG(200, " Size of cand list before: " << n_houghTrackCandCpy.size());
    for (auto it = n_houghTrackCandCpy.begin(); it != n_houghTrackCandCpy.end(); ++it) {
      idList = it->getIdList();
      n_tc = it->getCoord();
      d = n_tc.Y();                      // new version, with HS curve rho = 2/r * sin(phi - phi0), r and phi0 being coords of hit
      theta = n_tc.X();
      B2DEBUG(250, "  Theta: " << theta << " d: " << d);
    }

    //id = 0;
    while (n_houghTrackCandCpy.size() > 0) {
      auto it = n_houghTrackCandCpy.begin();
      idList = it->getIdList();
      n_tc = it->getCoord();
      d = n_tc.Y();
      theta = n_tc.X();
      cnt = 1;

redo_n:
      for (auto it_in = (n_houghTrackCandCpy.begin() + 1); it_in != n_houghTrackCandCpy.end(); ++it_in) {
        n_tc_in = it_in->getCoord();

        if (fabs(n_tc.X() - n_tc_in.X()) < m_mergeThresholdN) {
          theta += n_tc_in.X();
          ++cnt;
          n_houghTrackCandCpy.erase(it_in);
          goto redo_n;
        }
      }
      if (n_houghTrackCandCpy.size() > 0) {
        n_houghTrackCandCpy.erase(it);
      }

      /* Add to list */
      n_houghTrackCandMerged.push_back(SVDHoughTrackCand(idList, TVector2(theta / ((double) cnt), n_tc.Y())));
    }

    B2DEBUG(200, "Size of cand list after: " << n_houghTrackCandMerged.size());
    for (auto it = n_houghTrackCandMerged.begin(); it != n_houghTrackCandMerged.end(); ++it) {
      idList = it->getIdList();
      n_tc = it->getCoord();
      theta =  n_tc.X();
      B2DEBUG(250, "  " << theta);
    }

    n_houghTrackCand = n_houghTrackCandMerged;
  }

  /* End of n-side track merger */

}


/*
 * Analyse the clusters and compare the extrapolated coordinates
 * with official Clusterizer.
 */
void
SVDHoughTrackingModule::trackAnalyseMCParticle()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDHoughTrack> storeHoughTrack(m_storeHoughTrack);
  int nMCParticles, nTracks;
  int cnt_events = 0;               // Event number for ttree m_rootTree
  //int min_pos;
  static int run = 1;
  bool primeOnly = true;
  bool usePhi = true;
  bool useTheta = true;
  //bool* track_match_n;
  //bool* track_match_p;
  //bool* track_match;
  double BFieldStrength = 0;
  double MCRadius = 1000;
  double phi = 0., theta = 0., r = 1.E9;
  int    charge = 0;
  int    hough_charge = 1000, hough_curvsign = 1000;
  double hough_pT = -1000, hough_radius = 1000;
  double theta_tolerance = m_thetaTolerance;        /* tolerance in theta */
  double phi_tolerance = m_phiTolerance;            /* tolerance in phi */
  double radius_tolerance = m_radiusTolerance;      /* tolerance in radius in m */
  double pT_tolerance = m_pTTolerance;              /* tolerance in pT in GeV */
  double min, dist = -1000, pT = 0.0;
  /** mindelta_xxx: minimum delta of xxx */
  double mindelta_phi, mindelta_theta;
  /** xxx_mindelta: xxx corresponding to mindelta_xxx */
  double phi_mindelta, theta_mindelta;
  int nPrimary = 0, nCorrReco = 0, nCorrRecoN = 0, nCorrRecoP = 0;
  double m_r = 0.0;
  double frac = 360.0;
  double projected_theta;
  TVector3 mom, prim_mom;

  BFieldMap& bfieldMap = BFieldMap::Instance();
  TVector3 magField = bfieldMap.getBField(TVector3(0, 0, 0));
  BFieldStrength = magField.Mag();

  int i = 0, j = 0;

  nMCParticles = storeMCParticles.getEntries();
  nTracks = storeHoughTrack.getEntries();

//   /* Don't analyse high occ events */
//   if (nTracks > 10000) {
//     B2WARNING("High occupancy event. Skipping...");
//     ++validEvents; /* High occ doesn't mean it's not a valid event */
//     return;
//   }

  /* In Tb mode we skip this event, when there are not enough hits created in the
   * sensor to create a track.
   */
  if (m_tbMapping) {
    if (n_clusters.size() < 3 || p_clusters.size() < 3) {
      return;
    }
  }

  /* Valid event */
  ++validEvents;

  /* Disable Theta reco */
  if (m_usePhiOnly) {
    useTheta = false;
  }

  /* Disable Phi reco */
  if (m_useThetaOnly) {
    usePhi = false;
  }

  // ersetze new... durch std::vector
  vector<bool> track_match_n; // (nMCParticles, false);           // new bool[nMCParticles]();
  vector<bool> track_match_p; // (nMCParticles, false);           // new bool[nMCParticles]();
  vector<bool> track_match; // (nMCParticles, false);             // new bool[nMCParticles]();
  //track_match_n.reserve(nMCParticles);
  //track_match_p.reserve(nMCParticles);
  //track_match.reserve(nMCParticles);
  nPrimary = 0;
  pT = 0.0;

  if (m_compareMCParticleVerbose) {
    B2INFO("MCParticle Comparison: ");
  } else {
    B2DEBUG(1, "MCParticle Comparison: ");
  }

  for (const MCParticle& particle : storeMCParticles) {
    //for (int i = 0; i < nMCParticles; ++i) {

    //cnt_events++;
    // Trying to access particle.getCharge() would give a problem for some particles (mainly nuclei, nuclear fractions, nucleons)
    if (!TDatabasePDG::Instance()->GetParticle(particle.getPDG())) {
      //B2WARNING("Unknown PDG code - particle.getCharge() would create error message!");
      continue;
    } else {
      charge = particle.getCharge();              // storeMCParticles[i]->getCharge();
    }
    mom = particle.getMomentum();               // storeMCParticles[i]->getMomentum();
    TVector3 vertex = particle.getVertex();     // storeMCParticles[i]->getVertex();

    MCParticleInfo mcParticleInfo(particle, magField);
    double Omega = mcParticleInfo.getOmega();

    //  if (/*mom.Perp() < 0.04 || */ mom.Theta()/Unit::deg < 16 || mom.Theta()/Unit::deg > 152) {
    //    //B2WARNING("Track with pT < 50 MeV. Can't reach outermost SVD layer => Skipping!!!");
    //    continue;
    //  }

    cnt_events++;

    //if (storeMCParticles[i]->getMother() == NULL || !primeOnly) {                   //  doof, weil funktioniert nicht für zerfallende Teilchen
    /*if ((particle.hasStatus(MCParticle::c_PrimaryParticle) && charge != 0) {*/
    if (particle.hasStatus(MCParticle::c_PrimaryParticle) &&  particle.hasStatus(MCParticle::c_StableInGenerator) &&
        /* (particle.hasStatus(MCParticle::c_LeftDetector) || particle.hasStatus(MCParticle::c_StoppedInDetector)) && */
        charge != 0) {           // funktioniert dann für aktualisierte Version mit neuer Schleife

      m_treeMCDataEventNo         = evtNumber;
      m_treeMCDataEventParticleNo = MCParticleNumber++;
      m_treeMCDataPDG             = particle.getPDG();
      m_treeMCDataCharge          = particle.getCharge();
      m_treeMCDataPhi             = mom.Phi() / Unit::deg;
      m_treeMCDataTheta           = mom.Theta() / Unit::deg;
      m_treeMCDataPx              = mom.X();
      m_treeMCDataPy              = mom.Y();
      m_treeMCDataPz              = mom.Z();
      m_treeMCDataPT              = mom.Perp();
      m_treeMCDataVx              = vertex.X() / Unit::mm;
      m_treeMCDataVy              = vertex.Y() / Unit::mm;
      m_treeMCDataVz              = vertex.Z() / Unit::mm;

      m_rootTreeMCData->Fill();

      /* TTree event number. != Simulation event number !!! */
      m_treeTrackingEventNo++;

      B2DEBUG(20000, "Vertex: " << vertex.X() << " " << vertex.Y() << " " << vertex.Z());
      prim_mom = mom;

      m_treeMCPDG_all = particle.getPDG();
      m_treeMCVertexX_all = vertex.X() / Unit::mm;
      m_treeMCVertexY_all = vertex.Y() / Unit::mm;
      m_treeMCVertexZ_all = vertex.Z() / Unit::mm;
      m_treeMCMomentumX_all = mom.X();
      m_treeMCMomentumY_all = mom.Y();
      m_treeMCMomentumZ_all = mom.Z();

      /* Get pT of particles */
      pT = mom.Perp();
      //MCRadius = (pT / (-0.299792458 * BFieldStrength * (double)charge));      // should be the track radius of the MCParticle in m
      MCRadius = 1 / Omega;
      m_histMCTrackR->Fill(MCRadius / Unit::m);
      m_treeMCTrackR = MCRadius / Unit::m;
      m_histMCCurvatureSign->Fill(-1.0 * charge);
      m_treeMCCurvatureSign = -1 * charge;
      m_histMCCharge->Fill(charge);
      m_treeMCCharge = charge;
      /* Write pT in pT histogram */
      m_histMCPTDist->Fill(pT);
      m_treeMCPTDist = pT;
      /* Write Phi and Theta into histogram */
      m_histMCPhiDist->Fill(mom.Phi() / Unit::deg);
      m_treeMCPhiDist = mom.Phi() / Unit::deg;
      m_histMCThetaDist->Fill(mom.Theta() / Unit::deg);
      m_treeMCThetaDist = mom.Theta() / Unit::deg;

      m_histHoughThetaDistvsMCPhiDist2D->Fill(mom.Phi() / Unit::deg, mom.Theta() / Unit::deg);
      //m_histThetaPhiDist2D->Fill(mom.Phi() / Unit::deg, mom.Theta() / Unit::deg);   // CWessel, 29.10.2015

      if (m_tbMapping) {
        projected_theta = mom.Theta();
      } else if (m_projectionRecoN) {
        projected_theta = atan(mom.Y() / mom.Z()); /* Calculate the projected theta */
        //projected_theta = atan(mom.X() / mom.Z()); /* Calculate the projected theta */
      } else {
        //projected_theta = atan(mom.Perp() / mom.Z());  // works puurrrrrfectly, most theta are reconstructed correctly, not only in projection! :)
        projected_theta = mom.Theta();
      }
      m_histProjectedThetaDist->Fill(projected_theta / Unit::deg);
      m_treeProjectedThetaDist = projected_theta / Unit::deg;

      B2DEBUG(250, "  MCParticleInfo: Mom Vec: " << mom.X() << " " << mom.Y() << " " << mom.Z());
      if (m_compareMCParticleVerbose) {
        cout << "  ID: " << particle.getIndex() << " PDG ID: "
             << boost::format("+%+3f") % particle.getPDG()
             << " Phi: " << boost::format("%+3.6f") % mom.Phi() << " ("
             << boost::format("%+3.6f") % (mom.Phi() / Unit::deg) << ")"
             << " Theta: " << boost::format("%+3.6f") % mom.Theta() << " ("
             << boost::format("%+3.6f") % (mom.Theta() / Unit::deg) << ") "
             << " P: " << boost::format("%+2.3f") % mom.Mag()
             << " Pt: " << boost::format("%+2.3f") % mom.Perp();
      } else {
        B2DEBUG(1, "  ID: " << particle.getIndex() << " PDG ID: " << particle.getPDG()
                << " Phi: " << mom.Phi() << " (" << mom.Phi() / Unit::deg << ")"
                << " Theta: " << mom.Theta() << " (" << mom.Theta() / Unit::deg << ")");
      }
      ++nPrimary;
      mindelta_phi = mindelta_theta = 2.0 * M_PI;
      phi_mindelta = theta_mindelta = 2.0 * M_PI;
      min = 9E+99;
      //for (int j = 0; j < nTracks; ++j) {
      j = 0;
      for (const SVDHoughTrack& svdHoughTrack : storeHoughTrack) {

        m_treeHoughDataEventNo          = evtNumber;
        m_treeHoughDataEventTrackNo     = HoughTrackNumber++;
        m_treeHoughDataCharge           = svdHoughTrack.getTrackCharge();
        m_treeHoughDataCurvSign         = svdHoughTrack.getTrackCurvature();
        m_treeHoughDataPhi              = svdHoughTrack.getTrackPhi() / Unit::deg;
        m_treeHoughDataTheta            = svdHoughTrack.getTrackTheta() / Unit::deg;
        m_treeHoughDataRadius           = svdHoughTrack.getTrackR() / Unit::m;

        m_rootTreeHoughData->Fill();


        /* This is NOT the real track radius, but only the (maybe wrongly signed) value
         * from the hough space intersection point.
         * The real radius is fabs(r), the curvature: see below.
         * The signed radius is then hough_radius = hough_curvsign * fabs(r)
         */
        //r = storeHoughTrack[j]->getTrackR();
        //hough_charge = storeHoughTrack[j]->getTrackCharge();
        //hough_curvsign = storeHoughTrack[j]->getTrackCurvature();
        //hough_radius = hough_curvsign*fabs(r);

        r = svdHoughTrack.getTrackR();
        hough_charge = svdHoughTrack.getTrackCharge();
        hough_curvsign = svdHoughTrack.getTrackCurvature();
        hough_radius = hough_curvsign * fabs(r);
        hough_pT = 0.299792458 * BFieldStrength * fabs(r) / Unit::m;

        /* XXX: Hack for determing curling direction */

        /* see further below
        m_histHoughTrackR->Fill(hough_radius);
        m_treeHoughTrackR = hough_radius;
        m_histHoughCharge->Fill(hough_charge);
        m_treeHoughCharge = hough_charge;
        m_histHoughCurvatureSign->Fill(hough_curvsign);
        m_treeHoughCurvatureSign = hough_curvsign;

        if (hough_curvsign == -1*charge) {
          m_histCorrectReconHoughCurvatureSign->Fill(hough_curvsign);
          m_treeCorrectReconHoughCurvatureSign = hough_curvsign;
          m_histCorrectReconMCCurvatureSign->Fill(-1.0*charge);
          m_treeCorrectReconMCCurvatureSign = -1*charge;
        }
        if (hough_charge == charge) {
          m_histCorrectReconHoughCharge->Fill(hough_charge);
          m_histCorrectReconMCCharge->Fill(charge);
        }
        */

        //if (charge < 0) {
        if (hough_charge < 0) {
          /* Hough space intersection gives right phi for negatively charged particles,
           * but not positively charged particles
           */
          //phi = storeHoughTrack[j]->getTrackPhi();
          phi = svdHoughTrack.getTrackPhi();
        } else {
          /* For positively charged particles, M_PI has to be added or substracted,
           * depending on the sign of the intersection point rho = 1/r <=> r = 1/rho
           */
          if (r < 0.0) {
            //phi = storeHoughTrack[j]->getTrackPhi() - M_PI;
            phi = svdHoughTrack.getTrackPhi() - M_PI;
          } else {
            //phi = storeHoughTrack[j]->getTrackPhi() + M_PI;
            phi = svdHoughTrack.getTrackPhi() + M_PI;
          }
        }

        if (phi > M_PI) {
          phi -= 2.0 * M_PI;
        }
        if (phi < -1.0 * M_PI) {
          phi += 2.0 * M_PI;
        }

        //theta = storeHoughTrack[j]->getTrackTheta();
        theta = svdHoughTrack.getTrackTheta();

        if (theta < 0.0) {
          theta += M_PI;
        } else if (theta > M_PI) {
          theta -= M_PI;
        }

        /*
        if (theta > M_PI) {
          theta -= 2.0*M_PI;
        }
        if (theta < -1.0*M_PI) {
          theta += 2.0*M_PI;
        }
        */

        //dist = fabs((phi - mom.Phi())) + fabs((theta - mom.Theta()));
        dist = sqrt((phi - mom.Phi()) * (phi - mom.Phi()) + (theta - mom.Theta()) * (theta - mom.Theta()));
        if (dist < min) {
          min = dist;
          //min_pos = j;
          //min_phi = (phi - mom.Phi()) / Unit::deg;
          //min_theta = (theta - mom.Theta()) / Unit::deg;
        }

        if (fabs(phi - mom.Phi()) <= fabs(mindelta_phi)) {
          mindelta_phi = phi - mom.Phi();
          phi_mindelta = phi;
          if (mindelta_phi > M_PI) {
            mindelta_phi -= 2.0 * M_PI;
          } else if (mindelta_phi < -1.0 * M_PI) {
            mindelta_phi += 2.0 * M_PI;
          }
          m_r = hough_radius;
        }

        /* see further down
        if (fabs(m_r / Unit::m - MCRadius) <= radius_tolerance) {
          m_histCorrectReconHoughTrackR->Fill(m_r / Unit::m);
          m_histCorrectReconMCTrackR->Fill(MCRadius);
        }
        */

        /*
        //if (fabs((theta - (mom.Theta() * sin(mom.Phi())))) < fabs(min_theta)) {
        if (fabs(theta - projected_theta) < fabs(mindelta_theta)) {               // works, if using theta = -1.0*n_tc.X() in fac3d
          mindelta_theta = theta - projected_theta;
          theta_mindelta = theta;
          //if (fabs((storeHoughTrack[j]->getTrackTheta() - (mom.Theta()))) < fabs(min_theta)) {
          //  min_theta = ((storeHoughTrack[j]->getTrackTheta() - (mom.Theta())));
        }
        */

        /* these two if's work quite nicely... but nut perfect
        double theta_tmp = theta;
        if (theta_tmp > M_PI/2.0) {
          theta_tmp = M_PI - theta_tmp;
        }

        if (fabs(theta_tmp - projected_theta) < fabs(mindelta_theta)) {
          mindelta_theta = theta_tmp - projected_theta;
          theta_mindelta = theta;
        }
        */

        // works puurrrrrfectly, most theta are reconstructed correctly, not only in projection! :)
        //if (fabs(theta - mom.Theta()) < fabs(mindelta_theta)) {
        //  mindelta_theta = theta - mom.Theta();
        //  theta_mindelta = theta;
        //}
        if (fabs(theta - projected_theta) < fabs(mindelta_theta)) {
          mindelta_theta = theta - projected_theta;
          theta_mindelta = theta;
        }



#ifdef DBG_THETA
        ofstream os_angles;
        os_angles.open("dbg/angles.dat", ofstream::out | ofstream::app);
        os_angles << mom.Theta() / Unit::deg << "\t" << projected_theta / Unit::deg << "\t"
                  << theta / Unit::deg << "\t" << mom.Phi() / Unit::deg << "\t"
                  << phi / Unit::deg << endl;
        os_angles.close();
#endif


        B2DEBUG(150, "  MCP " << i << " Mom: " << mom.X() << ", " << mom.Y() << ", " << mom.Z()
                << " with Track (" << phi << ", "
                << theta << ") " << j << " Difference: phi = "
                << mindelta_phi << " theta = " << mindelta_theta << " projected theta: " << projected_theta << " ("
                << projected_theta / Unit::deg << ")");

        j++;
      }

      mindelta_theta /= Unit::deg;
      mindelta_phi /= Unit::deg;

      // set phi/theta to the values corresponding to the minimal distance phi/theta mindelta_phi/theta
      phi = phi_mindelta;
      theta = theta_mindelta;

      if (phi > M_PI) {
        phi -= 2.0 * M_PI;
      }
      if (phi < -1.0 * M_PI) {
        phi += 2.0 * M_PI;
      }

      if (theta < 0.0) {
        theta += M_PI;
      } else if (theta > M_PI) {
        theta -= M_PI;
      }

      if (fabs(m_r / Unit::m) <= 100) {
        m_histHoughTrackR->Fill(m_r / Unit::m);
        m_treeHoughTrackR = m_r / Unit::m;
        m_histHoughCharge->Fill(hough_charge);
        m_treeHoughCharge = hough_charge;
        m_histHoughCurvatureSign->Fill(hough_curvsign);
        m_treeHoughCurvatureSign = hough_curvsign;

        // Fill hough_pT into histogram
        m_histHoughPTDist->Fill(hough_pT);
        m_treeHoughPTDist = pT;

      }

      //int MCcurvsign = -1*charge ;
      if (hough_curvsign == -1 * charge) {
        m_histCorrectReconMCCurvatureSign->Fill(-1 * charge);
        m_treeCorrectReconMCCurvatureSign = -1 * charge;
        m_histCorrectReconHoughCurvatureSign->Fill(hough_curvsign);
        m_treeCorrectReconHoughCurvatureSign = hough_curvsign;
      }
      if (hough_charge == charge) {
        m_histCorrectReconMCCharge->Fill(charge);
        m_treeCorrectReconMCCharge = charge;
        m_histCorrectReconHoughCharge->Fill(hough_charge);
        m_treeCorrectReconHoughCharge = hough_charge;
      }

      if (fabs(m_r / Unit::m - MCRadius) <= radius_tolerance) {
        m_histCorrectReconMCTrackR->Fill(MCRadius);
        m_treeCorrectReconMCTrackR = MCRadius;
        m_histCorrectReconHoughTrackR->Fill(m_r / Unit::m);
        m_treeCorrectReconHoughTrackR = m_r / Unit::m;
      }


      /* Plot spread in histogram */
      if (nTracks != 0) {
        m_histHoughDiffPhi->Fill(mindelta_phi);
        m_treeHoughDiffPhi = mindelta_phi;
        m_histHoughDiffTheta->Fill(mindelta_theta);
        m_treeHoughDiffTheta = mindelta_theta;
        m_histHoughDiffPhivsPhi->Fill(mom.Phi() / Unit::deg, mindelta_phi);
        m_histHoughDiffThetavsTheta->Fill(mom.Theta() / Unit::deg, mindelta_theta);
      }

      /* Missing hit plots for MCParticle track in Phi and Theta */
      if (nTracks == 0) {
        m_histMissedHitsvsMCPT->Fill(pT);
        m_treeMissedPT = pT;
        m_histMissedHitsvsMCPhi->Fill(mom.Phi() / Unit::deg);
        m_treeMissedPhi = mom.Phi() / Unit::deg;
        m_histMissedHitsvsMCPT->Fill(mom.Theta() / Unit::deg);
        m_treeMissedTheta = mom.Theta() / Unit::deg;
      }

      if (usePhi /*&& mom.Theta() / Unit::deg > 17.0 && mom.Theta() / Unit::deg < 150.0*/) {
        m_histHoughPhiDist->Fill(phi / Unit::deg);
        m_treeHoughPhiDist = phi / Unit::deg;

        if (fabs(mindelta_phi) < phi_tolerance || (fabs(mindelta_phi) > (frac - phi_tolerance)
                                                   && fabs(mindelta_phi) != frac)) {
          if (m_r < 0.0) {
            if (m_r > minR) {
              minR = m_r;
            }
          } else {
            if (m_r < maxR) {
              maxR = m_r;
            }
          }
          if (m_compareMCParticleVerbose) {
            cout << "\t\033[1;32m" << " found  (" << mindelta_phi << ")\033[0m";
          }

          /* Efficiency vs pT */
          //  m_histHoughPhiReconvsMCPT->Fill(pT);    // obsolete, durch nächste Zeile ersetzt
          m_histHoughPhivsMCPTDist->Fill(pT);
          //m_treePTPhiRecon = pT;                    // obsolete, durch nächste Zeile ersetzt
          m_treeHoughPhivsMCPTDist = pT;

          //m_histPhiRecon->Fill(mom.Phi() / Unit::deg); /* Efficiency vs Phi */
          //m_histHoughPhiDist->Fill(phi / Unit::deg);
          //m_treePhiRecon = mom.Phi() / Unit::deg;

          m_histCorrectReconMCPhiDist->Fill(mom.Phi() / Unit::deg);
          m_treeCorrectReconMCPhiDist = mom.Phi() / Unit::deg;
          m_histCorrectReconHoughPhiDist->Fill(phi / Unit::deg);
          m_treeCorrectReconHoughPhiDist = phi / Unit::deg;

          m_histHoughThetavsMCPhiDist->Fill(mom.Phi() / Unit::deg); /* Efficiency of Theta vs Phi */
          //m_treeThetaReconPhiDist = mom.Phi() / Unit::deg;
          m_treeHoughThetavsMCPhiDist = mom.Phi();


          m_histCorrectReconPhivsMCPTDist->Fill(pT);                // For efficiency of the reconstruction of Phi as function of pT

          //track_match_p[i] = true;
          track_match_p.push_back(true);
          track_match.push_back(true);
        } else {
          track_match_p.push_back(false);
          track_match.push_back(false);
          if (m_compareMCParticleVerbose) {
            cout << "\t\033[1;31m" << " failed (" << mindelta_phi << ")\033[0m";
          }
        }
        if (!useTheta && m_compareMCParticleVerbose) {
          cout << endl;
        }
      }

      if (useTheta /*&& mom.Theta() / Unit::deg > 17.0 && mom.Theta() / Unit::deg < 150.0*/) {
        m_histHoughThetaDist->Fill(theta / Unit::deg);
        m_treeHoughThetaDist = theta / Unit::deg;

        if (fabs(mindelta_theta) < theta_tolerance) {
          if (m_compareMCParticleVerbose) {
            cout << "\033[1;32m" << " found (" << mindelta_theta << ")\033[0m" << endl;
          }

          /* Efficiency vs pT */
          //m_histHoughThetaReconvsMCPT->Fill(pT);    // obsolete, durch nächste Zeile ersetzt
          m_histHoughThetavsMCPTDist->Fill(pT);
          //m_treePTThetaRecon = pT;                  // obsolete, durch nächste Zeile ersetzt
          m_treeHoughThetavsMCPTDist = pT;

          //m_histThetaRecon->Fill(mom.Theta() / Unit::deg); /* Efficiency vs Theta */
          //m_treeThetaRecon = mom.Theta() / Unit::deg;
          //m_histProjectedThetaRecon->Fill(projected_theta / Unit::deg); /* Efficiency vs Theta */

          m_histCorrectReconMCThetaDist->Fill(mom.Theta() / Unit::deg);
          m_treeCorrectReconMCThetaDist = mom.Theta() / Unit::deg;
          m_histCorrectReconHoughThetaDist->Fill(theta / Unit::deg);
          m_treeCorrectReconHoughThetaDist = theta / Unit::deg;

          //m_histThetaReconPhiDist->Fill(mom.Phi() / Unit::deg); /* Efficiency of Theta vs Phi */ // obsolete??? ersetzt durch nächste Zeile
          m_histCorrectReconHoughThetavsMCPhiDist->Fill(mom.Phi() / Unit::deg); /* Efficiency of Theta vs Phi */
          m_treeCorrectReconHoughThetavsMCPhiDist = mom.Phi() / Unit::deg;

          m_histCorrectReconThetavsMCPTDist->Fill(pT);              // For efficiency of the reconstruction of Theta as function of pT


          m_histHoughThetaCorrectReconvsMCPhi2D->Fill(mom.Phi() / Unit::deg, mom.Theta() / Unit::deg);

          //track_match_n[i] = true;
          track_match_n.push_back(true);

          /* Found Theta, check also if Phi was correctly reconstructed */
          /* imho not completely wrong, but should be checked in "if(usePhi && useTheta)" below
          if (track_match_p[i]) {
            m_histPtRecon->Fill(pT);
            track_match[i] = true;
          }
          */
        } else {
          track_match_n.push_back(false);
          track_match[track_match.size() - 1] = false;
          if (m_compareMCParticleVerbose) {
            cout << "\033[1;31m" << " failed (" << mindelta_theta << ")\033[0m" << endl;
          }
        }
      }

      m_histHoughPhivsMCPhi2D_all->Fill(mom.Phi() / Unit::deg, phi / Unit::deg);
      m_histHoughThetavsMCTheta2D_all->Fill(mom.Theta() / Unit::deg, theta / Unit::deg);
      m_histHoughPhivsMCTheta2D_all->Fill(mom.Theta() / Unit::deg, phi / Unit::deg);
      m_histHoughThetavsMCPhi2D_all->Fill(mom.Phi() / Unit::deg, theta / Unit::deg);

      //m_treeHoughTrackR = m_r / Unit::m;
      //m_histHoughPhi->Fill(phi / Unit::deg);
      //m_treeHoughPhi = phi / Unit::deg;
      //m_histHoughTheta->Fill(theta / Unit::deg);
      //m_treeHoughTheta = theta / Unit::deg;
      hough_pT = fabs(-0.299792458 * 1.5 * hough_charge * (m_r / Unit::m));
      //m_treeHoughPT = fabs(-0.299792458 * 1.5 * hough_charge * m_r);

      if (fabs(hough_pT - pT) <= pT_tolerance) {
        m_histCorrectReconMCPTDist->Fill(pT);
        m_treeCorrectReconMCPTDist = pT;
        m_histCorrectReconHoughPTDist->Fill(hough_pT);
        m_treeCorrectReconHoughPTDist = hough_pT;
      }


      if (usePhi && useTheta /* && mom.Theta() / Unit::deg > 17.0 && mom.Theta() / Unit::deg < 150.0*/) {
        //m_histHoughTrackRRecon->Fill(m_r / Unit::m);
        //m_treeHoughTrackR = m_r / Unit::m;


        /* When the following statement is true, we assume the track to be found / correctly reconstructed
         * Thus, all histograms m_histCorrectReconXXX are filled in this case
         * The "real" efficiencies of this algorithm are to be calculated with these histograms
         */
        if ((fabs(mindelta_theta) < theta_tolerance) && (fabs(mindelta_phi) < phi_tolerance ||
                                                         (fabs(mindelta_phi) > (frac - phi_tolerance) && fabs(mindelta_phi) != frac))) {

          m_treeMCPDG = particle.getPDG();
          m_treeMCVertexX = vertex.X() / Unit::mm;
          m_treeMCVertexY = vertex.Y() / Unit::mm;
          m_treeMCVertexZ = vertex.Z() / Unit::mm;
          m_treeMCMomentumX = mom.X();
          m_treeMCMomentumY = mom.Y();
          m_treeMCMomentumZ = mom.Z();


          //if (track_match_p[i] && track_match_n[i]) {
          if (track_match_p[track_match_p.size() - 1] == true && track_match_n[track_match_n.size() - 1] == true) {
            //track_match[i] = true;
            track_match[track_match.size() - 1] = true;
          }

          //m_histThetaPhiRecon2D->Fill(mom.Phi() / Unit::deg, mom.Theta() / Unit::deg);
          m_histThetaCorrectReconvsMCPhi2D->Fill(mom.Phi() / Unit::deg, mom.Theta() / Unit::deg);
          m_histCorrectReconHoughTrackR->Fill(m_r / Unit::m);
          m_treeTrackRRecon = m_r / Unit::m;

          m_histCorrectReconTrackR->Fill(MCRadius);
          m_treeCorrectReconTrackR = MCRadius;
          m_histCorrectReconCurvatureSign->Fill(-1.0 * charge);     // Fill(hough_curvsign) ?
          m_treeCorrectReconCurvatureSign = -1 * charge;
          m_histCorrectReconCharge->Fill(charge);                   // Fill(hough_charge) ?
          m_treeCorrectReconCharge = charge;
          m_histCorrectReconPTDist->Fill(pT);
          m_treeCorrectReconPTDist = pT;
          m_histCorrectReconPhiDist->Fill(mom.Phi() / Unit::deg);
          m_treeCorrectReconPhiDist = mom.Phi() / Unit::deg;
          m_histCorrectReconThetaDist->Fill(mom.Theta() / Unit::deg);
          m_treeCorrectReconThetaDist = mom.Theta() / Unit::deg;

          m_histCorrectReconvsMCPTDist->Fill(pT);                   // For total efficiency as function of pT

          m_histHoughPhivsMCPhi2D_reco->Fill(mom.Phi() / Unit::deg, phi / Unit::deg);
          m_histHoughThetavsMCTheta2D_reco->Fill(mom.Theta() / Unit::deg, theta / Unit::deg);
          m_histHoughPhivsMCTheta2D_reco->Fill(mom.Theta() / Unit::deg, phi / Unit::deg);
          //m_histHoughThetavsMCPhi_reco->Fill(phi / Unit::deg, mom.Theta() / Unit::deg);
          m_histHoughThetavsMCPhi2D_reco->Fill(mom.Phi() / Unit::deg, theta / Unit::deg);

          /* If angles are correct, prove whether also the radius is correct */
          if (fabs((m_r / Unit::m) - MCRadius) < radius_tolerance) {
            m_histHoughTrackRCorrectRecon->Fill(MCRadius);
            m_treeHoughTrackRCorrectRecon = MCRadius;
          }

        }      // end of if( (fabs(min_theta) < theta_tolerance) ..... )

      }

      /// Fill tree
      m_rootTreeTracking->Fill();
      B2DEBUG(150, "\n");
    }


    i++;
  }
  if (m_compareMCParticleVerbose) {
    cout << "  Total MCParticles tracks: " << nMCParticles << " Reconstructed tracks: " << nTracks << endl;
  }

  nCorrRecoN = 0;
  nCorrRecoP = 0;
  nCorrReco = 0;
  for (int i = 0; i < nPrimary; ++i) {
    //for (int i = 0; i < track_match_p.size(); ++i) {
    //if (track_match[i] == true) {
    if (track_match_p[i] == true && track_match_n[i] == true) {
      ++nCorrReco;
    }
    if (track_match_n[i] == true) {
      ++nCorrRecoN;
    }
    if (track_match_p[i] == true) {
      ++nCorrRecoP;
    }
  }

  /* Recalculate track efficiency */
  if (nCorrReco > 0) {
    curTrackEff += ((((double) nCorrReco)) / (double)(nPrimary));
    //curTrackEff += (double) nCorrReco / (double)track_match.size();
    //m_histEventEfficiency->Fill( (double)nCorrReco / (double)track_match.size() );
    //m_treeEventEfficiency = (double)nCorrReco / (double)track_match.size();
    m_histEventEfficiency->Fill((double)nCorrReco / (double)nPrimary);
    m_treeEventEfficiency = (double)nCorrReco / (double)nPrimary;
  }
  if (nCorrRecoN > 0) {
    curTrackEffN += ((((double) nCorrRecoN)) / (double)(nPrimary));
    //curTrackEffN += ((((double) nCorrRecoN)) / (double)(track_match_n.size()));
  }
  if (nCorrRecoP > 0) {
    curTrackEffP += ((((double) nCorrRecoP)) / (double)(nPrimary));
    //curTrackEffP += ((((double) nCorrRecoP)) / (double)(track_match_p.size()));
  }
  //ntotFakeTracks += (double) (nTracks - nCorrRecoP - nCorrRecoN);
  ntotTracks += (unsigned int) nTracks;
//  if ((nTracks - nCorrRecoP) > 0) {
//    ntotFakeTracks += (double)(nTracks - nCorrRecoP);
  if ((nTracks - nCorrReco) > 0) {
    ntotFakeTracks += (double)(nTracks - nCorrReco);
    m_histEventFakeRate->Fill(nTracks - nCorrReco);
    m_treeEventFakeRate = nTracks - nCorrReco;
    //  m_histPtFake->Fill(pT, (nTracks - nCorrRecoP));   // wrong here, shifted some lines below to "if(nPrimary == 1)" clause
  }
  B2DEBUG(1, "   Fake tracks: " << ntotFakeTracks << " " << nTracks << " " << nCorrRecoP);

  /* For Fake study, only when we have one primary track */
  /*  if (nPrimary == 1) {
      m_treeFakes = (double)(nTracks - nCorrRecoP);
      m_histFakesinPTvsMCPT->Fill(pT, (double)(nTracks - nCorrRecoP));
      //m_histPtFakesPhi->Fill(prim_mom.Perp(), (double)(nTracks - nCorrRecoP));
      //m_histFakesinPhivsMCPT->Fill(prim_mom.Perp(), (double)(nTracks - nCorrRecoP));
      m_histFakesinPhivsMCPhi->Fill(prim_mom.Phi() / Unit::deg, (double)(nTracks - nCorrRecoP));
      m_histFakesinThetavsMCTheta->Fill(prim_mom.Theta() / Unit::deg, (double)(nTracks - nCorrRecoP));
    }*/

  m_treeEventEventNo = evtNumber;
  m_rootTreeEfficiency->Fill();
  //m_rootTreeTracking->Fill();

  ++run;

  //delete[] track_match_n;
  //delete[] track_match_p;
  //delete[] track_match;
}

void
SVDHoughTrackingModule::printCustomSummary()
{
  B2DEBUG(1, "--------- Custom Summary ---------");
  printClusters(n_clusters, true);
  printClusters(p_clusters, false);
  B2DEBUG(1, "--------- End Custom Summary ---------");
}

