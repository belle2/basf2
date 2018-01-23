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
#include <pxd/dataobjects/PXDTrueHit.h>

using namespace Belle2;
using ConversionState = std::bitset<2>;

REG_MODULE(RT2SPTCConverter)

RT2SPTCConverterModule::RT2SPTCConverterModule() :
  Module(),
  m_trackSel(nullptr)

{
  setDescription("Module for converting RecoTracks (e.g. from TrackFinderMCTruth) to SpacePointTrackCands.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // input
  addParam("RecoTracksName", m_RecoTracksName, "Name of container of RecoTracks", std::string(""));

  // required for conversion
  addParam("SVDClusters", m_SVDClusterName, "SVDCluster collection name", std::string(""));

  addParam("SVDandPXDSPName", m_SVDAndPXDSPName, "Name of the collection for SVD and PXD SpacePoints. Note: that "
           "both PXD and SVD SpacePoints should be contained in that collection.", std::string(""));

  // optional input
  addParam("SVDSingleClusterSP", m_SVDSingleClusterSPName, "SVD single Cluster SpacePoints collection name.", std::string(""));

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

  addParam("noKickCutsFile", m_noKickCutsFile,
           "TFile that contains the list of cuts to select trainins sample. If parameter left empty NoKickCuts are not applied",
           std::string(""));

  addParam("noKickOutput", m_noKickOutput,
           "If true produce a TFile with some histograms useful to understand behaviour of training sample selection", false);

  addParam("ignorePXDHits", m_ignorePXDHits, "If true no PXD hits will be used when creating the SpacePointTrackCand", bool(false));

  initializeCounters();
}

// ------------------------------ INITIALIZE ---------------------------------------
void RT2SPTCConverterModule::initialize()
{
  B2INFO("RT2SPTCConverter -------------- initialize() ---------------------");
  // initialize Counters
  initializeCounters();

  // check if all required StoreArrays are here
  StoreArray<SVDCluster> SVDClusters; SVDClusters.isRequired(m_SVDClusterName);
  StoreArray<SpacePoint> SpacePoints; SpacePoints.isRequired(m_SVDAndPXDSPName);


  StoreArray<RecoTrack> recoTracks(m_RecoTracksName);
  recoTracks.isRequired(m_RecoTracksName);

  // registering StoreArray for SpacePointTrackCand
  StoreArray<SpacePointTrackCand> spTrackCand(m_SPTCName);
  spTrackCand.registerInDataStore(m_SPTCName, DataStore::c_ErrorIfAlreadyRegistered);

  StoreArray<MCParticle> mcparticles;
  if (mcparticles.isOptional()) {
    m_mcParticlesPresent = true;
  }

  if (m_useTrueHits) {
    StoreArray<SVDTrueHit> SVDTrueHit; SVDTrueHit.isRequired();
  }

  // register Relation to RecoTrack
  spTrackCand.registerRelationTo(recoTracks);

  m_trackSel = new NoKickRTSel(m_noKickCutsFile, m_noKickOutput);

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

    if (m_noKickCutsFile.size() != 0) {
      bool passCut = m_trackSel->trackSelector(recoTrack);
      if (!passCut) {
        m_ncut++;
        continue; //exclude tracks with catastrophic multiple scattering interactions
      } else {
        m_npass++;
      }
    }
    std::pair<std::vector<const SpacePoint*>, ConversionState> spacePointStatePair;

    // the hit informations from the recotrack, the option "true" will result in a sorted vector
    std::vector<RecoHitInformation*> hitInfos = recoTrack.getRecoHitInformations(true);

    // if requested remove the PXD hits
    // NOTE: in RecoTracks there is also a function to get sorted SVD hits only but this uses not the same code as getRecoHitInformations!
    if (m_ignorePXDHits) {
      std::vector<RecoHitInformation*> hitInfos_buff;
      for (std::vector<RecoHitInformation*>::iterator it = hitInfos.begin(); it < hitInfos.end(); it++) {
        if ((*it)->getTrackingDetector() != RecoHitInformation::c_PXD) hitInfos_buff.push_back(*it);
      }
      hitInfos = hitInfos_buff;
    }

    B2DEBUG(20, "New call getSpacePointsFromRecoHitInformationViaTrueHits. Number of hitInfos: " << hitInfos.size());
    B2DEBUG(20, "number of SVD hits in RecoTrack : " << recoTrack.getNumberOfSVDHits());
    B2DEBUG(20, "number of PXD hits in RecoTrack : " << recoTrack.getNumberOfPXDHits());
    B2DEBUG(20, "number of CDC hits in RecoTrack : " << recoTrack.getNumberOfCDCHits());

    if (m_useTrueHits) {
      spacePointStatePair = getSpacePointsFromRecoHitInformationViaTrueHits(hitInfos);
    } else {
      spacePointStatePair = getSpacePointsFromRecoHitInformations(hitInfos);
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
RT2SPTCConverterModule::getSpacePointsFromRecoHitInformationViaTrueHits(std::vector<RecoHitInformation*> hitInfos)
{
  std::vector<const SpacePoint*> finalSpacePoints;
  ConversionState state;


  // loop over all cluster to determine the SpacePoints that define the given RecoTrack.
  for (const RecoHitInformation* hitInfo : hitInfos) {

    // ignore all hits that are not SVD or PXD
    if (hitInfo->getTrackingDetector() != RecoHitInformation::c_SVD && hitInfo->getTrackingDetector() != RecoHitInformation::c_PXD)
      continue;


    const VXDTrueHit* relatedTrueHit = NULL;
    RelationsObject* cluster = NULL;

    // SVD case
    if (hitInfo->getTrackingDetector() == RecoHitInformation::c_SVD) {
      cluster = hitInfo->getRelatedTo<SVDCluster>();
      if (cluster) {
        relatedTrueHit = cluster->getRelatedTo<SVDTrueHit>();
        B2DEBUG(20, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: Number of related SVDTrueHits: " <<
                cluster->getRelationsTo<SVDTrueHit>().size());
      }
    }

    // PXD case
    if (hitInfo->getTrackingDetector() == RecoHitInformation::c_PXD) {
      cluster = hitInfo->getRelatedTo<PXDCluster>();
      if (cluster) {
        relatedTrueHit = cluster->getRelatedTo<PXDTrueHit>();
        B2DEBUG(20, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: Number of related PXDTrueHits: "
                << cluster->getRelationsTo<PXDTrueHit>().size());
      }
    }

    if (!relatedTrueHit) {
      state.set(c_undefinedError);
      B2DEBUG(10, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: TrueHit missing.");
      if (m_skipProblematicCluster) continue;
      else break;
    }

    // NOTE: double cluster SVD SP and PXD SP should be stored in the same StoreArray!
    const SpacePoint* relatedSpacePoint = relatedTrueHit->getRelatedFrom<SpacePoint>(m_SVDAndPXDSPName);

    // special case for the SVD cluster as there is the option for single cluster SP
    if (hitInfo->getTrackingDetector() == RecoHitInformation::c_SVD) {
      if (!relatedSpacePoint && m_useSingleClusterSP) {
        relatedSpacePoint = cluster->getRelatedFrom<SpacePoint>(m_SVDSingleClusterSPName);
        if (!relatedSpacePoint->getRelatedTo<SVDTrueHit>()) relatedSpacePoint = nullptr;
      }
    } // end SVD

    if (!relatedSpacePoint) {
      state.set(c_undefinedError);
      B2DEBUG(10, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: SpacePoint missing.");
      if (m_skipProblematicCluster) continue;
      else break;
    }


    // Prevent adding the same SpacePoint twice as there are 2 clusters per SP for SVD
    if (std::find(finalSpacePoints.begin(), finalSpacePoints.end(), relatedSpacePoint) == finalSpacePoints.end()) {
      finalSpacePoints.push_back(relatedSpacePoint);
    }
  }
  B2DEBUG(10, "RT2SPTCConverter::getSpacePointsFromClustersViaTrueHits: Number of SpacePoints: " << finalSpacePoints.size());
  return std::make_pair(finalSpacePoints, state);
}


std::pair<std::vector<const SpacePoint*>, ConversionState>
RT2SPTCConverterModule::getSpacePointsFromRecoHitInformations(std::vector<RecoHitInformation*> hitInfos)
{
  std::vector<const SpacePoint*> finalSpacePoints; /**< For all SpacePoints in this vector a TrackNode will be created */
  ConversionState state;

  // loop over all cluster to determine the SpacePoints that define the given RecoTrack.
  for (size_t iHit = 0; iHit < hitInfos.size(); ++iHit) {

    // ignore all hits that are not SVD or PXD
    if (hitInfos[iHit]->getTrackingDetector() != RecoHitInformation::c_SVD &&
        hitInfos[iHit]->getTrackingDetector() != RecoHitInformation::c_PXD) continue;


    std::vector<const SpacePoint*> spacePointCandidates; /**< For all SpacePoints in this vector a TrackNode will be created */

    // simple case PXD : there is a one to one relation between cluster and SpacePoint
    if (hitInfos[iHit]->getTrackingDetector() == RecoHitInformation::c_PXD) {
      PXDCluster* pxdCluster = hitInfos.at(iHit)->getRelated<PXDCluster>();
      SpacePoint* relatedPXDSP = NULL;
      if (pxdCluster) relatedPXDSP = pxdCluster->getRelated<SpacePoint>();
      // if found a spacepoint one is already done!
      if (relatedPXDSP) {
        finalSpacePoints.push_back(relatedPXDSP);
        continue;
      }
    } // end PXD case

    // At this point it has to be a SVD cluster, for SVD one has to combine u and v clusters
    SVDCluster* clusterA = hitInfos.at(iHit)->getRelated<SVDCluster>();

    // if it is not PXD it has to be a SVD hit so the cluster has to exist!!
    if (!clusterA) {
      B2WARNING("SVDCluster to hit not found! This should not happen!");
      state.set(c_undefinedError);
      if (m_skipProblematicCluster) continue;
      else break;
    }

    RelationVector<SpacePoint> relatedSpacePointsA = clusterA->getRelationsFrom<SpacePoint>(m_SVDAndPXDSPName);

    SVDCluster* clusterB = NULL;
    if (iHit + 1 < hitInfos.size()) clusterB = hitInfos.at(iHit + 1)->getRelated<SVDCluster>();

    B2DEBUG(20, "RT2SPTCConverter::getSpacePointsFromClusters: Number of SpacePoints related to first cluster: " <<
            relatedSpacePointsA.size());

    // Try to verify SpacePoint by using next cluster to build a U/V pair.
    if (clusterA && clusterB && (clusterA->isUCluster() != clusterB->isUCluster())) {
      auto relatedSpacePointsB = clusterB->getRelationsFrom<SpacePoint>(m_SVDAndPXDSPName);

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
      if (spacePointCandidates.size() == 1) ++iHit; /**< Cluster information already used. Don't use it again. */
    } // end first case

    // Look up if it is part of single cluster SP collection. If no dedicated collection is given the default collection will be tried again!
    if (clusterA && spacePointCandidates.size() != 1 && m_useSingleClusterSP) {

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
  } // end loop hits

  return std::make_pair(finalSpacePoints, state);
}

void RT2SPTCConverterModule::endRun()
{
  B2RESULT("Number of Selected Tracks (NoKickRTSel): " << m_npass);
  B2RESULT("Number of Rejected Tracks (NoKickRTSel): " << m_ncut);

  m_trackSel->produceHistoNoKick();
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
