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
  TVector2 TrackCandV, TrackCandU;
  double TrackRadius, TrackPhi, TrackTheta, TrackZzero;
  bool all = false; /* combine every track in V and U */

  TVector3 houghMomentum;
  vector<double> positionCovariance(9, 0.);
  vector<double> momentumCovariance(9, 0.);

  if (m_combineAllTrackCands) {
    all = true;
  }
  if (storeDATCONTracks.isValid()) {
    storeDATCONTracks.clear();
  }

  tracks = 0;

  for (auto it = vTrackCand.begin(); it != vTrackCand.end(); ++it) {
    for (auto it_in = uTrackCand.begin(); it_in != uTrackCand.end(); ++it_in) {
      v_idList = it->getIdList();
      u_idList = it_in->getIdList();

      if (compareList(u_idList, v_idList) || all) {

        int curvsign = 0;
        int charge = -curvsign;

        ++tracks;

        TrackCandV  = it->getCoord();
        TrackCandU  = it_in->getCoord();
        TrackRadius     = 1.0 / TrackCandU.Y();
        TrackPhi   = TrackCandU.X();
        TrackTheta = TrackCandV.X();
        TrackZzero     = TrackCandV.Y();

        if (m_usePhase2Simulation) {
          // ATTENTION TODO FIXME : This still has to be implemented!!!
          // So far no phase 2 specific algorithms have been implemented and tested!
          B2WARNING("This mode is not yet implemented, nothing will happen! Return...");
          return;
        } else {  // begin of "else" belonging to "if(m_usePhase2Simulation)"
          if (TrackRadius < 0.0) {
            charge = -1;
            curvsign = -charge;
            TrackPhi = TrackPhi + M_PI / 2.0;
          } else {
            charge = 1;
            curvsign = -charge;
            TrackPhi = (TrackPhi + M_PI / 2.0) - M_PI;
          }
          if (TrackPhi > M_PI) {
            TrackPhi -= 2.0 * M_PI;
          } //else if (TrackPhi < -1.0 * M_PI) {
          if (TrackPhi < -1.0 * M_PI) {
            TrackPhi += 2.0 * M_PI;
          }

          if (TrackCandV.Y() > 0.0) {
            TrackTheta = -1.0 * TrackTheta;
          } else {
            TrackTheta = M_PI - TrackTheta;
          }

          if (TrackTheta < 0.0) {
            TrackTheta += M_PI;
          } else if (TrackTheta > M_PI) {
            TrackTheta -= M_PI;
          }

          if (fabs(TrackRadius) < 5000) {
            storeDATCONTracks.appendNew(DATCONTrack(tracks, TrackRadius, TrackPhi, TrackZzero, TrackTheta, charge, curvsign));
            DATCONTracks.push_back(DATCONTrack(tracks, TrackRadius, TrackPhi, TrackZzero, TrackTheta, charge, curvsign));
          }

          B2Vector3D magField = BFieldManager::getFieldInTesla({0, 0, 0});
          double BFieldStrength = magField.Mag();

          double pX = 0.299792458 * BFieldStrength * TrackRadius * cos(TrackPhi);
          double pY = 0.299792458 * BFieldStrength * TrackRadius * sin(TrackPhi);
          double pZ = 0.299792458 * BFieldStrength * TrackRadius / tan(TrackTheta);

          houghMomentum.SetXYZ(pX, pY, pZ);

          saveHitsToRecoTrack(u_idList, houghMomentum);

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

  RecoTrack* recoTrack = storeDATCONRecoTracks.appendNew(seedPosition, seedMomentum, 1., "", m_storeDATCONSVDClusterName, "", "", "",
                                                         m_storeRecoHitInformationName);

  for (auto it = idList.begin(); it != idList.end(); it++) {
    spacePointIndex = (int)(*it) - (((int)(*it) / 10000) * 10000);

    DATCONSVDSpacePoint* spacepoint = storeDATCONSVDSpacePoints[spacePointIndex];
    vector<SVDCluster> DATCONSVDClusters = spacepoint->getAssignedDATCONSVDClusters();
    for (auto& datconsvdcluster : DATCONSVDClusters) {

      for (auto& svdcluster : storeDATCONSVDCluster) {
        if (svdcluster.getSensorID() == datconsvdcluster.getSensorID() &&
            svdcluster.isUCluster()  == datconsvdcluster.isUCluster()  &&
            svdcluster.getPosition() == datconsvdcluster.getPosition() &&
            svdcluster.getCharge()   == datconsvdcluster.getCharge()   &&
            svdcluster.getSize()     == datconsvdcluster.getSize()) {

          RecoHitInformation* recohitinfo = storeRecoHitInformation.appendNew(&svdcluster, RecoHitInformation::OriginTrackFinder::c_other,
                                            sortingParameter);
          recohitinfo->addRelationTo(&svdcluster);
          svdcluster.addRelationTo(recohitinfo);
          recoTrack->addRelationTo(recohitinfo);
          svdcluster.addRelationTo(recoTrack);
          recoTrack->addSVDHit(&svdcluster, sortingParameter);
          recoTrack->setChargeSeed(svdcluster.getSeedCharge());
        }
      }
      sortingParameter++;
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

    TVector2 TrackCandU, TrackCandU_in;
    double TrackRadius, TrackPhi;
    double inverseTrackRadius;

    while (uTrackCandCopy.size() > 0) {
      auto it             = uTrackCandCopy.begin();
      idList              = it->getIdList();
      TrackCandU          = it->getCoord();
      TrackRadius         = 1.0 / TrackCandU.Y();
      inverseTrackRadius  = TrackCandU.Y();
      TrackPhi            = TrackCandU.X();
      count               = 1;

      bool cancelflag = false;
      while (true) {

        for (auto it_in = (uTrackCandCopy.begin() + 1); it_in != uTrackCandCopy.end(); ++it_in) {
          TrackCandU_in = it_in->getCoord();
          cancelflag = false;
          if (fabs(TrackCandU.X() - TrackCandU_in.X()) < m_mergeThresholdU) {
            TrackPhi    += TrackCandU_in.X();
            TrackRadius += 1.0 / TrackCandU_in.Y();
            inverseTrackRadius += TrackCandU_in.Y();
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
//       uTrackCandMerged.push_back(DATCONTrackCand(idList, TVector2(TrackPhi / ((double) count), (1.0 / (TrackRadius / ((double) count))))));
      uTrackCandMerged.push_back(DATCONTrackCand(idList, TVector2(TrackPhi / ((double) count), inverseTrackRadius / ((double) count))));
    }
    uTrackCand = uTrackCandMerged;
  }
  /* End of u-side trackCand merger */

  /* Begin of v-side trackCand merger */
  if (m_useTrackCandMerger && m_useTrackCandMergerV) {

    TVector2 TrackCandV, TrackCandV_in;
    double TrackZzero, TrackTheta;

    while (vTrackCandCopy.size() > 0) {
      auto it     = vTrackCandCopy.begin();
      idList      = it->getIdList();
      TrackCandV  = it->getCoord();
      TrackZzero  = TrackCandV.Y();
      TrackTheta  = TrackCandV.X();
      count       = 1;

      bool cancelflag = false;
      while (true) {

        for (auto it_in = (vTrackCandCopy.begin() + 1); it_in != vTrackCandCopy.end(); ++it_in) {
          TrackCandV_in = it_in->getCoord();
          cancelflag = false;
          if (fabs(TrackCandV.X() - TrackCandV_in.X()) < m_mergeThresholdV) {
            TrackTheta += TrackCandV_in.X();
            TrackZzero += TrackCandV_in.Y();
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
      vTrackCandMerged.push_back(DATCONTrackCand(idList, TVector2(TrackTheta / ((double) count), TrackZzero / ((double) count))));

    }
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
  double trackPhi = 0., trackRadius = 0., trackTheta = 0., TrackZzero = 0.;
  double trackPhi_in = 0., trackRadius_in = 0., trackTheta_in = 0., TrackZzero_in = 0.;
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
    TrackZzero      = it->getTrackd();
    trackCharge = it->getTrackCharge();

    PhiAverage    = trackPhi;
    RadiusAverage = trackRadius;
    ThetaAverage  = trackTheta;
    DAverage      = TrackZzero;

    count = 1;

    bool cancelflag = false;
    while (true) {

      for (auto it_in = (TracksCopy.begin() + 1); it_in != TracksCopy.end(); ++it_in) {
        trackPhi_in    = it_in->getTrackPhi();
        trackRadius_in = it_in->getTrackR();
        trackTheta_in  = it_in->getTrackTheta();
        TrackZzero_in  = it_in->getTrackd();
        trackCharge_in = it_in->getTrackCharge();

        cancelflag = false;
        if (fabs(PhiAverage - trackPhi_in) < m_mergeThresholdPhi && fabs(ThetaAverage - trackTheta_in) < m_mergeThresholdTheta
            && trackCharge == trackCharge_in) {
          PhiAverage    += trackPhi_in;
          RadiusAverage += trackRadius_in;
          ThetaAverage  += trackTheta_in;
          DAverage      += TrackZzero_in;

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
    count         = 1;

  }

  DATCONTracks = TracksMerged;
}
