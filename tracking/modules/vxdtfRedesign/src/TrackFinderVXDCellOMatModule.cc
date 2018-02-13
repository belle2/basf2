/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Jonas Wagner, Felix Metzner         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/geometry/BFieldManager.h>

#include <tracking/modules/vxdtfRedesign/TrackFinderVXDCellOMatModule.h>
#include <tracking/trackFindingVXD/algorithms/NetworkPathConversion.h>
#include <tracking/trackFindingVXD/segmentNetwork/NodeNetworkHelperFunctions.h>


using namespace std;
using namespace Belle2;

REG_MODULE(TrackFinderVXDCellOMat)


TrackFinderVXDCellOMatModule::TrackFinderVXDCellOMatModule() : Module()
{
  //Set module properties
  setDescription("The TrackFinderVXD Cell-O-Mat module."
                 "\n It uses the output produced by the SegmentNetworkProducerModule to create"
                 "SpacePointTrackCands using a Cellular Automaton algorithm implementation.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("NetworkName",
           m_PARAMNetworkName,
           "name for StoreObjPtr< DirectedNodeNetwork> which contains the networks needed.",
           string(""));

  addParam("SpacePointTrackCandArrayName",
           m_PARAMSpacePointTrackCandArrayName,
           "name for StoreArray< SpacePointTrackCand> to be filled.",
           string(""));

  addParam("printNetworks",
           m_PARAMprintNetworks,
           "If true for each event and each network created a file with a graph is created.", bool(false));

  addParam("strictSeeding",
           m_PARAMstrictSeeding,
           "Regulates if every node with enough notes below it is used as a seed or only the outermost nodes.",
           bool(true));

  addParam("storeSubsets",
           m_PARAMstoreSubsets,
           "Regulates if every subset of sufficient length of a path shall be collected as separate path or not.",
           bool(false));

  addParam("setFamilies",
           m_PARAMsetFamilies,
           "Additionally assign a common family identifier to all Tracks that are share a node.",
           bool(false));

  addParam("selectBestPerFamily",
           m_PARAMselectBestPerFamily,
           "Select only the best track candidate for each family.",
           bool(false));

  addParam("xBestPerFamily",
           m_PARAMxBestPerFamily,
           "Number of best track candidates to be created per family.",
           m_PARAMxBestPerFamily);

  addParam("maxFamilies",
           m_PARAMmaxFamilies,
           "Maximal number of families allowed in an event; if exceeded, the event execution will be skipped.",
           m_PARAMmaxFamilies);

}


void TrackFinderVXDCellOMatModule::initialize()
{
  m_network.isRequired(m_PARAMNetworkName);
  m_TCs.registerInDataStore(m_PARAMSpacePointTrackCandArrayName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);

  if (m_PARAMselectBestPerFamily) {
    m_sptcSelector = std::make_unique<SPTCSelectorXBestPerFamily>(m_PARAMxBestPerFamily);
  }
}


void TrackFinderVXDCellOMatModule::beginRun()
{
  if (m_PARAMselectBestPerFamily) {
    // BField is required by all QualityEstimators
    double bFieldZ = BFieldManager::getFieldInTesla({0, 0, 0}).Z();
    m_sptcSelector->setMagneticFieldForQE(bFieldZ);
  }
}


void TrackFinderVXDCellOMatModule::event()
{
  m_eventCounter++;

  DirectedNodeNetwork< Segment<TrackNode>, CACell >& segmentNetwork = m_network->accessSegmentNetwork();

  /// apply CA algorithm:
  int nRounds = m_cellularAutomaton.apply(segmentNetwork);
  if (nRounds < 0) { B2ERROR("CA failed, skipping event!"); return; }

  if (m_PARAMprintNetworks) {
    std::string fileName = m_PARAMNetworkName + "_CA_Ev" + std::to_string(m_eventCounter);
    DNN::printCANetwork<Segment< Belle2::TrackNode>>(segmentNetwork, fileName);
  }

  /// mark valid Cells as Seeds:
  unsigned int nSeeds = m_cellularAutomaton.findSeeds(segmentNetwork, m_PARAMstrictSeeding);
  if (nSeeds == 0) {
    //B2WARNING("In Event: " << m_eventCounter << " no seed could be found -> no TCs created!");
    return;
  }

  /// mark families
  if (m_PARAMsetFamilies) {
    unsigned short nFamilies = m_familyDefiner.defineFamilies(segmentNetwork);
    if (nFamilies > m_PARAMmaxFamilies)  {
      B2ERROR("Maximal number of track canidates per event was exceeded: Number of Families = " << nFamilies);
      return;
    }
    m_sptcSelector->prepareSelector(nFamilies);
  }

  /// collect all Paths starting from a Seed:
  m_collectedPaths.clear();
  if (not m_pathCollector.findPaths(segmentNetwork, m_collectedPaths, m_PARAMstoreSubsets)) {
    B2ERROR("VXDCellOMat got signal to abort the event.");
    return;
  }


  /// convert paths of directedNodeNetwork-nodes to paths of const SpacePoint*:
  ///  Resulting SpacePointPath contains SpacePoints sorted from the innermost to the outermost.
  for (auto& aPath : m_collectedPaths) {
    SpacePointTrackCand sptc = convertNetworkPath(aPath);

    if (m_PARAMselectBestPerFamily) {
      m_sptcSelector->testNewSPTC(sptc);
    } else {
      std::vector<const SpacePoint*> path = sptc.getHits();
      m_sptcCreator.createSPTC(m_TCs, path, sptc.getFamily());
    }
  }

  /// Create SPTCs in respective StoreArray if family based best candidate selection was performed.
  if (m_PARAMselectBestPerFamily) {
    std::vector<SpacePointTrackCand> bestPaths = m_sptcSelector->returnSelection();
    for (unsigned short iCand = 0; iCand < bestPaths.size(); iCand++) {
      SpacePointTrackCand cand = bestPaths.at(iCand);
      std::vector<const SpacePoint*> path = cand.getHits();
      m_sptcCreator.createSPTC(m_TCs, path, cand.getFamily());
    }
  }
}
