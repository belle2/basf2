/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 * Contributors: Thomas Hauth, Patrick Ecker                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclTrackBremFinder/ECLTrackBremFinderModule.h>

//Framework
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

//Tracking
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/BremHit.h>

//ECL
#include <ecl/modules/eclTrackBremFinder/BestMatchContainer.h>
#include <ecl/modules/eclTrackBremFinder/BremFindingMatchCompute.h>

//#include <genfit/Exception.h>

using namespace Belle2;

REG_MODULE(ECLTrackBremFinder)

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
}

void ECLTrackBremFinderModule::initialize()
{
  m_eclClusters.isRequired(m_param_eclClustersStoreArrayName);
  m_eclClusters.registerRelationTo(m_eclClusters);

  m_tracks.isRequired(m_param_tracksStoreArrayName);

  m_recoTracks.isRequired();

  m_bremHits.registerInDataStore();
  m_bremHits.registerRelationTo(m_eclClusters);
  m_bremHits.registerRelationTo(m_recoTracks);
}

void ECLTrackBremFinderModule::event()
{


  // todo: only iterate over the RecoTracks which have been identified as e-Tracks
  // either use the Clusters matched to tracks (non-neutral) or use the smarter decision
  // done by the neutral / non-neutral classification code
  // todo: there needs to be a global (all tracks vs. all clusters) conflict resolution,
  // sort tracks by Pt, as high energy tracks are more likely to radiate bremsstrahlung photons
  // with sufficient energy to be detected and reconstructed
  for (auto& track : m_tracks) {

    // since the module runs after the reconstruction the pid likelihood can be checked to sort out pion and kaon tracks
    const PIDLikelihood* pid = track.getRelated<PIDLikelihood>();
    if (pid) {
      int possiblePDGs[6] = {11, 211, 321, 2212, 13, 1000010020};
      int mostLikelyPDG = 0;
      double highestProb = 0;
      for (int pdg : possiblePDGs) {
        Const::ChargedStable chargedStable = Const::ChargedStable(pdg);
        double probability = pid->getProbability(chargedStable);
        if (probability > highestProb) {
          highestProb = probability;
          mostLikelyPDG = pdg;
        }
      }
      if (mostLikelyPDG != 11) {
        B2WARNING("Track is expected to be from particle with pdg " << mostLikelyPDG);
        continue;
      }
    }

    B2DEBUG(20, "Checking track for related ECLCluster");

    // does this track have a cluster assigned ?
    // this is required, otherwise we cannot assign any brems cluster
    ECLCluster* primaryClusterOfTrack = nullptr;
    auto relatedClustersToTrack =
      track.getRelationsWith<ECLCluster>
      (m_param_eclClustersStoreArrayName);       //check the cluster hypothesis ID here (take c_nPhotons hypothesis)!!
    for (auto& relatedCluster : relatedClustersToTrack) {
      auto particleHypothesisID = relatedCluster.getHypothesisId();
      if (particleHypothesisID == ECLCluster::c_nPhotons) {
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

    // possible improvement: use fast lookup using kd-tree, this is nasty
    // iterate over full cluster list to find possible compatible clusters
    for (ECLCluster& cluster : m_eclClusters) {
      //check if the cluster belongs to a photon or electron
      int particleHypothesisID = cluster.getHypothesisId();
      if (particleHypothesisID != ECLCluster::c_nPhotons) {
        B2DEBUG(20, "Cluster has wrong hypothesis!");
        continue;
      }

      //check if cluster is already related to a track -> if true, can't be bremsstrahlung cluster
      auto relatedTrack = cluster.getRelatedFrom<Track>();
      if (relatedTrack) {
        B2DEBUG(20, "Cluster already related to track, bailing out");
        continue;
      }

      //check if the cluster is already related to a primary cluster
      //procedure: first come, first served
      auto relatedCluster = cluster.getRelated<ECLCluster>();
      if (relatedCluster) {
        B2DEBUG(20, "Cluster already related to cluster!");
        continue;
      }

      // if this track has already a relation to a cluster, we cannot consider
      // it for ecl matching
      typedef std::tuple<ECLCluster*, genfit::MeasuredStateOnPlane, double > ClusterMSoPPair;
      BestMatchContainer<ClusterMSoPPair, double> matchContainer;

      // iterate over all track points and see whether this cluster matches
      // the track points direction
      // only VXD hits shall be used
      for (auto hit : recoTrack->getRecoHitInformations(true)) {
        if (hit->getTrackingDetector() == RecoHitInformation::c_PXD || hit->getTrackingDetector() == RecoHitInformation::c_SVD) {
          try {
            if (!recoTrack->hasTrackFitStatus()) {
              continue;
            }
            auto measState = recoTrack->getMeasuredStateOnPlaneFromRecoHit(hit);
            auto bremFinder = BremFindingMatchCompute(m_clusterAcceptanceFactor, cluster, measState);
            if (bremFinder.isMatch()) {
              ClusterMSoPPair match_pair = std::make_tuple(&cluster, measState, hit->getSortingParameter());
              matchContainer.add(match_pair, bremFinder.getDistanceHitCluster());
            }
          } catch (NoTrackFitResult) {
            B2DEBUG(29, "No track fit result available for this hit! Event: " << m_evtPtr->getEvent());
          } catch (genfit::Exception e) {
            B2WARNING("Exception" << e.what());
          }
        }
      }

      // set the params for the virtual hits
      try {
        std::vector<std::pair<float, RecoHitInformation*>> extrapolationParams = {};
        for (auto virtualHitRadius : m_virtualHitRadii) {
          BestMatchContainer<RecoHitInformation*, float> nearestHitContainer;
          for (auto hit : recoTrack->getRecoHitInformations(true)) {
            if (hit->useInFit() && recoTrack->hasTrackFitStatus()) {
              try {
                auto measState = recoTrack->getMeasuredStateOnPlaneFromRecoHit(hit);
                float hitRadius = measState.getPos().Perp();
                float distance = abs(hitRadius - virtualHitRadius);
                nearestHitContainer.add(hit, distance);
              } catch (NoTrackFitResult) {
                B2DEBUG(29, "No track fit result available for this hit! Event: " << m_evtPtr->getEvent());
              }
            }
          }
          if (nearestHitContainer.hasMatch()) {
            auto nearestHit = nearestHitContainer.getBestMatch();
            extrapolationParams.push_back({virtualHitRadius, nearestHit});
          }
        }

        // check for matches of the extrapolation of the virtual hits with the cluster position
        for (auto param : extrapolationParams) {
          auto fitted_state = recoTrack->getMeasuredStateOnPlaneFromRecoHit(param.second);
          try {
            fitted_state.extrapolateToCylinder(param.first);
            auto bremFinder = BremFindingMatchCompute(m_clusterAcceptanceFactor, cluster, fitted_state);
            if (bremFinder.isMatch()) {
              ClusterMSoPPair match_pair = std::make_tuple(&cluster, fitted_state, param.first);
              matchContainer.add(match_pair, bremFinder.getDistanceHitCluster());
            }
          } catch (genfit::Exception& exception1) {
            B2DEBUG(20, "Extrapolation failed!");
          }
        }
      } catch (const genfit::Exception& e) {
        B2WARNING("Exception" << e.what());
      }

      // loop over cluster
      // have we found the best possible track point for this cluster
      if (matchContainer.hasMatch()) {
        auto matchClustermSoP = matchContainer.getBestMatch();
        const auto fitted_state = std::get<1>(matchClustermSoP);

        const auto fitted_pos = fitted_state.getPos();
        const auto fitted_mom = fitted_state.getMom();
        const auto fitted_dir = fitted_state.getDir();

        const auto hit_theta = fitted_mom.Theta();
        const auto hit_phi = fitted_mom.Phi();

        B2DEBUG(20, "Best Cluster" << std::endl
                << " Cluster Phi=" << std::get<0>(matchClustermSoP)->getPhi() << " Theta=" << std::get<0>(matchClustermSoP)->getTheta()
                << " TrackHit Phi=" << hit_phi << " Theta=" << hit_theta);

        // only relate the clusters to each other
        // no relation btw. track point and cluster (yet)
        // add relation to the respective RecoHitInformation of the RecoTrack
        // add sorting parameter to relation, to get information about the place the photon was radiated
        //auto bremCluster = std::get<0>(matchClustermSoP);
        //auto bremFinder = BremFindingMatchCompute(m_clusterAcceptanceFactor, *bremCluster, fitted_state);
        //double clusterDistance = bremFinder.getDistanceHitCluster();
        if (primaryClusterOfTrack) {
          primaryClusterOfTrack->addRelationTo(std::get<0>(matchClustermSoP), std::get<2>(matchClustermSoP));
        }
        auto bremHit = m_bremHits.appendNew(BremHit(recoTrack, std::get<0>(matchClustermSoP),
                                                    fitted_pos, std::get<0>(matchClustermSoP)->getEnergy(),
                                                    0, fitted_mom));
        bremHit->addRelationTo(recoTrack);
        bremHit->addRelationTo(std::get<0>(matchClustermSoP));
      }
    }
  }
}
