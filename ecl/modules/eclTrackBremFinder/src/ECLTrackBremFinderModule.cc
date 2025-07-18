/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclTrackBremFinder/ECLTrackBremFinderModule.h>

/* ECL headers. */
#include <ecl/modules/eclTrackBremFinder/BestMatchContainer.h>
#include <ecl/modules/eclTrackBremFinder/BremFindingMatchCompute.h>

/* Basf2 headers. */
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/BremHit.h>

using namespace Belle2;

REG_MODULE(ECLTrackBremFinder);

ECLTrackBremFinderModule::ECLTrackBremFinderModule() :
  Module()
{
  setDescription("Use Track direction to pick up possible ECL Brem Cluster");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("eclClustersStoreArrayName", m_param_eclClustersStoreArrayName, "StoreArray name of the ECLClusters for brem matching",
           m_param_eclClustersStoreArrayName);

  addParam("tracksStoreArrayName", m_param_tracksStoreArrayName, "StoreArray name of the Tracks for brem matching",
           m_param_tracksStoreArrayName);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray name of the reco tracks used for brem search",
           m_param_recoTracksStoreArrayName);

  addParam("clusterAcceptanceFactor", m_clusterAcceptanceFactor,
           "Factor which is multiplied onto the cluster position error to check for matches",
           m_clusterAcceptanceFactor);

  addParam("virtualHitRadii", m_virtualHitRadii, "Radii where virtual hits for the extrapolation will be generated",
           m_virtualHitRadii);

  addParam("clusterDistanceCut", m_clusterDistanceCut,
           "Cut on the distance between the cluster position angle and the extrapolation angle",
           m_clusterDistanceCut);
}

void ECLTrackBremFinderModule::initialize()
{
  m_eclClusters.isRequired(m_param_eclClustersStoreArrayName);
  m_eclClusters.registerRelationTo(m_eclClusters);

  m_tracks.isRequired(m_param_tracksStoreArrayName);

  const std::string relationName = "Bremsstrahlung";
  m_eclClusters.registerRelationTo(m_tracks, DataStore::c_Event, DataStore::c_WriteOut, relationName);

  m_recoTracks.isRequired();

  m_bremHits.registerInDataStore();
  m_bremHits.registerRelationTo(m_eclClusters);
  m_bremHits.registerRelationTo(m_recoTracks);
}

void ECLTrackBremFinderModule::event()
{
  std::vector<ECLCluster*> eclClusters;
  for (ECLCluster& cluster : m_eclClusters) {
    /* Check if the cluster belongs to a photon or electron. */
    if (!cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
      continue;
    /*
     * Check if the cluster is already related to a track.
     * If true, it can't be a bremsstrahlung cluster.
     */
    const Track* relatedTrack = cluster.getRelatedFrom<Track>();
    if (relatedTrack != nullptr)
      continue;
    eclClusters.push_back(&cluster);
  }


  // either use the Clusters matched to tracks (non-neutral) or use the smarter decision
  // done by the neutral / non-neutral classification code
  // todo: there needs to be a global (all tracks vs. all clusters) conflict resolution,
  // sort tracks by Pt, as high energy tracks are more likely to radiate bremsstrahlung photons
  // with sufficient energy to be detected and reconstructed
  for (auto& track : m_tracks) {

    const TrackFitResult* trackFitResult = track.getTrackFitResult(Const::ChargedStable(211));
    if (trackFitResult) {
      double trackMomentum = trackFitResult->getMomentum().R();
      // if the momentum of the track is higher than 5 GeV, do not use this track
      if (trackMomentum > 5.0) {
        B2DEBUG(20, "Track momentum higher than 5GeV! Track is not used for bremsstrahlung finding");
        continue;
      }
    } else {
      continue;
    }


    B2DEBUG(20, "Checking track for related ECLCluster");

    // searching for an assigned primary cluster
    ECLCluster* primaryClusterOfTrack = nullptr;
    auto relatedClustersToTrack =
      track.getRelationsWith<ECLCluster>
      (m_param_eclClustersStoreArrayName);       //check the cluster hypothesis ID here (take c_nPhotons hypothesis)!!
    for (auto& relatedCluster : relatedClustersToTrack) {
      if (relatedCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        primaryClusterOfTrack = &relatedCluster;
      }
    }

    // get the RecoTrack to have easy access to individual hits and
    // their fit state
    auto recoTrack = track.getRelatedTo<RecoTrack>(m_param_recoTracksStoreArrayName);

    if (!recoTrack) {
      // no reco track
      B2DEBUG(20, "No RecoTrack for this Track");
      continue;
    }

    std::vector<RecoHitInformation*> recoHitInformations =
      recoTrack->getRecoHitInformations(true);

    // set the params for the virtual hits
    std::vector<std::pair<float, RecoHitInformation*>> extrapolationParams = {};
    std::vector<genfit::MeasuredStateOnPlane> extrapolatedStates = {};
    try {
      for (auto virtualHitRadius : m_virtualHitRadii) {
        BestMatchContainer<RecoHitInformation*, float> nearestHitContainer;
        for (RecoHitInformation* hit : recoHitInformations) {
          if (hit->useInFit() && recoTrack->hasTrackFitStatus()) {
            try {
              auto measState = recoTrack->getMeasuredStateOnPlaneFromRecoHit(hit);
              float hitRadius = measState.getPos().Perp();
              float distance = abs(hitRadius - virtualHitRadius);
              // for higher values the extrapolation will be too bad
              if (distance < 3) {
                nearestHitContainer.add(hit, distance);
              }
            } catch (NoTrackFitResult&) {
              B2DEBUG(29, "No track fit result available for this hit! Event: " << m_evtPtr->getEvent());
            }
          }
          if (nearestHitContainer.hasMatch()) {
            auto nearestHit = nearestHitContainer.getBestMatch();
            extrapolationParams.push_back({virtualHitRadius, nearestHit});
          }
        }
      }

      for (auto param : extrapolationParams) {
        auto fitted_state = recoTrack->getMeasuredStateOnPlaneFromRecoHit(param.second);
        try {
          fitted_state.extrapolateToCylinder(param.first);
          extrapolatedStates.push_back(fitted_state);
        } catch (genfit::Exception& exception1) {
          B2DEBUG(20, "Extrapolation failed!");
        }
      }
    } catch (const genfit::Exception& e) {
      B2WARNING("Exception" << e.what());
    }

    // possible improvement: use fast lookup using kd-tree, this is nasty
    // iterate over full cluster list to find possible compatible clusters
    for (ECLCluster* cluster : eclClusters) {

      //check if the cluster is already related to a BremHit
      //procedure: first come, first served
      auto relatedBremHit = cluster->getRelated<BremHit>();
      if (relatedBremHit) {
        B2DEBUG(20, "Cluster already assumed to be bremsstrahlung cluster!");
        continue;
      }

      typedef std::tuple<ECLCluster*, genfit::MeasuredStateOnPlane, double, double > ClusterMSoPPair;
      BestMatchContainer<ClusterMSoPPair, double> matchContainer;

      // iterate over all track points and see whether this cluster matches
      // the track points direction
      // only VXD hits shall be used
      for (RecoHitInformation* hit : recoHitInformations) {
        if (hit->getTrackingDetector() == RecoHitInformation::c_PXD || hit->getTrackingDetector() == RecoHitInformation::c_SVD) {
          try {
            if (!recoTrack->hasTrackFitStatus()) {
              continue;
            }
            auto measState = recoTrack->getMeasuredStateOnPlaneFromRecoHit(hit);
            auto bremFinder = BremFindingMatchCompute(m_clusterAcceptanceFactor, cluster, measState);
            if (bremFinder.isMatch()) {
              ClusterMSoPPair match_pair = std::make_tuple(cluster, measState, bremFinder.getDistanceHitCluster(),
                                                           bremFinder.getEffAcceptanceFactor());
              matchContainer.add(match_pair, bremFinder.getDistanceHitCluster());
            }
          } catch (NoTrackFitResult&) {
            B2DEBUG(29, "No track fit result available for this hit! Event: " << m_evtPtr->getEvent());
          } catch (genfit::Exception& e) {
            B2WARNING("Exception" << e.what());
          }
        }
      }

      // check for matches of the extrapolation of the virtual hits with the cluster position
      for (auto fitted_state : extrapolatedStates) {
        auto bremFinder = BremFindingMatchCompute(m_clusterAcceptanceFactor, cluster, fitted_state);
        if (bremFinder.isMatch()) {
          ClusterMSoPPair match_pair = std::make_tuple(cluster, fitted_state, bremFinder.getDistanceHitCluster(),
                                                       bremFinder.getEffAcceptanceFactor());
          matchContainer.add(match_pair, bremFinder.getDistanceHitCluster());
        }
      }


      // have we found the best possible track point for this cluster
      if (matchContainer.hasMatch()) {
        auto matchClustermSoP = matchContainer.getBestMatch();
        const auto fitted_state = std::get<1>(matchClustermSoP);

        const auto fitted_pos = ROOT::Math::XYZVector(fitted_state.getPos());
        const auto fitted_mom = ROOT::Math::XYZVector(fitted_state.getMom());

        const auto hit_theta = fitted_mom.Theta();
        const auto hit_phi = fitted_mom.Phi();

        B2DEBUG(20, "Best Cluster" << std::endl
                << " Cluster Phi=" << std::get<0>(matchClustermSoP)->getPhi() << " Theta=" << std::get<0>(matchClustermSoP)->getTheta()
                << " TrackHit Phi=" << hit_phi << " Theta=" << hit_theta);

        // create a BremHit if a match is found
        // relate this BremHit to the bremsstrahlung cluster and the recoTrack
        // if the track has a primary cluster, add a relation between bremsstrahlung cluster and primary cluster

        double effAcceptanceFactor = std::get<3>(matchClustermSoP);
        ECLCluster* bremCluster = std::get<0>(matchClustermSoP);
        double clusterDistance = std::get<2>(matchClustermSoP);

        if (fitted_pos.Rho() <= 16 && clusterDistance <= m_clusterDistanceCut) {
          m_bremHits.appendNew(recoTrack, bremCluster,
                               fitted_pos, bremCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons),
                               clusterDistance, effAcceptanceFactor);

          // add a relation between the bremsstrahlung cluster and the track to transfer the information to the analysis
          // set the acceptance factor as weight
          bremCluster->addRelationTo(&track, effAcceptanceFactor, "Bremsstrahlung");

          if (primaryClusterOfTrack) {
            primaryClusterOfTrack->addRelationTo(bremCluster, effAcceptanceFactor);
          }
        }
      }
    }
  }
}
