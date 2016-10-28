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

  addParam("sectorMapName",
           m_PARAMsecMapName,
           "the name of the SectorMap used for this instance.", string("testMap"));

  addParam("printNetworks",
           m_PARAMprintNetworks,
           "If true for each event and each network created a file with a graph is created.", bool(false));

  addParam("strictSeeding",
           m_PARAMstrictSeeding,
           "Regulates if every subset of sufficient length of a path shall be collected as separate path or not (if true, only one path per possibility is collected, if false subsets are collected too.",
           bool(true));

  addParam("removeVirtualIP",
           m_PARAMremoveVirtualIP,
           "If true, the virtual interaction Point will be removed from the track candidates.",
           bool(true));

}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ******************************+ initialize +******************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/



void TrackFinderVXDCellOMatModule::initialize()
{
  m_sectorMap.isRequired();
  bool wasFound = false;
  for (auto& setup : m_sectorMap->getAllSetups()) {
    auto& filters = *(setup.second);

    if (filters.getConfig().secMapName != m_PARAMsecMapName) { continue; }
    B2INFO("TrackFinderVXDCellOMatModule::initialize(): loading mapName: " << m_PARAMsecMapName << ".");
    m_config = filters.getConfig();
    wasFound = true;
    break; // have found our secMap no need for further searching
  }
  if (wasFound == false) B2FATAL("TrackFinderVXDCellOMatModule::initialize(): requested secMapName '" << m_PARAMsecMapName <<
                                   "' does not exist! Can not continue...");

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
   * - Whether or not there shall be a simple QI-calculator to be included, we have to decide later on.
   * - add parameters for:
   * -- seed-threshold (m_cellularAutomaton.findSeeds),
   * -- SPTC-Seed-settings (see sptcCreator for more details)
   * */
  m_eventCounter++;


  DirectedNodeNetwork< Segment<TrackNode>, CACell >& segmentNetwork = m_network->accessSegmentNetwork();

  if (m_PARAMprintNetworks) {
    std::string fileName = m_PARAMsecMapName + "_CA_Ev" + std::to_string(m_eventCounter);
    DNN::printCANetwork<Segment< Belle2::TrackNode>>(segmentNetwork, fileName);
  }


/// apply CA algorithm:
  int nRounds = m_cellularAutomaton.apply(segmentNetwork);
  if (nRounds < 0) { B2ERROR("CA failed, skipping event!"); return; }


/// mark valid Cells as Seeds:
  unsigned int nSeeds = m_cellularAutomaton.findSeeds(segmentNetwork, m_PARAMstrictSeeding);
  if (nSeeds == 0) { B2WARNING("TrackFinderVXDCellOMatModule: In Event: " << m_eventCounter << " no seed could be found -> no TCs created!"); return; }


  /// collect all Paths starting from a Seed:
  auto collectedPaths = m_pathCollector.findPaths(segmentNetwork);


  /// convert paths of directedNodeNetwork-nodes to paths of const SpacePoint*:
  vector< vector <const SpacePoint*> > collectedSpacePointPaths;
  for (auto& aPath : collectedPaths) {
    collectedSpacePointPaths.push_back({});
    vector <const SpacePoint*>& spPath = collectedSpacePointPaths.back();
    for (auto* aNode : *aPath) {
      spPath.push_back(aNode->getEntry().getOuterHit()->spacePoint);
    }
    spPath.push_back(aPath->back()->getEntry().getInnerHit()->spacePoint);
  }


  B2DEBUG(10, " TrackFinderVXDCellOMat-event" << m_eventCounter <<
          ": CA needed " << nRounds <<
          " for network with " << segmentNetwork.size() <<
          " nodes, which resulted in " << nSeeds <<
          ". Among these the pathCollector found " << m_pathCollector.nTrees <<
          " and " << collectedPaths.size() <<
          " paths while calling its collecting function " << m_pathCollector.nRecursiveCalls <<
          " times and checking " << m_pathCollector.nNodesPassed <<
          " nodes.");


  /// convert the raw paths to fullgrown SpacePoinTrackCands
  unsigned int nCreated = m_sptcCreator.createSPTCs(m_TCs, collectedSpacePointPaths, m_PARAMremoveVirtualIP);
  B2DEBUG(10, " TrackFinderVXDCellOMat-event" << m_eventCounter <<
          ": " << nCreated <<
          " TCs created and stored into StoreArray!");

}

