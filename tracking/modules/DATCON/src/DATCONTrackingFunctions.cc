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

/**
* Find and combine into 3D tracks.
*/
void
DATCONTrackingModule::fac3d()
{
  vector<unsigned int> v_idList, u_idList;
  unsigned int tracks;
  TVector2 v_tc, u_tc;
  double r, phi, theta, d;
  bool all = false; /* combine every track in V and U */

  TVector3 houghMomentum;
  vector<double> positionCovariance(9, 0.);
  vector<double> momentumCovariance(9, 0.);

  if (m_useAllStripCombinations  || m_combineAllTrackCands) {
    all = true;
  }
  if (storeDATCONTracks.isValid()) {
    storeDATCONTracks.clear();
  }

  tracks = 0;

  /* Full track combination using phi and theta */
  for (auto it = vTrackCand.begin(); it != vTrackCand.end(); ++it) {
    for (auto it_in = uTrackCand.begin(); it_in != uTrackCand.end(); ++it_in) {
      v_idList = it->getIdList();
      u_idList = it_in->getIdList();

      if (compareList(u_idList, v_idList) || all) {

        int curvsign = 0;
        int charge = -curvsign;

        ++tracks;

        v_tc  = it->getCoord();
        u_tc  = it_in->getCoord();            //!< u_tc contains intersection coords in p-HS (phi, rho), thus: u_tc = (phi, rho) of intersection
        r     = 1.0 / u_tc.Y();               //!< new version, with HS curve rho = 2/r * sin(phi - phi0), r and phi0 being coords of hit
        phi   = u_tc.X();
        theta = v_tc.X();
        d     = v_tc.Y();

        /* Determine if we use tb mapping or not */
        if (m_usePhase2Simulation) {

          /* Copy of Bruno's code */
          r = 1.0 / (2.0 * u_tc.Y());
          if (r < 0.0) {
            charge = -1;
            curvsign = -charge;
          } else if (r > 0.0) {
            charge = 1;
            curvsign = -charge;
          } else {phi = M_PI / 2.0;}

          if (phi > M_PI) {
//            phi -= 2.0 * M_PI;
          } else if (phi < -1.0 * M_PI) {
            phi += 2.0 * M_PI;
          }
          if (phi > M_PI / 2.0) {
            phi -= M_PI;
          } else if (phi < M_PI / -2.0) {
            phi += M_PI;
          }

          if (v_tc.Y() /*v_tc.X()*/ > 0.0) {
            theta = -1.0 * v_tc.X();
            //} else {
            //  theta = M_PI - v_tc.X();
            //}
          } else if (v_tc.Y() /*v_tc.X()*/ < 0.0) {
            theta = M_PI - v_tc.X();
          } else {
            theta = M_PI / 2.0;
          }

          theta = v_tc.X();
          d = 0;

          storeDATCONTracks.appendNew(DATCONTrack(tracks, r, phi, d, theta, charge, curvsign));
          DATCONTracks.push_back(DATCONTrack(tracks, r, phi, d, theta, charge, curvsign));

          /* End of copy of Bruno's code */

        } else {  // begin of "else" belonging to "if(m_usePhase2Simulation)"
          if (r < 0.0) {
            charge = -1;
            curvsign = -charge;
            phi = phi + M_PI / 2.0;
          } else {
            charge = 1;
            curvsign = -charge;
            phi = (phi + M_PI / 2.0) - M_PI;
          }
          if (phi > M_PI) {
            phi -= 2.0 * M_PI;
          } //else if (phi < -1.0 * M_PI) {
          if (phi < -1.0 * M_PI) {
            phi += 2.0 * M_PI;
          }

          if (v_tc.Y() > 0.0) {
            theta = -1.0 * theta;
          } else {
            theta = M_PI - theta;
          }

          if (theta < 0.0) {
            theta += M_PI;
          } else if (theta > M_PI) {
            theta -= M_PI;
          }

          if (fabs(r) < 5000) {
            storeDATCONTracks.appendNew(DATCONTrack(tracks, r, phi, d, theta, charge, curvsign));
            DATCONTracks.push_back(DATCONTrack(tracks, r, phi, d, theta, charge, curvsign));
          }

          B2Vector3D magField = BFieldManager::getFieldInTesla({0, 0, 0});
          double BFieldStrength = magField.Mag();

          double pX = 0.299792458 * BFieldStrength * r * cos(phi);
          double pY = 0.299792458 * BFieldStrength * r * sin(phi);
          double pZ = 0.299792458 * BFieldStrength * r / tan(theta);

          houghMomentum.SetXYZ(pX, pY, pZ);

//           saveHitsToRecoTrack(u_idList, houghMomentum);

        }
      }
    }
  }
}


/**
* Save (Track) Hits to RecoTrack
*/
void
DATCONTrackingModule::saveHitsToRecoTrack(std::vector<unsigned int>& idList, TVector3 momentum)
{
  int spacePointIndex;
  unsigned int sortingParameter = 0;

  TVector3 seedPosition(0.0, 0.0, 0.0);
  TVector3 seedMomentum = momentum;

//   RecoTrack recoTrack(seedPosition, seedMomentum, 1.);
  RecoTrack* recoTrack = storeDATCONRecoTracks.appendNew(seedPosition, seedMomentum, 1.);

  for (auto it = idList.begin(); it != idList.end(); it++) {
    spacePointIndex = (int)(*it) - (((int)(*it) / 10000) * 10000);

    DATCONSVDSpacePoint* spacepoint = storeDATCONSVDSpacePoints[spacePointIndex];
    vector<SVDCluster> DATCONSVDClusters = spacepoint->getAssignedDATCONSVDClusters();
    for (auto& datconsvdcluster : DATCONSVDClusters) {
      RecoHitInformation::UsedSVDHit* usedSVDHit = &datconsvdcluster;
      RecoHitInformation* recohitinfo = m_storeRecoHitInformation.appendNew(usedSVDHit, RecoHitInformation::OriginTrackFinder::c_other,
                                        sortingParameter);
//       recoTrack->addSVDHit(&datconsvdcluster, sortingParameter++);
      recohitinfo->addRelationTo(usedSVDHit);
      usedSVDHit->addRelationTo(recohitinfo);
      recoTrack->addSVDHit(&datconsvdcluster, sortingParameter++);
    }
  }
}


/**
* Merge duplicated tracks (means tracks with difference of m_mergeThreshold).
*/
void
DATCONTrackingModule::trackCandMerger()
{
  int count;
  //unsigned int id;
  vector<unsigned int> idList;

  std::vector<DATCONTrackCand> uTrackCandCopy;
  std::vector<DATCONTrackCand> vTrackCandCopy;
  std::vector<DATCONTrackCand> uTrackCandMerged;
  std::vector<DATCONTrackCand> vTrackCandMerged;

  vTrackCandCopy = vTrackCand;
  uTrackCandCopy = uTrackCand;

  /* Begin of u-side trackCand merger */

  if (m_useTrackCandMerger && m_useTrackCandMergerU) {

    TVector2 u_tc, u_tc_in;
    double r, phi;

    B2DEBUG(200, " Size of cand list before: " << uTrackCandCopy.size());

    while (uTrackCandCopy.size() > 0) {
      auto it = uTrackCandCopy.begin();
      idList  = it->getIdList();
      u_tc    = it->getCoord();
      r       = 1.0 / u_tc.Y();
      phi     = u_tc.X();
      count   = 1;

      bool cancelflag = false;
      while (true) {

        for (auto it_in = (uTrackCandCopy.begin() + 1); it_in != uTrackCandCopy.end(); ++it_in) {
          u_tc_in = it_in->getCoord();
          cancelflag = false;
          if (fabs(u_tc.X() - u_tc_in.X()) < m_mergeThresholdU) {
            phi += u_tc_in.X();
            r += 1.0 / u_tc_in.Y();
            ++count;
            uTrackCandCopy.erase(it_in);
            break;
          }
          cancelflag = true;
        }

        if (cancelflag == true || ((uTrackCandCopy.begin() + 1) == uTrackCandCopy.end())) {
          break;
        }
      }
      if (uTrackCandCopy.size() > 0) {
        uTrackCandCopy.erase(it);
      }

      /* Add to list */
      uTrackCandMerged.push_back(DATCONTrackCand(idList, TVector2(phi / ((double) count), (1.0 / (r / ((double) count))))));
    }

    B2DEBUG(200, "Size of cand list after: " << uTrackCandMerged.size());
    uTrackCand = uTrackCandMerged;
  }

  /* End of u-side trackCand merger */


  /* Begin of v-side trackCand merger */

  if (m_useTrackCandMerger && m_useTrackCandMergerV) {

    TVector2 v_tc, v_tc_in;
    double d, theta;

    B2DEBUG(200, " Size of cand list before: " << vTrackCandCopy.size());

    while (vTrackCandCopy.size() > 0) {
      auto it = vTrackCandCopy.begin();
      idList  = it->getIdList();
      v_tc    = it->getCoord();
      d       = v_tc.Y();
      theta   = v_tc.X();
      count   = 1;

      bool cancelflag = false;
      while (true) {

        for (auto it_in = (vTrackCandCopy.begin() + 1); it_in != vTrackCandCopy.end(); ++it_in) {
          v_tc_in = it_in->getCoord();
          cancelflag = false;
          if (fabs(v_tc.X() - v_tc_in.X()) < m_mergeThresholdV) {
            theta += v_tc_in.X();
            d += v_tc_in.Y();
            ++count;
            vTrackCandCopy.erase(it_in);
            break;
          }
          cancelflag = true;
        }

        if (cancelflag == true || ((vTrackCandCopy.begin() + 1) == vTrackCandCopy.end())) {
          break;
        }
      }
      if (vTrackCandCopy.size() > 0) {
        vTrackCandCopy.erase(it);
      }

      /* Add to list */
      vTrackCandMerged.push_back(DATCONTrackCand(idList, TVector2(theta / ((double) count), d / ((double) count))));

    }

    B2DEBUG(200, "Size of cand list after: " << vTrackCandMerged.size());
    vTrackCand = vTrackCandMerged;
  }

  /* End of v-side trackCand merger */

}

/**
* Merge duplicated tracks (means tracks with difference of m_mergeThreshold).
*/
void
DATCONTrackingModule::trackMerger()
{

  std::vector<DATCONTrack> TracksCopy;
  std::vector<DATCONTrack> TracksMerged;

  int count = 1;
  int trackID = 1;
  int trackCharge = 0, trackCharge_in = 0;
  double trackPhi = 0., trackRadius = 0., trackTheta = 0., trackD = 0.;
  double trackPhi_in = 0., trackRadius_in = 0., trackTheta_in = 0., trackD_in = 0.;
  double PhiAverage = 0., RadiusAverage = 0., ThetaAverage = 0., DAverage = 0.;

  if (storeDATCONTracks.isValid()) {
    storeDATCONTracks.clear();
  }

  TracksCopy = DATCONTracks;

  trackID = 1;

  while (TracksCopy.size() > 0) {
    auto it     = TracksCopy.begin();
    trackPhi    = it->getTrackPhi();
    trackRadius = it->getTrackR();
    trackTheta  = it->getTrackTheta();
    trackD      = it->getTrackd();
    trackCharge = it->getTrackCharge();

    PhiAverage    = trackPhi;
    RadiusAverage = trackRadius;
    ThetaAverage  = trackTheta;
    DAverage      = trackD;

    count = 1;

    bool cancelflag = false;
    while (true) {

      for (auto it_in = (TracksCopy.begin() + 1); it_in != TracksCopy.end(); ++it_in) {
        trackPhi_in    = it_in->getTrackPhi();
        trackRadius_in = it_in->getTrackR();
        trackTheta_in  = it_in->getTrackTheta();
        trackD_in      = it_in->getTrackd();
        trackCharge_in = it_in->getTrackCharge();

        cancelflag = false;
        if (fabs(PhiAverage - trackPhi_in) < m_mergeThresholdPhi && fabs(ThetaAverage - trackTheta_in) < m_mergeThresholdTheta
            && trackCharge == trackCharge_in) {
          PhiAverage    += trackPhi_in;
          RadiusAverage += trackRadius_in;
          ThetaAverage  += trackTheta_in;
          DAverage      += trackD_in;

          ++count;
          TracksCopy.erase(it_in);
          break;
        }
        cancelflag = true;
      }

      if (cancelflag == true || ((TracksCopy.begin() + 1) == TracksCopy.end())) {
        break;
      }
    }

    if (TracksCopy.size() > 0) {
      TracksCopy.erase(it);
    }

    /* Add to list */
    PhiAverage    /= (double)count;
    RadiusAverage /= (double)count;
    ThetaAverage  /= (double)count;
    DAverage      /= (double)count;
    int curvatureSign = -trackCharge;
    TracksMerged.push_back(DATCONTrack(trackID, RadiusAverage, PhiAverage, DAverage, ThetaAverage, trackCharge, curvatureSign));
    storeDATCONTracks.appendNew(DATCONTrack(trackID, RadiusAverage, PhiAverage, DAverage, ThetaAverage, trackCharge, curvatureSign));
    trackID++;

    PhiAverage    = 0.;
    RadiusAverage = 0.;
    ThetaAverage  = 0.;
    DAverage      = 0.;
    count = 1;

  }

  DATCONTracks = TracksMerged;
}
