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
#include <mdst/dataobjects/KlId.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>


#include <framework/logging/Logger.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/Exception.h>
#include <utility>
#include <math.h>
#include <TLorentzVector.h>

/** Helper functions for all klid modules to improve readability of the code */
namespace KlId {

  /**get Belle stle track flag */
  int BelleTrackFlag(const Belle2::KLMCluster& cluster, const float angle = 0.26)
  {
    const TVector3& pos = cluster.getClusterPosition();

    Belle2::StoreArray<Belle2::TrackFitResult> tracks;
    for (const Belle2::TrackFitResult& track : tracks) {
      const TVector3& trackPos = track.getPosition();

      if (trackPos.Angle(pos) < angle) {
        B2DEBUG(150, "BelleFlagTracklAngle::" << trackPos.Angle(pos));
        return 1;
      }
    }
    return 0;
  }


  /**get Belle stle ECL flag */
  int BelleECLFlag(const Belle2::KLMCluster& cluster, const float angle = 0.26)
  {
    const TVector3& pos = cluster.getClusterPosition();
    Belle2::StoreArray<Belle2::ECLCluster> eclclusters;

    for (const Belle2::ECLCluster& eclcluster : eclclusters) {

      const TVector3& clusterPos = eclcluster.getClusterPosition();

      if (clusterPos.Angle(pos) < angle) {
        B2DEBUG(150, "BelleFlagECLAngle::" << clusterPos.Angle(pos));
        return 1;
      }
    }
    return 0;
  }

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
    bool stop = false;
    while (!stop) {
      ++hirachy_counter;
      if (part -> getPDG() == 130) {
        return hirachy_counter;
      }
      if ((part -> getMother() == nullptr)) {
        stop = true;
      } else {
        part = part -> getMother();
      }
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
    bool stop = false;
    while (!stop) {
      if (std::abs(part -> getPDG()) == pdg) {
        return true;
      }
      if ((part -> getMother() == nullptr)) {
        stop = true;
      } else {
        part = part -> getMother();
      }
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

    bool stop = false;
    while (!stop) {
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
      if ((part -> getMother() == nullptr)) {
        stop = true;
      } else {
        part = part -> getMother();
      }
    }
    return part ->getPDG();
  }





  /** find closest ECL Cluster and its distance */
  std::pair<Belle2::ECLCluster*, double> findClosestECLCluster(const TVector3& klmClusterPosition)
  {

    Belle2::ECLCluster* closestECL = nullptr ;
    double closestECLAngleDist = 1e10;
    double angularDist = 1e10;
    Belle2::StoreArray<Belle2::ECLCluster> eclClusters;

    if (eclClusters.getEntries() > 0) {
      unsigned int index = 0;
      unsigned int indexOfClosestCluster = 0;
      for (Belle2::ECLCluster& eclcluster : eclClusters) {
        const TVector3& eclclusterPos = eclcluster.getClusterPosition();
        angularDist = eclclusterPos.Angle(klmClusterPosition);
        if (angularDist < closestECLAngleDist) {
          closestECLAngleDist = angularDist;
          // the problem here is one can not just use a refenrence to klmCluster because the next cluster will be written in the same address
          // so that after the loop the reference would always point to the last cluster in the list...
          // if you know a more elegant solution than using the index pls tell me (Jo)
          indexOfClosestCluster = index;
        }
        ++index;
      }
      closestECL = eclClusters[indexOfClosestCluster];
    }
    return std::make_pair(closestECL, closestECLAngleDist);
  }


  /** find nearest KLMCluster, tis distance and the av intercluster distance */
  std::tuple<const Belle2::KLMCluster*, double, double> findClosestKLMCluster(const TVector3& klmClusterPosition)
  {

    Belle2::StoreArray<Belle2::KLMCluster> klmClusters;
    const Belle2::KLMCluster* closestKLM = nullptr;
    double closestKLMDist = 1e10;
    double avInterClusterDist = 0;
    double nextClusterDist = 1e10;
    double nKLMCluster = klmClusters.getEntries();

    if (nKLMCluster > 1) {

      unsigned int index = 0;
      unsigned int indexOfClosestCluster = 0;
      for (const Belle2::KLMCluster& nextCluster : klmClusters) {

        const TVector3& nextClusterPos = nextCluster.getClusterPosition();
        const TVector3& clustDistanceVec = nextClusterPos - klmClusterPosition;

        nextClusterDist = clustDistanceVec.Mag2();
        avInterClusterDist = avInterClusterDist + nextClusterDist;

        if ((nextClusterDist < closestKLMDist) and not(nextClusterDist == 0)) {
          closestKLMDist = nextClusterDist ;
          indexOfClosestCluster = index;
        }
        ++index;
      }// for next_cluster

      closestKLM = klmClusters[indexOfClosestCluster];
      avInterClusterDist = avInterClusterDist / (1. * nKLMCluster);
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
