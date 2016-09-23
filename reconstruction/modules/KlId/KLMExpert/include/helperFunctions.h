/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * **************************************************************************/

#pragma once


#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>
#include <reconstruction/dataobjects/KlId.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/Exception.h>
#include <utility>
#include <math.h>
#include <TLorentzVector.h>

using namespace Belle2;
using namespace std;

/** Helper functions for all klid modules to improve readability of the code */
namespace KlIdHelpers {

  /** return if MCparticle is beambkg */
  int mcParticleIsBeamBKG(MCParticle* part)
  {
    if (part == nullptr) {
      return 1;
    } else {
      return 0;
    }
  }


  /** return if mc particle is a K_long */
  int mcParticleIsKlong(MCParticle* part)
  {

    if (mcParticleIsBeamBKG(part)) {
      return 0;
    }

    while (!(part -> getMother() == nullptr)) {
      if (part -> getPDG() == 130) {
        return 1;
      }
      part = part -> getMother();
    }
    return 0;
  }


  /** find closest ECL Cluster and its distance */
  pair<ECLCluster*, double> findClosestECLCluster(const TVector3& klmClusterPosition)
  {

    ECLCluster* closestECL = nullptr ;
    double initDistance = 9999999;
    double closestECLAngleDist = 99999999;
    StoreArray<ECLCluster> eclClusters;

    for (ECLCluster& eclcluster : eclClusters) {

      const TVector3& eclclusterPos = eclcluster.getclusterPosition();
      closestECLAngleDist = eclclusterPos.Angle(klmClusterPosition);

      if (closestECLAngleDist < initDistance) {
        //turn ref to pointer so you can check for null
        closestECL = &eclcluster;
      }

    }

    return make_pair(closestECL, closestECLAngleDist);
  }


  /** find nearest KLMCluster, tis distance and the av intercluster distance */
  tuple<const KLMCluster*, double, double> findClosestKLMCluster(const TVector3& klmClusterPosition)
  {

    StoreArray<KLMCluster> klmClusters;
    const KLMCluster* closestKLM = nullptr;
    double closestKLMDist = 99999999;
    double avInterClusterDist = 0;
    double nextClusterDist = 99999999;
    double nKLMCluster = klmClusters.getEntries();

    for (const KLMCluster& nextCluster : klmClusters) {

      const TVector3& nextClusterPos = nextCluster.getClusterPosition();
      const TVector3& clustDistanceVec = nextClusterPos - klmClusterPosition;

      nextClusterDist = clustDistanceVec.Mag2();
      avInterClusterDist = avInterClusterDist + nextClusterDist;

      if ((nextClusterDist < closestKLMDist) and not(nextClusterDist == 0)) {
        closestKLMDist = nextClusterDist ;
        closestKLM = &nextCluster;
      }
    }// for next_cluster

    // normalise avarage inter cluster dist
    if (nKLMCluster) {
      avInterClusterDist = avInterClusterDist / (1. * nKLMCluster);
    } else {
      avInterClusterDist = 0;
    }

    return make_tuple(closestKLM, closestKLMDist, avInterClusterDist);
  }


  /** find nearest genfit track and return it and its distance  */
  tuple<RecoTrack*, double, std::unique_ptr<const TVector3> > findClosestTrack(const TVector3& clusterPosition)
  {
    StoreArray<RecoTrack> genfitTracks;
    double oldDistance = INFINITY;
    RecoTrack* closestTrack = nullptr;
    TVector3 poca = TVector3(0, 0, 0);


    for (RecoTrack& track : genfitTracks) {
      try {
        genfit::MeasuredStateOnPlane state;
        state = track.getMeasuredStateOnPlaneFromLastHit();
        state.extrapolateToPoint(clusterPosition);
        const TVector3& trackPos = state.getPos();

        const TVector3& distanceVecCluster = clusterPosition - trackPos;
        double newDistance = distanceVecCluster.Mag2();

        // overwrite old distance
        if (newDistance < oldDistance) {
          oldDistance = newDistance;
          closestTrack = &track;
          poca = trackPos;
        }

      } catch (genfit::Exception& e) {
      }// try
    }// for gftrack

    if (not closestTrack) {
      return make_tuple(closestTrack, oldDistance, std::unique_ptr<const TVector3>(nullptr));
    } else {
      return make_tuple(closestTrack, oldDistance, std::unique_ptr<const TVector3>(new TVector3(poca)));
    }
  }



}//end namespace
