/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/VXDTFRedesign/TrackFinderVXDCellOMatModule.h>

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
}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ******************************+ initialize +******************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/



void TrackFinderVXDCellOMatModule::initialize()
{
  m_network.isRequired(m_PARAMNetworkName);
  m_TCs.registerInDataStore(m_PARAMSpacePointTrackCandArrayName, DataStore::c_DontWriteOut);
}



/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *******************************+  beginRun +******************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void TrackFinderVXDCellOMatModule::beginRun()
{

}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *********************************+ event +********************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void TrackFinderVXDCellOMatModule::event()
{
  /**
   * CA: Take the segmentNetwork, loop over all nodes (which are the segments) and ask their connected inner nodes (which are the neighbours) if they are compatible.
   * If yes, allow them to increase their state in the upgrade step. This is in fact a simplified sketch of the CA-algorithm, which will not fully be described here again.
   * After several rounds, each segment has got its final state.
   * The second step in the module then will be to call a findSeeds(segmentNetwork)-function.
   * This function marks all segments as seeds, whose state is "high enough".
   * All these seeds are then allowed to be seed for a tc-collector (e.g the path finder-algorithm from Rudi) to start collecting.
   * The resulting bunch of TrackCands are then stored as SpacePointTrackCands in a storeArray.
   * Whether or not there shall be a simple QI-calculator to be included, we have to decide later on.
   * Alternatively all of them are set onto a value of 0.5, but slightly smeared to suppress issues with the Hopfield NeuralNetwork.
   * */
  m_eventCounter++;

  DirectedNodeNetwork< Segment<TrackNode>, CACell >& segmentNetwork = m_network->accessSegmentNetwork();

  int nRounds = m_cellularAutomaton.apply(segmentNetwork);
  if (nRounds < 0) { B2ERROR("CA failed, skipping event!"); return; }


  unsigned int nSeeds = m_cellularAutomaton.findSeeds(segmentNetwork);
  if (nSeeds == 0) { B2WARNING("TrackFinderVXDCellOMatModule: In Event: " << m_eventCounter << " no seed could be found -> no TCs created!"); return; }


  vector< vector<Segment<TrackNode>*>> collectedPaths = m_pathCollector.findPaths(segmentNetwork);

  B2DEBUG(10, " TrackFinderVXDCellOMat-event" << m_eventCounter <<
          ": CA needed " << nRounds <<
          " for network with " << segmentNetwork.size() <<
          " nodes, which resulted in " << nSeeds <<
          ". Among these the pathCollector found " << m_pathCollector.nTrees <<
          " and " << collectedPaths.size() <<
          " paths while calling its collecting function " << m_pathCollector.nRecursiveCalls <<
          " times and checking " << m_pathCollector.nNodesPassed <<
          " nodes.")


  unsigned int nCreated = m_sptcCreator.createSPTCs(m_TCs, collectedPaths);
  B2DEBUG(10, " TrackFinderVXDCellOMat-event" << m_eventCounter <<
          ": " << nCreated <<
          " TCs created and stored into StoreArray!")


  /** TODO: convert following pseudo-code to real one (findTCs(...) and createSpacePointTrackCand(...) not existing yet -> TODO!)
   * int nRounds = cellularAutomaton.apply(segmentNetwork);
   *
   * if nRounds < 0 { B2ERROR("CA failed, skipping event!"); return;}
   *
   * unsigned int nSeeds = cellularAutomaton.findSeeds(segmentNetwork);
   *
   * if nSeeds == 0 { B2WARNING("TrackFinderVXDCellOMatModule: In Event: " << nEvent << " no seed could be found -> no TCs created!"); return; }
   *
   * unsigned int nTCTrees = 0, nTCs = 0;
   * for (auto* aNode : segmentNetwork) {
   *   if (aNode->getMetaInfo().isSeed() == false) continue;
   *   auto foundTCs = findTCs(aNode); // collects all TCs starting from this seed.
   *
   *   if (foundTCs.isEmpty()) continue;
   *
   *   nTCTrees++;
   *
   *   for (auto& aTC : foundTCs) {
   *     createSpacePointTrackCand(aTC);
   *     nTCs++;
   *   }
   * }
   *
   * B2DEBUG(10, "TrackFinderVXDCellOMatModule: In Event: " << nEvent <<
   *       ": ca Took " << nRounds <<
   *       " rounds and produced " << nSeeds <<
   *       " seeds. This resulted in " <<  nTCTrees <<
   *       " trees of TCs and " << nTCs <<
   *       " TCs created in the end")
   * */


}


/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ********************************+  endRun +******************************** **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/


void TrackFinderVXDCellOMatModule::endRun()
{

}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *******************************+ terminate +******************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/


void TrackFinderVXDCellOMatModule::terminate()
{

}

