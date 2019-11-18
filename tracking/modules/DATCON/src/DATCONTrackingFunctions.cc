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

#include <tracking/gfbfield/GFGeant4Field.h>

using namespace std;
using namespace Belle2;

/**
* Find and combine into 3D tracks.
*/
void
DATCONTrackingModule::findandcombine3d()
{
  unsigned int tracks;

  TVector3 houghMomentum;

  if (storeDATCONTracks.isValid()) {
    storeDATCONTracks.clear();
  }

  tracks = 0;

  for (auto it = vTrackCand.begin(); it != vTrackCand.end(); ++it) {
    for (auto it_in = uTrackCand.begin(); it_in != uTrackCand.end(); ++it_in) {
      vector<unsigned int> v_idList = it->getIdList();
      vector<unsigned int> u_idList = it_in->getIdList();

      if (compareList(u_idList, v_idList)) {

        int curvsign = 0;
        int charge = -curvsign;

        ++tracks;

        TVector2 TrackCandV  = it->getCoord();
        TVector2 TrackCandU  = it_in->getCoord();
        double TrackRadius = 1.0 / TrackCandU.Y();
        double TrackPhi    = TrackCandU.X();
        double TrackTheta  = TrackCandV.X();
        double TrackZzero  = TrackCandV.Y();

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
  unsigned int sortingParameter = 0;

  TVector3 seedPosition(0.0, 0.0, 0.0);
  TVector3 seedMomentum = momentum;

  RecoTrack* recoTrack = storeDATCONRecoTracks.appendNew(seedPosition, seedMomentum, 1., "", m_storeDATCONSVDClusterName, "", "", "",
                                                         m_storeRecoHitInformationName);

  for (auto it = idList.begin(); it != idList.end(); it++) {
    int spacePointIndex = (int)(*it) - (((int)(*it) / 10000) * 10000);

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
  vector<unsigned int> idList;

  std::vector<DATCONTrackCand> uTrackCandCopy;
  std::vector<DATCONTrackCand> vTrackCandCopy;
  std::vector<DATCONTrackCand> uTrackCandMerged;
  std::vector<DATCONTrackCand> vTrackCandMerged;

  vTrackCandCopy = vTrackCand;
  uTrackCandCopy = uTrackCand;

  /* Begin of u-side trackCand merger */
  if (m_useTrackCandMerger && m_useTrackCandMergerU) {

    while (uTrackCandCopy.size() > 0) {
      auto it             = uTrackCandCopy.begin();
      idList              = it->getIdList();
      TVector2 TrackCandU = it->getCoord();
      double TrackRadius  = 1.0 / TrackCandU.Y();
      double inverseTrackRadius  = TrackCandU.Y();
      double TrackPhi     = TrackCandU.X();
      count               = 1;

      bool cancelflag = false;
      while (true) {

        for (auto it_in = (uTrackCandCopy.begin() + 1); it_in != uTrackCandCopy.end(); ++it_in) {
          TVector2 TrackCandU_in = it_in->getCoord();
          cancelflag = false;
          if (fabs(TrackCandU.X() - TrackCandU_in.X()) < m_mergeThresholdU) {
            TrackPhi    += TrackCandU_in.X();
            TrackRadius += 1.0 / TrackCandU_in.Y();
            inverseTrackRadius += TrackCandU_in.Y();
            ++count;
            uTrackCandCopy.erase(it_in);
            break;
          }
          // cppcheck-suppress redundantAssignment
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
      uTrackCandMerged.push_back(DATCONTrackCand(idList, TVector2(TrackPhi / ((double) count), inverseTrackRadius / ((double) count))));
    }
    uTrackCand = uTrackCandMerged;
  }
  /* End of u-side trackCand merger */

  /* Begin of v-side trackCand merger */
  if (m_useTrackCandMerger && m_useTrackCandMergerV) {

    while (vTrackCandCopy.size() > 0) {
      auto it     = vTrackCandCopy.begin();
      idList      = it->getIdList();
      TVector2 TrackCandV  = it->getCoord();
      double TrackZzero  = TrackCandV.Y();
      double TrackTheta  = TrackCandV.X();
      count       = 1;

      bool cancelflag = false;
      while (true) {

        for (auto it_in = (vTrackCandCopy.begin() + 1); it_in != vTrackCandCopy.end(); ++it_in) {
          TVector2 TrackCandV_in = it_in->getCoord();
          cancelflag = false;
          if (fabs(TrackCandV.X() - TrackCandV_in.X()) < m_mergeThresholdV) {
            TrackTheta += TrackCandV_in.X();
            TrackZzero += TrackCandV_in.Y();
            ++count;
            vTrackCandCopy.erase(it_in);
            break;
          }
          // cppcheck-suppress redundantAssignment
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

  int trackID = 1;

  if (storeDATCONTracks.isValid()) {
    storeDATCONTracks.clear();
  }

  TracksCopy = DATCONTracks;

  trackID = 1;

  while (TracksCopy.size() > 0) {
    auto it     = TracksCopy.begin();
    double trackPhi    = it->getTrackPhi();
    double trackRadius = it->getTrackRadius();
    double trackTheta  = it->getTrackTheta();
    double TrackZzero  = it->getTrackZzero();
    int    trackCharge = it->getTrackCharge();

    double PhiAverage    = trackPhi;
    double RadiusAverage = trackRadius;
    double ThetaAverage  = trackTheta;
    double ZzeroAverage  = TrackZzero;

    int count = 1;

    bool cancelflag = false;
    while (true) {

      for (auto it_in = (TracksCopy.begin() + 1); it_in != TracksCopy.end(); ++it_in) {
        double trackPhi_in    = it_in->getTrackPhi();
        double trackRadius_in = it_in->getTrackRadius();
        double trackTheta_in  = it_in->getTrackTheta();
        double TrackZzero_in  = it_in->getTrackZzero();
        int    trackCharge_in = it_in->getTrackCharge();

        cancelflag = false;
        if (fabs(PhiAverage - trackPhi_in) < m_mergeThresholdPhi && fabs(ThetaAverage - trackTheta_in) < m_mergeThresholdTheta
            && trackCharge == trackCharge_in) {
          PhiAverage    += trackPhi_in;
          RadiusAverage += trackRadius_in;
          ThetaAverage  += trackTheta_in;
          ZzeroAverage  += TrackZzero_in;

          ++count;
          TracksCopy.erase(it_in);
          break;
        }
        // cppcheck-suppress redundantAssignment
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
    ZzeroAverage  /= (double)count;
    int curvatureSign = -trackCharge;
    TracksMerged.push_back(DATCONTrack(trackID, RadiusAverage, PhiAverage, ZzeroAverage, ThetaAverage, trackCharge, curvatureSign));
    storeDATCONTracks.appendNew(DATCONTrack(trackID, RadiusAverage, PhiAverage, ZzeroAverage, ThetaAverage, trackCharge,
                                            curvatureSign));
    trackID++;

    PhiAverage    = 0.;
    RadiusAverage = 0.;
    ThetaAverage  = 0.;
    ZzeroAverage  = 0.;
    count         = 1;

  }

  DATCONTracks = TracksMerged;
}
