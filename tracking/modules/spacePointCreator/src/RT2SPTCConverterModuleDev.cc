/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/RT2SPTCConverterModuleDev.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <algorithm> // find

#include <svd/dataobjects/SVDCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>

using namespace Belle2;
using ConversionState = std::bitset<2>;

REG_MODULE(RT2SPTCConverter)

RT2SPTCConverterModule::RT2SPTCConverterModule() :
  Module()
{
  setDescription("Module for converting RecoTracks (e.g. from TrackFinderMCTruth) to SpacePointTrackCands.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // input
  addParam("RecoTracksName", m_RecoTracksName, "Name of container of RecoTracks", std::string(""));

  // required for conversion
  addParam("SVDClusters", m_SVDClusterName, "SVDCluster collection name", std::string(""));

  addParam("SVDDoubleClusterSP", m_SVDDoubleClusterSPName, "SVD Cluster SpacePoints collection name.", std::string(""));

  // optional input
  addParam("SVDSingleClusterSP", m_SVDSingleClusterSPName, "SVD single Cluster SpacePoints collection name", std::string(""));

  // output
  addParam("SpacePointTCName", m_SPTCName,
           "Name of the container under which SpacePointTrackCands will be stored in the DataStore (NOTE: These SpaceTrackCands are not checked for curling behaviour, but are simply converted and stored!)",
           std::string(""));

  // parameters
  addParam("minSP", m_minSP,
           "Minimum number of SpacePoints a SpacePointTrackCand has to contain in order to get registered in the DataStore. If set to 0, any number is accepted",
           0);

  addParam("useTrueHits", m_useTrueHits, "Converts clusters via their relations to TrueHits.", false);

  addParam("skipProblematicCluster", m_skipProblematicCluster,
           "If set to true clusters that could not be converted are skipped instead of throwing away the complete SPTC",
           false);

  addParam("useSingleClusterSP", m_useSingleClusterSP, "Set to true if these SpacePoints should be used as fallback.", false);

  addParam("markRecoTracks", m_markRecoTracks, "If True RecoTracks where conversion problems occurred are marked dirty.", false);

  initializeCounters();
}

// ------------------------------ INITIALIZE ---------------------------------------
void RT2SPTCConverterModule::initialize()
{
  B2INFO("RT2SPTCConverter -------------- initialize() ---------------------");
  // initialize Counters
  initializeCounters();

  // check if all required StoreArrays are here
  StoreArray<SVDCluster>::required(m_SVDClusterName);
  StoreArray<SpacePoint>::required(m_SVDDoubleClusterSPName);


  StoreArray<RecoTrack> recoTracks(m_RecoTracksName);
  recoTracks.required(m_RecoTracksName);

  // registering StoreArray for SpacePointTrackCand
  StoreArray<SpacePointTrackCand> spTrackCand(m_SPTCName);
  spTrackCand.registerPersistent(m_SPTCName);

  StoreArray<MCParticle> mcparticles;
  if (mcparticles.isOptional()) {
    m_mcParticlesPresent = true;
  }

  if (m_useTrueHits) {
    StoreArray<SVDTrueHit>::required();
  }

  // register Relation to RecoTrack
  spTrackCand.registerRelationTo(recoTracks);

}

// ------------------------------------- EVENT -------------------------------------------------------
void RT2SPTCConverterModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(1, "RT2SPTCConverter::event(). Processing event " << eventCounter << " --------");

  StoreArray<RecoTrack> m_recoTracks(m_RecoTracksName);
  StoreArray<SpacePointTrackCand> spacePointTrackCands(m_SPTCName); // output StoreArray

  StoreArray<SVDCluster> svdClusters(m_SVDClusterName);

  for (auto& recoTrack : m_recoTracks) {

    std::pair<std::vector<const SpacePoint*>, ConversionState> spacePointStatePair;
    if (m_useTrueHits) {
      spacePointStatePair = getSpacePointsFromSVDClustersViaTrueHits(recoTrack.getSortedSVDHitList());
    } else {
      spacePointStatePair = getSpacePointsFromSVDClusters(recoTrack.getSortedSVDHitList());
    }
    B2DEBUG(10, "RT2SPTCConverter::event: Number of SpacePoints: " << spacePointStatePair.first.size() << "State: " <<
            spacePointStatePair.second);

    if (int(spacePointStatePair.first.size()) < m_minSP) {
      B2DEBUG(1, "RT2SPTCConverter::event: Not enough number of SpacePoints: " << spacePointStatePair.first.size() << " Required Number: "
              << m_minSP);
      m_minSPCtr++;
      continue; /**< skip this recoTrack */
    }

    if (spacePointStatePair.second.test(c_undefinedError)) {
      if (!m_skipProblematicCluster) {
        m_undefinedErrorCtr++;
        continue;  /**< skip this recoTrack */
      }
      if (m_markRecoTracks) recoTrack.setDirtyFlag();
    }

    SpacePointTrackCand spacePointTC;
    if (m_mcParticlesPresent) {
      MCParticle* mcParticle = recoTrack.getRelatedTo<MCParticle>();
      spacePointTC = SpacePointTrackCand(spacePointStatePair.first, mcParticle->getPDG(), mcParticle->getCharge(),
                                         recoTrack.getArrayIndex());
    } else {
      spacePointTC = SpacePointTrackCand(spacePointStatePair.first, 0, 0, recoTrack.getArrayIndex());
    }

    if (m_useTrueHits) spacePointTC.addRefereeStatus(SpacePointTrackCand::c_checkedTrueHits);

    if (spacePointStatePair.second.test(c_singleCluster)) {
      m_singleClusterUseCtr++;
      spacePointTC.addRefereeStatus(SpacePointTrackCand::c_singleClustersSPs);
      if (m_markRecoTracks) recoTrack.setDirtyFlag();
    }

    // convert momentum and position seed into a single 6D seed
    TVector3 momentumSeed = recoTrack.getMomentumSeed();
    TVector3 positionSeed = recoTrack.getPositionSeed();

    TVectorD seed6D(6);
    seed6D[0] = positionSeed.x();
    seed6D[1] = positionSeed.y();
    seed6D[2] = positionSeed.z();
    seed6D[3] = momentumSeed.Px();
    seed6D[4] = momentumSeed.Py();
    seed6D[5] = momentumSeed.Pz();
    spacePointTC.set6DSeed(seed6D);
    spacePointTC.setCovSeed(recoTrack.getSeedCovariance());

    if (spacePointStatePair.second == ConversionState(0)) {
      m_noFailCtr++;
      if (m_markRecoTracks) recoTrack.setDirtyFlag(false);
    }

    spacePointTrackCands.appendNew(spacePointTC)->addRelationTo(&recoTrack);
  } // end RecoTrack loop
}

std::pair<std::vector<const SpacePoint*>, ConversionState>
RT2SPTCConverterModule::getSpacePointsFromSVDClustersViaTrueHits(std::vector<SVDCluster*> clusters)
{
  std::vector<const SpacePoint*> finalSpacePoints;
  ConversionState state;

  // loop over all cluster to determine the SpacePoints that define the given RecoTrack.
  for (const auto& cluster : clusters) {
    const auto relatedTrueHit = cluster->getRelatedTo<SVDTrueHit>();
    if (!relatedTrueHit) {
      state.set(c_undefinedError);
      B2DEBUG(10, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: TrueHit missing.");
      if (m_skipProblematicCluster) continue;
      else break;
    }
    B2DEBUG(20, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: Number of related TrueHits: " <<
            cluster->getRelationsTo<SVDTrueHit>().size());

    const SpacePoint* relatedSpacePoint = relatedTrueHit->getRelatedFrom<SpacePoint>(m_SVDDoubleClusterSPName);
    if (!relatedSpacePoint && m_useSingleClusterSP) {
      relatedSpacePoint = cluster->getRelatedFrom<SpacePoint>(m_SVDSingleClusterSPName);
      if (!relatedSpacePoint->getRelatedTo<SVDTrueHit>()) relatedSpacePoint = nullptr;
    }
    if (!relatedSpacePoint) {
      state.set(c_undefinedError);
      B2DEBUG(10, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: SpacePoint missing.");
      if (m_skipProblematicCluster) continue;
      else break;
    }

    // Prevent adding the same SpacePoint twice
    if (std::find(finalSpacePoints.begin(), finalSpacePoints.end(), relatedSpacePoint) == finalSpacePoints.end()) {
      finalSpacePoints.push_back(relatedSpacePoint);
    }
  }
  B2DEBUG(10, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: Number of SpacePoints: " << finalSpacePoints.size());
  return std::make_pair(finalSpacePoints, state);
}


std::pair<std::vector<const SpacePoint*>, ConversionState>
RT2SPTCConverterModule::getSpacePointsFromSVDClusters(std::vector<SVDCluster*> clusters)
{
  std::vector<const SpacePoint*> finalSpacePoints; /**< For all SpacePoints in this vector a TrackNode will be created */
  ConversionState state;

  // loop over all cluster to determine the SpacePoints that define the given RecoTrack.
  for (size_t iCluster = 0; iCluster < clusters.size(); ++iCluster) {
    std::vector<const SpacePoint*> spacePointCandidates; /**< For all SpacePoints in this vector a TrackNode will be created */

    SVDCluster* clusterA = clusters.at(iCluster);
    auto relatedSpacePointsA = clusterA->getRelationsFrom<SpacePoint>(m_SVDDoubleClusterSPName);
    B2DEBUG(20, "RT2SPTCConverter::getSpacePointsFromClusters: Number of SpacePoints related to first cluster: " <<
            relatedSpacePointsA.size());

    // Try to verify SpacePoint by using next cluster to build a U/V pair.
    if (iCluster + 1 < clusters.size() && (clusterA->isUCluster() != clusters.at(iCluster + 1)->isUCluster())) {
      SVDCluster* clusterB = clusters.at(iCluster + 1);
      auto relatedSpacePointsB = clusterB->getRelationsFrom<SpacePoint>(m_SVDDoubleClusterSPName);

      // determine intersecting SpacePoints.
      for (const auto& spacePoint : relatedSpacePointsA) {
        for (const auto& spacePointCompare : relatedSpacePointsB) {
          if (spacePoint == spacePointCompare) {
            spacePointCandidates.push_back(&spacePoint);
            break;
          }
        }
      }
      B2DEBUG(20, "RT2SPTCConverter::getSpacePointsFromClusters: Number of intersections with next cluster: " <<
              spacePointCandidates.size());
      // Intersection should resolve to the single SpacePoint that was used to create the recoTrack.
      if (spacePointCandidates.size() == 1) ++iCluster; /**< Cluster information already used. Don't use it again. */
    } // end first case

    // Look up if it is part of the single cluster SP collection. If no dedicated collection is given the default collection will be tried again!
    if (spacePointCandidates.size() != 1 && m_useSingleClusterSP) {

      // look if it as single cluster!
      auto relatedSpacePoints = clusterA->getRelationsFrom<SpacePoint>(m_SVDSingleClusterSPName);
      if (relatedSpacePoints.size() == 1) {
        state.set(c_singleCluster);
        spacePointCandidates.push_back(relatedSpacePoints[0]);
      }
    } // second case

    B2DEBUG(10, "RT2SPTCConverter::getSpacePointsFromClusters: Conversion state is: " << state);

    if (spacePointCandidates.size() != 1) {
      state.set(c_undefinedError);
      if (m_skipProblematicCluster) continue;
      else break;
    }
    finalSpacePoints.push_back(spacePointCandidates.at(0));
  } // end loop cluster

  return std::make_pair(finalSpacePoints, state);
}

// -------------------------------- TERMINATE --------------------------------------------------------
void RT2SPTCConverterModule::terminate()
{
  B2RESULT("RT2SPTCConverter::terminate: Converted " << m_noFailCtr << "Reco Tracks without errors and "
           << m_singleClusterUseCtr << " Tracks were converted with a single Cluster SpacePoint. "
           << m_missingTrueHitCtr << " Tracks were skipped because they contained SpacePoints that had no relations to TrueHits, "
           << m_minSPCtr <<  " Tracks were skipped because they didn't contain enough SpacePoints and for "
           << m_undefinedErrorCtr << " Tracks occurred an undefined error.");
}

