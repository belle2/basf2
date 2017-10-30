/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Jonas Wagner, Felix Metzner         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// fw:
#include <framework/logging/Logger.h>

#include <tracking/modules/vxdtfRedesign/TrackFinderVXDBasicPathFinderModule.h>

#include <tracking/trackFindingVXD/segmentNetwork/NodeNetworkHelperFunctions.h>


using namespace std;
using namespace Belle2;

REG_MODULE(TrackFinderVXDBasicPathFinder)

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ******************************+ constructor +****************************** **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/


TrackFinderVXDBasicPathFinderModule::TrackFinderVXDBasicPathFinderModule() : Module()
{
  //Set module properties
  setDescription("The TrackFinderVXD BasicPathFinder module. \n It uses the output produced by the SegmentNetworkProducerModule to create SpacePointTrackCands by simply storing all possible paths stored in the SegmentNetwork.");
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

  addParam("selectBestPerFamily",
           m_PARAMselectBestPerFamily,
           "Select only the best track candidate for each family.",
           bool(false));

}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ******************************+ initialize +******************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/



void TrackFinderVXDBasicPathFinderModule::initialize()
{
  m_spacePoints.isRequired(m_spacePointsName);

  m_network.isRequired(m_PARAMNetworkName);
  m_TCs.registerInDataStore(m_PARAMSpacePointTrackCandArrayName, DataStore::c_DontWriteOut);

  if (m_PARAMselectBestPerFamily) {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  }
}




/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *********************************+ event +********************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void TrackFinderVXDBasicPathFinderModule::event()
{

  m_eventCounter++;


  DirectedNodeNetwork< Segment<TrackNode>, CACell >& segmentNetwork = m_network->accessSegmentNetwork();

  if (m_PARAMprintNetworks) {
    std::string fileName = m_PARAMsecMapName + "_BasicPF_Ev" + std::to_string(m_eventCounter);
    DNN::printCANetwork<Segment< Belle2::TrackNode>>(segmentNetwork, fileName);
  }


/// apply CA algorithm:
  int nRounds = m_cellularAutomaton.apply(segmentNetwork);
  if (nRounds < 0) { B2ERROR("Basic Path Finder failed, skipping event!"); return; }


/// mark valid Cells as Seeds:
  unsigned int nSeeds = 0;
  for (auto* aNode : segmentNetwork) {
    if (m_PARAMstrictSeeding && !(aNode->getOuterNodes().empty())) continue;
    if (aNode->getInnerNodes().empty()) continue; // mark as seed, if node has got an inner node..

    aNode->getMetaInfo().setSeed(true);
    nSeeds++;
  }
  B2DEBUG(15, "TrackFinderVXDBasicPathFinderModule - event " << m_eventCounter <<
          ": arking seeds is now finished. Of " << segmentNetwork.size() <<
          " cells total -> found " << nSeeds << " seeds");
  if (nSeeds == 0) { B2WARNING("TrackFinderVXDBasicPathFinderModule: In Event: " << m_eventCounter << " no seed could be found -> no TCs created!"); return; }

  // mark families
  if (m_PARAMsetFamilies) {
    unsigned short nFamilies = m_familyDefiner.defineFamilies(segmentNetwork);
    B2DEBUG(10, "Number of families in the network: " << nFamilies);
    if (m_PARAMselectBestPerFamily) {
      m_bestPaths.clear();
      m_familyIndex.clear();
      m_bestPaths.reserve(nFamilies);
      m_familyIndex.resize(nFamilies, -1);
    }
  }

  /// collect all Paths starting from a Seed:
  auto collectedPaths = m_pathCollector.findPaths(segmentNetwork, m_PARAMstoreSubsets);


  /// convert paths of directedNodeNetwork-nodes to paths of const SpacePoint*:
  //  Resulting SpacePointPath contains SpacePoints sorted from the innermost to the outermost.
  short family = -1;
  unsigned short current_index = 0;
  double qi = 0.;

  for (auto& aPath : collectedPaths) {
    vector <const SpacePoint*> spPath;
    spPath.reserve(aPath->size());
    spPath.push_back(aPath->back()->getEntry().getInnerHit()->m_spacePoint);
    for (auto aNodeIt = (*aPath).rbegin(); aNodeIt != (*aPath).rend();  ++aNodeIt) {
      spPath.push_back((*aNodeIt)->getEntry().getOuterHit()->m_spacePoint);
    }
    family = aPath->back()->getFamily();
    if (m_PARAMselectBestPerFamily) {
      SpacePointTrackCand tempSPTC = SpacePointTrackCand(spPath);
      qi = m_estimator->estimateQuality(tempSPTC.getSortedHits());
      if (m_familyIndex.at(family) == -1) {
        m_familyIndex[family] = current_index;
        current_index ++;
        tempSPTC.setQualityIndex(qi);
        m_bestPaths.push_back(tempSPTC);
      } else if (qi > m_bestPaths.at(m_familyIndex[family]).getQualityIndex()) {
        tempSPTC.setQualityIndex(qi);
        m_bestPaths.at(m_familyIndex[family]) = tempSPTC;
      }
    } else {
      m_sptcCreator.createSPTC(m_TCs, spPath, family);
    }
  }

  if (m_PARAMselectBestPerFamily) {
    for (unsigned short fam = 0; fam < m_familyIndex.size(); fam++) {
      std::vector<const SpacePoint*> path = m_bestPaths.at(m_familyIndex[fam]).getHits();
      m_sptcCreator.createSPTC(m_TCs, path, fam);
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
