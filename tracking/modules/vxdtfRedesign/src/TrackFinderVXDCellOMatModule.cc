/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/vxdtfRedesign/TrackFinderVXDCellOMatModule.h>

#include <tracking/trackFindingVXD/segmentNetwork/NodeNetworkHelperFunctions.h>

// fw:
#include <framework/logging/Logger.h>



using namespace std;
using namespace Belle2;

REG_MODULE(TrackFinderVXDCellOMat)

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ******************************+ constructor +****************************** **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/


TrackFinderVXDCellOMatModule::TrackFinderVXDCellOMatModule() : Module()
{
  //Set module properties
  setDescription("The TrackFinderVXD Cell-O-Mat module. \n It uses the output produced by the SegmentNetworkProducerModule to create SpacePointTrackCands using a Cellular Automaton algorithm implementation.");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("NetworkName",
           m_PARAMNetworkName,
           "name for StoreObjPtr< DirectedNodeNetwork> which contains the networks needed.",
           string(""));

  addParam("SpacePointTrackCandArrayName",
           m_PARAMSpacePointTrackCandArrayName,
           "name for StoreArray< SpacePointTrackCand> to be filled.",
           string(""));

  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string(""));

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
}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ******************************+ initialize +******************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/



void TrackFinderVXDCellOMatModule::initialize()
{
  m_spacePoints.isRequired(m_spacePointsName);

  m_network.isRequired(m_PARAMNetworkName);
  m_TCs.registerInDataStore(m_PARAMSpacePointTrackCandArrayName, DataStore::c_DontWriteOut);

}




/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *********************************+ event +********************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void TrackFinderVXDCellOMatModule::event()
{
  /**
   * TODO:
   * - add parameters for:
   * -- seed-threshold (m_cellularAutomaton.findSeeds),
   * */
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
  if (nSeeds == 0) { B2WARNING("TrackFinderVXDCellOMatModule: In Event: " << m_eventCounter << " no seed could be found -> no TCs created!"); return; }

  m_bestPaths.clear();
  m_familyIndex.clear();

  // mark families
  if (m_PARAMsetFamilies) {
    unsigned short nFamilies = m_familyDefiner.defineFamilies(segmentNetwork);
    B2DEBUG(10, "Number of families in the network: " << nFamilies);
    m_bestPaths.reserve(nFamilies);
    m_familyIndex.resize(nFamilies, -1);
  }

  /// collect all Paths starting from a Seed:
  auto collectedPaths = m_pathCollector.findPaths(segmentNetwork, m_PARAMstoreSubsets);


  /// convert paths of directedNodeNetwork-nodes to paths of const SpacePoint*:
  //  Resulting SpacePointPath contains SpacePoints sorted from the innermost to the outermost.
  unsigned short family;
  unsigned short current_index = 0;
  double qi;

  for (auto& aPath : collectedPaths) {
    vector <const SpacePoint*> spPath;
    spPath.reserve(aPath->size());
    spPath.push_back(aPath->back()->getEntry().getInnerHit()->spacePoint);
    for (auto aNodeIt = (*aPath).rbegin(); aNodeIt != (*aPath).rend();  ++aNodeIt) {
      spPath.push_back((*aNodeIt)->getEntry().getOuterHit()->spacePoint);
    }
    family = aPath->back()->getFamily();
    if (m_PARAMsetFamilies) {
      SpacePointTrackCand tempSPTC = SpacePointTrackCand(spPath);
      qi = m_estimator->estimateQuality(tempSPTC.getSortedHits());
      if (m_familyIndex.at(family) == -1) {
        m_familyIndex[family] = current_index;
        current_index ++;
        m_bestPaths.push_back(tempSPTC);
      } else if (qi > m_bestPaths.at(m_familyIndex[family]).getQualityIndex()) {
        tempSPTC.setQualityIndex(qi);
        m_bestPaths.at(m_familyIndex[family]) = tempSPTC;
      }
    } else {
      m_sptcCreator.createSPTCs(m_TCs, spPath, family);
    }
  }

  if (m_PARAMsetFamilies) {
    for (unsigned short fam = 0; fam < m_familyIndex.size(); fam++) {
      std::vector<const SpacePoint*> path = m_bestPaths.at(m_familyIndex[fam]).getHits();
      m_sptcCreator.createSPTCs(m_TCs, path, fam);
    }
  }


  B2DEBUG(10, " TrackFinderVXDCellOMat-event" << m_eventCounter <<
          ": CA needed " << nRounds <<
          " for network with " << segmentNetwork.size() <<
          " nodes, which resulted in " << nSeeds <<
          ". Among these the pathCollector found " << m_pathCollector.nTrees <<
          " and " << collectedPaths.size() <<
          " paths while calling its collecting function " << m_pathCollector.nRecursiveCalls <<
          " times.");
}
