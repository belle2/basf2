/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/vxdtfRedesign/SegmentNetworkProducerModule.h>

#include <tracking/trackFindingVXD/segmentNetwork/NodeNetworkHelperFunctions.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SegmentNetworkProducer)

SegmentNetworkProducerModule::SegmentNetworkProducerModule() : Module()
{
  InitializeCounters();

  vector<string> spacePointArrayNames = {""};

  vector<double> ipCoords = {0, 0, 0};

  vector<double> ipErrors = {0.2, 0.2, 1.};

  //Set module properties
  setDescription("The segment network producer module. \n This module takes a given sectorMap and storeArrays of spacePoints and creates a segmentNetwork (if activated).\nThe output: a StoreObjPtr of DirectedNodeNetworkContainer:\n - will contain a DirectedNodeNetwork< ActiveSector> (if parameter 'createNeworks' is [1;3])\n - will contain a DirectedNodeNetwork< SpacePoint > (if parameter 'createNeworks' is  [2;3])\n - will contain a DirectedNodeNetwork< Segment > (if parameter 'createNeworks' is [3])");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("CreateNeworks",
           m_PARAMCreateNeworks,
           "The output: a StoreObjPtr of DirectedNodeNetworkContainer:\n - will contain a DirectedNodeNetwork<ActiveSector> (if parameter 'createNeworks' is [1;3])\n - will contain a DirectedNodeNetwork<SpacePoint> (if parameter 'createNeworks' is  [2;3])\n - will contain a DirectedNodeNetwork<Segment> (if parameter 'createNeworks' is [3])",
           int(3));

  addParam("SpacePointsArrayNames",
           m_PARAMSpacePointsArrayNames,
           "contains names for storeArrays with spacePoints in it (add at least one)",
           spacePointArrayNames);

//   addParam("StaticSectorMapName",
//       m_PARAMStaticSectorMapName,
//       " sets the name of expected StoreArray with SpacePointTrackCand in it",
//       string("")); // TODO add real SecMap!

  addParam("NetworkOutputName",
           m_PARAMNetworkOutputName,
           "defines the unique name given to the output of this module - WARNING two instances of this module with the same name set in this parameter will abort the run!",
           string(""));

  addParam("addVirtualIP",
           m_PARAMAddVirtualIP,
           "if true, to the given SpacePoints a virtual interaction point at given coordinates with parameter 'virtualIPCoorindates' will be added",
           bool(true));

  addParam("virtualIPCoorindates",
           m_PARAMVirtualIPCoordinates,
           "excpects X, Y, and Z coordinates for virtual IP in global coordinates (only lists with 3 coordinates are allowed!). Only used if addVirtualIP == true",
           ipCoords);

  addParam("virtualIPErrors",
           m_PARAMVirtualIPErrors,
           "excpects errors for X, Y, and Z coordinates for virtual IP in global coordinates (only lists with 3 entries are allowed!). Only used if addVirtualIP == true",
           ipErrors);

  addParam("sectorMapName",
           m_PARAMsecMapName,
           "the name of the SectorMap used for this instance.", string("testMap"));

  addParam("printNetworks",
           m_PARAMprintNetworks,
           "If true for each event and each network created a file with a graph is created.", bool(false));

  addParam("allFiltersOff",
           m_PARAMallFiltersOff,
           "For debugging purposes: if true, all filters are deactivated for all hit-combinations and therefore all combinations are accepted.",
           bool(false));
}



void SegmentNetworkProducerModule::event()
{
  m_eventCounter++;
  B2INFO("\n" << "SegmentNetworkProducerModule:event: event " << m_eventCounter << "\n");

  // make sure that network exists:
  if (! m_network) {
    m_network.create();
    B2INFO("As no network (DirectedNodeNetworkContainer) was present, a new network was created");
  }

  vector< RawSectorData > collectedData = matchSpacePointToSectors();

  buildActiveSectorNetwork(collectedData);

  if (m_PARAMCreateNeworks < 2) { B2DEBUG(10, "SegmentNetworkProducerModule:event: event " << m_eventCounter << ": finished work after creating activeSectorNetwork"); return; }

  buildTrackNodeNetwork(); // apply-two-hit-filters

  if (m_PARAMCreateNeworks < 3) { B2DEBUG(10, "SegmentNetworkProducerModule:event: event " << m_eventCounter << ": finished work after creating trackNodeNetwork"); return; }

  buildSegmentNetwork(); // apply-three-hit-filters

  // TODO debug output with counters!
}



void SegmentNetworkProducerModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("SegmentNetworkProducerModule:endRun: events: " << m_eventCounter << " and invEvents: " << invEvents);
  B2WARNING("SegmentNetworkProducerModule:endRun: events: " << m_eventCounter << ", results:\n "
            << "matchSpacePoints-nSPsFound/nSPsLost/nRawSectorsFound: " << m_nSPsFound << "/" << m_nSPsLost << "/" << m_nRawSectorsFound << "\n"
            << ", buildActiveSectorNetwork-nBadSector InnerNotActive/NoInnerActive/NoInnerExisting: " << m_nBadSectorInnerNotActive << "/" <<
            m_nBadSectorNoInnerActive << "/" << m_nBadSectorNoInnerExisting << ", nGoodSectors/nSectorsLinked: " << m_nGoodSectorsFound << "/"
            << m_nSectorsLinked << "\n"
            << ", buildTrackNodeNetwork-nTrackNodesAccepted/nTrackNodesRejected/nTrackNodeLinksCreated: " << m_nTrackNodesAccepted << "/" <<
            m_nTrackNodesRejected << "/" << m_nTrackNodeLinksCreated << "\n"
            << ", buildSegmentNetwork-nSegmentsAccepted/nSegmentsRejected/nSegmentLinksCreated: " << m_nSegmentsAccepted << "/" <<
            m_nSegmentsRejected << "/" << m_nSegmentsLinksCreated << "\n");
}





/** for each SpacePoint given, find according sector and store them in a fast and intermediate way* */
std::vector< SegmentNetworkProducerModule::RawSectorData > SegmentNetworkProducerModule::matchSpacePointToSectors()
{
  std::vector< RawSectorData > collectedData; // contains the raw sectors to be activated
  std::vector<TrackNode* >& trackNodes = m_network->accessTrackNodes(); // collects trackNodes
  int nSPsFound = 0, nSPsLost = 0;

  for (StoreArray<SpacePoint>& storeArray : m_spacePoints) {

    // match all SpacePoints with the sectors:
    for (SpacePoint& aSP : storeArray) {

      const StaticSectorType* sectorFound = findSectorForSpacePoint(aSP);

      if (sectorFound == nullptr) {
        B2WARNING("matchSpacePointToSectors: SP in sensor " << aSP.getVxdID() << " no sector found, SpacePoint discarded!");
        nSPsLost++;
        continue;
      }
      nSPsFound++;

      TrackNode* trackNode = new TrackNode();
      trackNode->spacePoint = &aSP;
      trackNodes.push_back(trackNode);

      // sector for SpacePoint exists:
      FullSecID foundSecID = sectorFound->getFullSecID();
      B2DEBUG(5, "matchSpacePointToSectors: SP found!: " << foundSecID.getFullSecString()); // TODO Jan8_2016: remove!

      vector<RawSectorData>::iterator iter =
        std::find_if(
          collectedData.begin(),
          collectedData.end(),
          [&](const RawSectorData & entry) -> bool
      { return entry.secID == foundSecID; }
        );

      // if secID not in collectedData:
      if (iter == collectedData.end()) {
        collectedData.push_back({ foundSecID , false, nullptr, sectorFound, {trackNode}});
      } else {
        iter->hits.push_back(trackNode);
      }
//    B2INFO("matchSpacePointToSectors: trackNode Found2: " << *trackNode); // TODO Jan8_2016: remove!

    } // loop over SpacePoints in StoreArray
  } // loop over StoreArrays

  B2DEBUG(1, "SegmentNetworkProducerModule::matchSpacePointToSectors() (ev " << m_eventCounter << "): nSPs found/lost: " << nSPsFound
          << "/" << nSPsLost <<
          ", nTrackNodes/nCollectedData(nSectors activated): " << trackNodes.size() << "/" << collectedData.size());
  m_nSPsFound += nSPsFound;
  m_nSPsLost += nSPsLost;
  m_nRawSectorsFound += collectedData.size();
  // store IP-coordinates
  if (m_PARAMAddVirtualIP == true) {
    m_network->setVirtualInteractionPoint(m_virtualIPCoordinates, m_virtualIPErrors);
    TrackNode* vIP = m_network->getVirtualInteractionPoint();
    const StaticSectorType* sectorFound = findSectorForSpacePoint((vIP->getHit()));
    collectedData.push_back({FullSecID(), false, nullptr, sectorFound, {vIP}}); // TODO: which FullSecID for the vIP?
  }

  return collectedData;
}



/** build a DirectedNodeNetwork< ActiveSector >, where all ActiveSectors are stored which have SpacePoints* and compatible inner- or outer neighbours */
void SegmentNetworkProducerModule::buildActiveSectorNetwork(std::vector< SegmentNetworkProducerModule::RawSectorData >&
                                                            collectedData)
{
  // access (yet) empty activeSectorNetwork:
  DirectedNodeNetwork<ActiveSector<StaticSectorType, TrackNode>, VoidMetaInfo>& activeSectorNetwork =
    m_network->accessActiveSectorNetwork();
  // activeSectors are to be stored separately:
  vector<ActiveSector<StaticSectorType, TrackNode>*>& activeSectors = m_network->accessActiveSectors();
  int nNoSPinThisInnerSector = 0, nNoValidOfAllInnerSectors = 0, nNoInnerExisting = 0, nSectorsLinked = 0;

  // loop over all raw sectors found so far:
  for (RawSectorData& outerSectorData : collectedData) {
    ActiveSector<StaticSectorType, TrackNode>* outerSector = new ActiveSector<StaticSectorType, TrackNode>
    (outerSectorData.staticSector);

    // skip double-adding of nodes into the network after first time found -> speeding up the code:
    bool wasAnythingFoundSoFar = false;
    // find innerSectors of outerSector and add them to the network:
    const std::vector<FullSecID>& innerSecIDs = outerSector->getInner2spSecIDs();

    for (const FullSecID innerSecID : innerSecIDs) {
      vector<RawSectorData>::iterator innerRawSecPos =
        std::find_if(
          collectedData.begin(),
          collectedData.end(),
          [&](const RawSectorData & entry) -> bool
//       { return (entry.wasCreated == false) and (entry.secID == innerSecID); }
      { return (entry.secID == innerSecID); }
        );

      // current inner sector has no SpacePoints in this event:
      if (innerRawSecPos == collectedData.end()) { B2DEBUG(5, "SegmentNetworkProducerModule::buildActiveSectorNetwork() (ev " << m_eventCounter << "): sector " << outerSectorData.secID.getFullSecString() << " had inner sector " << innerSecID.getFullSecString() << " but no inner hits - discarding combination..."); nNoSPinThisInnerSector++; continue; }

      // take care of inner sector first:
      ActiveSector<StaticSectorType, TrackNode>* innerSector = nullptr;
      if (innerRawSecPos->wasCreated) { // was already there
        innerSector = innerRawSecPos->sector;
      } else {
        innerSector = new ActiveSector<StaticSectorType, TrackNode>(innerRawSecPos->staticSector);
        innerRawSecPos->wasCreated = true;
        innerRawSecPos->sector = innerSector;
        for (Belle2::TrackNode* hit : innerRawSecPos->hits) { hit->sector = innerSector; }
        // add all SpacePoints of this sector to ActiveSector:
        innerSector->addHits(innerRawSecPos->hits);
        activeSectors.push_back(innerSector);
      }

      // when accepting combination the first time, take care of outer sector:
      if (!wasAnythingFoundSoFar) {
        outerSectorData.wasCreated = true;
        outerSectorData.sector = outerSector;
        for (Belle2::TrackNode* hit : outerSectorData.hits) { hit->sector = outerSector; }
        // add all SpacePoints of this sector to ActiveSector:
        outerSector->addHits(outerSectorData.hits);
        activeSectors.push_back(outerSector);

        B2DEBUG(5, "SegmentNetworkProducerModule::buildActiveSectorNetwork(): sector " << outerSector->getName() <<
                " will be combined with inner sector " << innerSector->getName());

        activeSectorNetwork.linkTheseEntries(*outerSector, *innerSector);
        wasAnythingFoundSoFar = true;
        nSectorsLinked++;
        continue;
      }
      activeSectorNetwork.addInnerToLastOuterNode(*innerSector);
    } // inner sector loop

    // discard outerSector if no valid innerSector could be found
    if (wasAnythingFoundSoFar == false) { B2DEBUG(5, "SegmentNetworkProducerModule::buildActiveSectorNetwork(): sector " << outerSectorData.secID.getFullSecString() << " had no matching inner sectors of " << innerSecIDs.size() << " stored in secMap - discarding as outer sector..."); nNoValidOfAllInnerSectors++; if (innerSecIDs.empty()) { nNoInnerExisting++; }; delete outerSector; }
    /** WARNING this could lead to the situation, that a sector can be created twice! (but time consuming hit-adding will only be done once)
     * - once as an innerEnd of a network, where it gets deleted here, since no valid inner Sectors exist
     * - then again when it is the innerSector of another outer one! shall this be optimized?
     * */
  } // outer sector loop
  B2DEBUG(1,
          "SegmentNetworkProducerModule::buildActiveSectorNetwork() (ev " << m_eventCounter <<
          "): nNoSPinThisInnerSector/nNoValidOfAllInnerSectors/nNoInnerExisting: " <<
          nNoSPinThisInnerSector << "/" << nNoValidOfAllInnerSectors << "/" << nNoInnerExisting <<
          ", size of nActiveSectors/activeSectorNetwork: " << activeSectors.size() << "/" << activeSectorNetwork.size());
  m_nGoodSectorsFound += activeSectors.size();
  m_nSectorsLinked += nSectorsLinked;
  m_nBadSectorInnerNotActive += nNoSPinThisInnerSector;
  m_nBadSectorNoInnerActive += nNoValidOfAllInnerSectors;
  m_nBadSectorNoInnerExisting += nNoInnerExisting;

  if (!m_PARAMprintNetworks) return;

  std::string fileName = m_vxdtfFilters->getConfig().secMapName + "_ActiveSector_Ev" + std::to_string(m_eventCounter);
  DNN::printNetwork<ActiveSector<StaticSectorType, TrackNode>, VoidMetaInfo>(activeSectorNetwork, fileName);
}


/** old name: segFinder. use SpacePoints stored in ActiveSectors to store and link them in a DirectedNodeNetwork< SpacePoint > */
void SegmentNetworkProducerModule::buildTrackNodeNetwork()
{
  DirectedNodeNetwork<ActiveSector<StaticSectorType, Belle2::TrackNode>, VoidMetaInfo>& activeSectorNetwork =
    m_network->accessActiveSectorNetwork();
  DirectedNodeNetwork<Belle2::TrackNode, VoidMetaInfo>& hitNetwork = m_network->accessHitNetwork();
  int nAccepted = 0, nRejected = 0, nLinked = 0;;

  // loop over outer sectors to get their hits(->outerHits) and inner sectors
  for (auto* outerSector : activeSectorNetwork.getNodes()) {
    if (outerSector->getInnerNodes().empty()) continue; // go to next sector
    const vector<TrackNode*>& outerHits = outerSector->getEntry().getHits();
    if (outerHits.empty()) continue;

    // loop over inner sectors to get their hits(->innerHits) and check their compatibility
    for (auto* innerSector : outerSector->getInnerNodes()) {
      const vector<TrackNode*>& innerHits = innerSector->getEntry().getHits();
      if (innerHits.empty()) continue;

      for (TrackNode* outerHit : outerHits) {
        // skip double-adding of nodes into the network after first time found -> speeding up the code:
        bool wasAnythingFoundSoFar = false;

        for (TrackNode* innerHit : innerHits) {
          // applying filters provided by the sectorMap:
          bool accepted = outerSector->getEntry().acceptTwoHitCombination(
                            innerSector->getEntry().getFullSecID(),
                            *outerHit,
                            *innerHit);

          if (m_PARAMallFiltersOff) accepted = true; // bypass all filters

          if (accepted == false) { nRejected++; continue; } // skip combinations which weren't accepted
          nAccepted++;

          // store combination of hits in network:
          if (!wasAnythingFoundSoFar) {
            hitNetwork.linkTheseEntries(*outerHit, *innerHit);
            nLinked++;
            wasAnythingFoundSoFar = true;
            continue;
          }
          hitNetwork.addInnerToLastOuterNode(*innerHit);
        } // inner hit loop
      } // outer hit loop
    } // inner sector loop
  } // outer sector loop
  B2DEBUG(1, "SegmentNetworkProducerModule::buildTrackNodeNetwork() (ev " << m_eventCounter << "): nAccepted/nRejected: " << nAccepted
          << "/" << nRejected <<
          ", size of nLinked/hitNetwork: " << nLinked << "/" << hitNetwork.size());
  m_nTrackNodesAccepted += nAccepted;
  m_nTrackNodesRejected += nRejected;
  m_nTrackNodeLinksCreated += nLinked;

  if (!m_PARAMprintNetworks) return;

  std::string fileName = m_vxdtfFilters->getConfig().secMapName + "_TrackNode_Ev" + std::to_string(m_eventCounter);
  DNN::printNetwork<Belle2::TrackNode, VoidMetaInfo>(hitNetwork, fileName);
}



/** old name: nbFinder. use connected SpacePoints to form segments which will stored and linked in a DirectedNodeNetwork< Segment > */
void SegmentNetworkProducerModule::buildSegmentNetwork()
{
  DirectedNodeNetwork<Belle2::TrackNode, VoidMetaInfo>& hitNetwork = m_network->accessHitNetwork();
  DirectedNodeNetwork<Segment< Belle2::TrackNode>, CACell>& segmentNetwork = m_network->accessSegmentNetwork();
  vector<Belle2::Segment<Belle2::TrackNode>* >& segments = m_network->accessSegments();
  int nAccepted = 0, nRejected = 0, nLinked = 0;;

  for (DirectedNode<TrackNode, VoidMetaInfo>* outerHit : hitNetwork.getNodes()) {
    const vector<DirectedNode<TrackNode, VoidMetaInfo>*>& centerHits = outerHit->getInnerNodes();
    if (centerHits.empty()) continue; // go to next outerHit

    for (DirectedNode<TrackNode, VoidMetaInfo>* centerHit : centerHits) {
      const vector<DirectedNode<TrackNode, VoidMetaInfo>*>& innerHits = centerHit->getInnerNodes();
      if (innerHits.empty()) continue; // go to next centerHit

      // skip double-adding of nodes into the network after first time found -> speeding up the code:
      bool wasAnythingFoundSoFar = false;
      for (DirectedNode<TrackNode, VoidMetaInfo>* innerHit : innerHits) {

        // applying filters provided by the sectorMap:
        bool accepted = outerHit->getEntry().sector->acceptThreeHitCombination(
                          centerHit->getEntry().sector->getFullSecID(),
                          innerHit->getEntry().sector->getFullSecID(),
                          outerHit->getEntry(),
                          centerHit->getEntry(),
                          innerHit->getEntry());

        B2DEBUG(5, "buildSegmentNetwork: outer/Center/Inner: " << outerHit->getEntry().getName() << "/" << centerHit->getEntry().getName()
                << "/" << innerHit->getEntry().getName() << ", accepted: " << std::to_string(accepted));

        if (m_PARAMallFiltersOff) accepted = true; // bypass all filters

        if (accepted == false) { nRejected++; continue; } // skip combinations which weren't accepted
        nAccepted++;

        // create innerSegment first (order of storage in vector<segments> is irrelevant):
        Segment<TrackNode>* innerSegment = new Segment<TrackNode>(
          centerHit->getEntry().sector->getFullSecID(),
          innerHit->getEntry().sector->getFullSecID(),
          &centerHit->getEntry(),
          &innerHit->getEntry());
        B2DEBUG(5, "buildSegmentNetwork: innerSegment: " << innerSegment->getName());
        DirectedNode<Segment<TrackNode>, CACell>* tempInnerSegmentnode = segmentNetwork.getNode(*innerSegment);
        if (tempInnerSegmentnode == nullptr) {
          segments.push_back(innerSegment);
        } else {
          delete innerSegment;
          innerSegment = &(tempInnerSegmentnode->getEntry());
        }

        // store combination of hits in network:
        if (!wasAnythingFoundSoFar) {
          // create outerSector:
          Segment<TrackNode>* outerSegment = new Segment<TrackNode>(
            outerHit->getEntry().sector->getFullSecID(),
            centerHit->getEntry().sector->getFullSecID(),
            &outerHit->getEntry(),
            &centerHit->getEntry());
          B2DEBUG(5, "buildSegmentNetwork: outerSegment(freshly created): " << outerSegment->getName() << " to be linked with inner segment: "
                  << innerSegment->getName());
//      segmentNetwork.linkTheseEntries(*outerSegment, *innerSegment);
          DirectedNode<Segment<TrackNode>, CACell>* tempOuterSegmentnode = segmentNetwork.getNode(*outerSegment);
          if (tempOuterSegmentnode == nullptr) {
            segments.push_back(outerSegment);
          } else {
            delete outerSegment;
            outerSegment = &(tempOuterSegmentnode->getEntry());
          }

          B2DEBUG(5, "buildSegmentNetwork: outerSegment (after duplicate check): " << outerSegment->getName() <<
                  " to be linked with inner segment: " << innerSegment->getName());
          segmentNetwork.linkTheseEntries(*outerSegment, *innerSegment);
          nLinked++;
          wasAnythingFoundSoFar = true;
          continue;
        }
        segmentNetwork.addInnerToLastOuterNode(*innerSegment);
      } // innerHit-loop
    } // centerHit-loop
  } // outerHit-loop

  B2DEBUG(1, "SegmentNetworkProducerModule::buildSegmentNetwork() (ev " << m_eventCounter << "): nAccepted/nRejected: " << nAccepted
          << "/" << nRejected <<
          ", size of nLinked/hitNetwork: " << nLinked << "/" << segmentNetwork.size());
  m_nSegmentsAccepted += nAccepted;
  m_nSegmentsRejected += nRejected;
  m_nSegmentsLinksCreated += nLinked;

  if (!m_PARAMprintNetworks) return;

  std::string fileName = m_vxdtfFilters->getConfig().secMapName + "_Segment_Ev" + std::to_string(m_eventCounter);
  DNN::printNetwork<Segment< Belle2::TrackNode>, CACell>(segmentNetwork, fileName);
  DNN::printCANetwork<Segment< Belle2::TrackNode>>(segmentNetwork, "CA" + fileName);

}
