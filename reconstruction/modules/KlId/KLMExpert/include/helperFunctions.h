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

/** Helper functions for all klid modules to improve readability of the code */
namespace KlIdHelpers {

  /** return if MCparticle is beambkg */
  int mcParticleIsBeamBKG(Belle2::MCParticle* part)
  {
    if (part == nullptr) {
      return 1;
    } else {
      return 0;
    }
  }

  /** return the mc hirachy of the klong 0:not a klong 1:final particle, 2: klong is mother etc */
  int mcParticleIsKlong(Belle2::MCParticle* part)
  {
    unsigned int hirachy_counter = 0;
    if (mcParticleIsBeamBKG(part)) {
      return 0;
    }

    while (!(part -> getMother() == nullptr)) {
      ++hirachy_counter;
      if (part -> getPDG() == 130) {
        return hirachy_counter;
      }
      part = part -> getMother();
    }
    return 0;
  }

  /** checks if a cluster is signal under the mcWeightcondition (mcWeight = energy deposition) */
  bool isKLMClusterSignal(const Belle2::KLMCluster& cluster, float mcWeigthCut = 0.66)
  {
    const auto mcParticleWeightPair = cluster.getRelatedToWithWeight<Belle2::MCParticle>();
    Belle2::MCParticle* part        = mcParticleWeightPair.first;
    if (!part) {return false; }
    float mcWeight = mcParticleWeightPair.second;
    if (mcParticleIsKlong(part) && (mcWeight > mcWeigthCut)) {
      return true;
    } else {
      return false;
    }
  }

  /** checks if a cluster is signal under the mcWeightcondition (mcWeight = energy deposition) */
  bool isECLClusterSignal(const Belle2::ECLCluster& cluster, float mcWeigthCut = 0.66)
  {
    const auto mcParticleWeightPair = cluster.getRelatedToWithWeight<Belle2::MCParticle>();
    Belle2::MCParticle* part        = mcParticleWeightPair.first;
    if (!part) {return false; }
    float mcWeight = mcParticleWeightPair.second;
    if (mcParticleIsKlong(part) && (mcWeight > mcWeigthCut)) {
      return true;
    } else {
      return false;
    }
  }


  /** return if mc particle has a certain pdg in the decay chain*/
  int isMCParticlePDG(Belle2::MCParticle* part, int pdg)
  {
    while (!(part -> getMother() == nullptr)) {
      if (std::abs(part -> getPDG()) == pdg) {
        return true;
      }
      part = part -> getMother();
    }
    return false;
  }


  /** return if mc particles primary pdg.
   *  this is very imprecise but sufficient
   *  to understand if the backgrounds are charged,
   *  hadronic or gammas which is whats relevant
   *  for klid investigations.
   * */
  int getPrimaryPDG(Belle2::MCParticle* part)
  {

    if (mcParticleIsBeamBKG(part)) {
      return -999;
    }
    while (!(part -> getMother() == nullptr)) {
      if (isMCParticlePDG(part, 130)) {
        return 130;
      }
      if (isMCParticlePDG(part, 310)) {
        return 310;
      }
      if (isMCParticlePDG(part, 321)) {
        return 321;
      }
      if (isMCParticlePDG(part, 2112)) {
        return 2112;
      }
      if (isMCParticlePDG(part, 2212)) {
        return 2212;
      }
      if (isMCParticlePDG(part, 211)) {
        return 211;
      }
      if (isMCParticlePDG(part, 111)) {
        return 111;
      }
      if (isMCParticlePDG(part, 13)) {
        return 13;
      }
      if (isMCParticlePDG(part, 11)) {
        return 11;
      }
      if (isMCParticlePDG(part, 22)) {
        return 22;
      }
      part = part -> getMother();
    }
    return 0;
  }





  /** find closest ECL Cluster and its distance */
  std::pair<Belle2::ECLCluster*, double> findClosestECLCluster(const TVector3& klmClusterPosition)
  {

    Belle2::ECLCluster* closestECL = nullptr ;
    double initDistance = 9999999;
    double closestECLAngleDist = 99999999;
    Belle2::StoreArray<Belle2::ECLCluster> eclClusters;

    for (Belle2::ECLCluster& eclcluster : eclClusters) {

      const TVector3& eclclusterPos = eclcluster.getClusterPosition();
      closestECLAngleDist = eclclusterPos.Angle(klmClusterPosition);

      if (closestECLAngleDist < initDistance) {
        //turn ref to pointer so you can check for null
        closestECL = &eclcluster;
      }

    }

    return std::make_pair(closestECL, closestECLAngleDist);
  }


  /** find nearest KLMCluster, tis distance and the av intercluster distance */
  std::tuple<const Belle2::KLMCluster*, double, double> findClosestKLMCluster(const TVector3& klmClusterPosition)
  {

    Belle2::StoreArray<Belle2::KLMCluster> klmClusters;
    const Belle2::KLMCluster* closestKLM = nullptr;
    double closestKLMDist = 99999999;
    double avInterClusterDist = 0;
    double nextClusterDist = 99999999;
    double nKLMCluster = klmClusters.getEntries();

    for (const Belle2::KLMCluster& nextCluster : klmClusters) {

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

    return std::make_tuple(closestKLM, closestKLMDist, avInterClusterDist);
  }


  /** find nearest genfit track and return it and its distance  */
  std::tuple<Belle2::RecoTrack*, double, std::unique_ptr<const TVector3> > findClosestTrack(const TVector3& clusterPosition,
      float cutAngle)
  {
    Belle2::StoreArray<Belle2::RecoTrack> genfitTracks;
    double oldDistance = 10000000;//dont wanna use infty cos that kills tmva...
    Belle2::RecoTrack* closestTrack = nullptr;
    TVector3 poca = TVector3(0, 0, 0);


    for (Belle2::RecoTrack& track : genfitTracks) {



      try {
        genfit::MeasuredStateOnPlane state;
        genfit::MeasuredStateOnPlane state_for_cut;
        state_for_cut = track.getMeasuredStateOnPlaneFromFirstHit();

        // only use tracks that are close cos the extrapolation takes ages
        if (clusterPosition.Angle(state_for_cut.getPos()) < cutAngle) {


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
        }
      } catch (genfit::Exception& e) {
      }// try
    }// for gftrack

    if (not closestTrack) {
      return std::make_tuple(closestTrack, oldDistance, std::unique_ptr<const TVector3>(nullptr));
    } else {
      return std::make_tuple(closestTrack, oldDistance, std::unique_ptr<const TVector3>(new TVector3(poca)));
    }
  }

}//end namespace
