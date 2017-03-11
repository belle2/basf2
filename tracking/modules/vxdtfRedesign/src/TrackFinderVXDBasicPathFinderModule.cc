/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/vxdtfRedesign/TrackFinderVXDBasicPathFinderModule.h>

#include <tracking/trackFindingVXD/segmentNetwork/NodeNetworkHelperFunctions.h>

// fw:
#include <framework/logging/Logger.h>



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

  //Relations SpacePoints and SpacePointTCs:
  m_TCs.registerRelationTo(m_spacePoints, DataStore::c_Event, DataStore::c_DontWriteOut);
  m_spacePoints.registerRelationTo(m_TCs, DataStore::c_Event, DataStore::c_DontWriteOut);
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
    //DNN::printCANetwork<Segment< Belle2::TrackNode>>(segmentNetwork, fileName);
  }


/// apply CA algorithm:
  //int nRounds = m_cellularAutomaton.apply(segmentNetwork);
  //if (nRounds < 0) { B2ERROR("Basic Path Finder failed, skipping event!"); return; }


/// mark valid Cells as Seeds:
  unsigned int nSeeds = 0;
  for (auto& aNodIter : segmentNetwork.getNodes()) {
    auto aNode = aNodIter.second;
    if (m_PARAMstrictSeeding && !(aNode->getOuterNodes().empty())) continue;
    if (aNode->getInnerNodes().empty()) continue; // mark as seed, if node has got an inner node..

    aNode->getMetaInfo().setSeed(true);
    nSeeds++;
  }
  B2DEBUG(15, "TrackFinderVXDBasicPathFinderModule - event " << m_eventCounter <<
          ": arking seeds is now finished. Of " << segmentNetwork.size() <<
          " cells total -> found " << nSeeds << " seeds");
  if (nSeeds == 0) { B2WARNING("TrackFinderVXDBasicPathFinderModule: In Event: " << m_eventCounter << " no seed could be found -> no TCs created!"); return; }


  /// collect all Paths starting from a Seed:
  auto collectedPaths = m_pathCollector.findPaths(segmentNetwork, m_PARAMstoreSubsets);


  /// convert paths of directedNodeNetwork-nodes to paths of const SpacePoint*:
  //  Resulting SpacePointPath contains SpacePoints sorted from the innermost to the outermost.
  vector< vector <const SpacePoint*> > collectedSpacePointPaths;
  collectedSpacePointPaths.reserve(collectedPaths.size());
  for (auto& aPath : collectedPaths) {
    vector <const SpacePoint*> spPath;
    spPath.push_back(aPath->back()->getEntry().getInnerHit()->spacePoint);
    for (auto aNodeIt = (*aPath).rbegin(); aNodeIt != (*aPath).rend();  ++aNodeIt) {
      spPath.push_back((*aNodeIt)->getEntry().getOuterHit()->spacePoint);
    }
    collectedSpacePointPaths.push_back(spPath);
  }


  B2DEBUG(10, " TrackFinderVXDCellOMat-event" << m_eventCounter <<
          " for network with " << segmentNetwork.size() <<
          " nodes, which resulted in " << nSeeds <<
          ". Among these the pathCollector found " << m_pathCollector.nTrees <<
          " and " << collectedPaths.size() <<
          " paths while calling its collecting function " << m_pathCollector.nRecursiveCalls <<
          " times.");


  /// convert the raw paths to fullgrown SpacePoinTrackCands
  unsigned int nCreated = m_sptcCreator.createSPTCs(m_TCs, collectedSpacePointPaths);
  B2DEBUG(10, " TrackFinderVXDCellOMat-event" << m_eventCounter <<
          ": " << nCreated <<
          " TCs created and stored into StoreArray!");

}
