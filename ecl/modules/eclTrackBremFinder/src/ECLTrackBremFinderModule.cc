/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackBremFinder/ECLTrackBremFinderModule.h>
#include <ecl/modules/eclTrackBremFinder/BestMatchContainer.h>
#include <ecl/modules/eclTrackBremFinder/BremFinding.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Angle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <set>

#include <TMath.h>
#include "../include/ECLTrackBremFinderModule.h"
#include "../../../../genfit2/code2/core/include/AbsFitterInfo.h"
#include "../../../../genfit2/code2/core/include/Exception.h"

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLTrackBremFinder)

ECLTrackBremFinderModule::ECLTrackBremFinderModule() :
  Module()
{
  setDescription("Use Track direction to pick up possible ECL Brem Cluster");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("clusterAcceptanceFactor", m_clusterAcceptanceFactor,
           "Factor which is multipied onto the cluster position error to check for matches",
           m_clusterAcceptanceFactor);

  addParam("hitPositionCut", m_hitPositionCut, "Cut on the position of the reco hits, so for example CDC hits were ignored",
           m_hitPositionCut);

  addParam("angleCorrection", m_angleCorrection,
           "Shall the angle of the cluster be transformed into the coordinate system of the hits",
           m_angleCorrection);

}

void ECLTrackBremFinderModule::initialize()
{
  StoreArray<ECLCluster> eclClusters;
  eclClusters.registerRelationTo(eclClusters);

  StoreArray<genfit::MeasuredStateOnPlane> mSoPl;
  eclClusters.registerRelationTo(mSoPl);

}

void ECLTrackBremFinderModule::event()
{
  StoreArray<Track> tracks;
  StoreArray<RecoTrack> recoTracks;
  StoreArray<ECLCluster> eclClusters;
  StoreObjPtr<EventMetaData> evtMetaData;
  StoreArray<genfit::MeasuredStateOnPlane> mSoPl;

  B2DEBUG(1, "### Event " << evtMetaData->getEvent());

  // todo: only iterate over the RecoTracks which have been identified as e-Tracks
  // either use the Clusters matched to tracks (non-neutral) or use the smater decision
  // done by the neutral / non-neutral classification code
  // todo: there needs to be a global (all tracks vs. all clusters) conflict resolution,
  // sort tracks by Pt, as high energy tracks are more likely to radiate bremsstrahlung-photons
  // whith suffiecient energy to be detected and reconstructed
  for (auto& track : tracks) {

    B2DEBUG(1, "Checking track for related ECLCluster");

    // does this track have a cluster assigned ?
    // this is required, otherwise we cannot assign any brems cluster
    ECLCluster* primaryClusterOfTrack = nullptr;
    auto relatedClustersToTrack =
      track.getRelationsWith<ECLCluster>();     //check the particle hypothesis ID here (has to be 6 for electron)!!
    for (auto& relatedCluster : relatedClustersToTrack) {
      int particleHypothesisID = relatedCluster.getHypothesisId();
      if (particleHypothesisID == 6) {
        primaryClusterOfTrack = &relatedCluster;
      }
    }
    if (!primaryClusterOfTrack)
      continue;

    // get the RecoTrack to have easy acces to individual hits and
    // their fit state
    auto recoTrack = track.getRelatedTo<RecoTrack>("RecoTracks");

    if (!recoTrack) {
      // no reco track
      B2DEBUG(1, "No RecoTrack for this Track");
      continue;
    }

    // todo: use fast lookup using kd-tree, this is nasty
    // iterate over full cluster list to find possible compatible clusters
    //size_t i = 0;
    for (ECLCluster& cluster : eclClusters) {           //also check particle hypothesis ID here (now should be 5 for photons)!!!

      PhiAngle clusterPhi(cluster.getPhi(), cluster.getUncertaintyPhi());
      ThetaAngle clusterTheta(cluster.getTheta(), cluster.getUncertaintyTheta());

      auto clusterPosition = cluster.getClusterPosition();

      //B2INFO("Checking cluster #" << i << std::endl
      //       << " Cluster Phi=" << clusterPhi.getAngle() << " +- " << clusterPhi.getError()
      //       << " Theta=" << clusterTheta.getAngle() << " +- " << clusterTheta.getError());
      //i++;

      //check if the cluster belongs to a photon or electron
      int particleHypothesisID = cluster.getHypothesisId();
      if (particleHypothesisID == 6) {
        B2DEBUG(1, "Cluster belongs to electron, bailing out");
        continue;
      }

      //check if cluster is already related to a track -> if true, can't be bremsstrahlung cluster
      auto relatedTrack = cluster.getRelatedFrom<Track>();
      if (relatedTrack) {
        B2DEBUG(1, "Cluster already related to track, bailing out");
        continue;
      }

      //check if the cluster is already related to a primary cluster
      //procedure: first come, first served
      auto relatedCluster = cluster.getRelated<ECLCluster>();
      if (relatedCluster) {
        B2DEBUG(1, "Cluster already related to cluster!");
        continue;
      }

      // if this track has already a relation to a cluster, we cannot consider
      // it for ecl matching
      typedef std::tuple<ECLCluster*, genfit::MeasuredStateOnPlane, double > ClusterMSoPPair;
      BestMatchContainer<ClusterMSoPPair, double> matchContainer;

      // iterate over all trackpoints and see whether this cluster matches
      // the trackpoints direction
      genfit::MeasuredStateOnPlane outermostTrackPointFittedState = genfit::MeasuredStateOnPlane();
      genfit::MeasuredStateOnPlane innermostTrackPointFittedState = genfit::MeasuredStateOnPlane();
      double maxRho = 0, minRho = 16;
      bool outermostTrackPointWasSet = false, innermostTrackPointWasSet = false;
      for (auto& track_point : recoTrack->getHitPointsWithMeasurement()) {
        auto fitter_info = track_point->getFitterInfo();

        if (!fitter_info) {
          // not fitter info available for this hit
          B2DEBUG(1, "No fitter info available!");
          continue;
        }
        auto fitted_state = fitter_info->getFittedState();
        auto fitted_pos = fitted_state.getPos();

        if (std::sqrt(fitted_pos.X()*fitted_pos.X() + fitted_pos.Y()*fitted_pos.Y()) > m_hitPositionCut) {
          B2DEBUG(1, "Hit in CDC!");
          continue;
        }

        if (fitted_pos.Mag() > maxRho) {
          outermostTrackPointFittedState = fitted_state;
          maxRho = fitted_pos.Mag();
          outermostTrackPointWasSet = true;
        }

        if (fitted_pos.Mag() < minRho) {
          innermostTrackPointFittedState = fitted_state;
          minRho = fitted_pos.Mag();
          innermostTrackPointWasSet = true;
        }

        BremFinding bremFinder = BremFinding(m_clusterAcceptanceFactor, cluster, fitted_state);

        if (m_angleCorrection) {
          bremFinder.setAngleCorrectionTrue();
        }
        /*
                bremFinder.setECLCluster(cluster);
                bremFinder.setFitterInfo(fitter_info);
                bremFinder.setClusterAcceptanceFactor(m_clusterAcceptanceFactor);
        */
        if (bremFinder.isMatch()) {
          ClusterMSoPPair match_pair = std::make_tuple(&cluster, fitted_state, track_point->getSortingParameter());
          matchContainer.add(match_pair, bremFinder.getDistanceHitCluster());
        }
      }

      //Set a virtual hit at the edge of the cdc and check if the position matches  a cluster position
      if (outermostTrackPointWasSet) {
        auto fitted_state = outermostTrackPointFittedState;
        try {
          fitted_state.extrapolateToCylinder(16.);
        } catch (genfit::Exception& exception1) {
          B2INFO("exception thrown");
          continue;
        }

        BremFinding bremFinder = BremFinding(m_clusterAcceptanceFactor, cluster, fitted_state);
        if (bremFinder.isMatch()) {
          ClusterMSoPPair match_pair = std::make_tuple(&cluster, fitted_state, -2);
          matchContainer.add(match_pair, bremFinder.getDistanceHitCluster());
        }
      }

      if (outermostTrackPointWasSet) {
        auto fitted_state = outermostTrackPointFittedState;
        try {
          fitted_state.extrapolateToCylinder(15.);
        } catch (genfit::Exception& exception1) {
          B2INFO("exception thrown");
          continue;
        }

        BremFinding bremFinder = BremFinding(m_clusterAcceptanceFactor, cluster, fitted_state);
        if (bremFinder.isMatch()) {
          ClusterMSoPPair match_pair = std::make_tuple(&cluster, fitted_state, -3);
          matchContainer.add(match_pair, bremFinder.getDistanceHitCluster());
        }
      }

      //Set a virtual hit at the edge of the beampipe and check if the position matches  a cluster position
      if (innermostTrackPointWasSet) {
        auto fitted_state = innermostTrackPointFittedState;
        try {
          fitted_state.extrapolateToCylinder(1.05);
        } catch (genfit::Exception& exception1) {
          B2INFO("exception thrown");
          continue;
        }

        BremFinding bremFinder = BremFinding(m_clusterAcceptanceFactor, cluster, fitted_state);
        if (bremFinder.isMatch()) {
          ClusterMSoPPair match_pair = std::make_tuple(&cluster, fitted_state, -1);
          matchContainer.add(match_pair, bremFinder.getDistanceHitCluster());
        }
      }

      // loop over cluster
      // have we found the best possible track point for this cluster
      if (matchContainer.hasMatch()) {

        auto matchClustermSoP = matchContainer.getBestMatch();
        /*
                auto fitter_info = matchClustermSoP.second->getFitterInfo();
                if (!fitter_info) {
                  // not fitter info available for this hit
                  continue;
                }
        */
        auto fitted_state = std::get<1>(matchClustermSoP);

        auto fitted_pos = fitted_state.getPos();
        auto fitted_mom = fitted_state.getMom();
        auto fitted_dir = fitted_state.getDir();

        auto hit_theta = fitted_mom.Theta();
        auto hit_phi = fitted_mom.Phi();

        B2DEBUG(1, "Best Cluster" << std::endl
                << " Cluster Phi=" << std::get<0>(matchClustermSoP)->getPhi() << " Theta=" << std::get<0>(matchClustermSoP)->getTheta()
                << " TrackHit Phi=" << hit_phi << " Theta=" << hit_theta);

        // only relate the clusters to each other
        // no relation btw. track point and cluster (yet)
        // add relation to the respective RecoHitInformation of the RecoTrack
        // add sorting parameter to relation, to get information about the place the photon was radiated
        primaryClusterOfTrack->addRelationTo(std::get<0>(matchClustermSoP), std::get<2>(matchClustermSoP));

// newly created relation
        auto thisShouldBePrimaryCluster = std::get<0>(matchClustermSoP)->getRelatedFrom<ECLCluster>();
        B2DEBUG(1, "-> Relation setup " << (thisShouldBePrimaryCluster != nullptr));

      }

    }

  }
}

