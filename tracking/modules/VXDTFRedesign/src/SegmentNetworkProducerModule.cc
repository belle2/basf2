/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/VXDTFRedesign/SegmentNetworkProducerModule.h>


using namespace std;
using namespace Belle2;


REG_MODULE(SegmentNetworkProducer)

SegmentNetworkProducerModule::SegmentNetworkProducerModule() : Module()
{
  InitializeCounters();

  vector<string> spacePointArrayNames = {""};

  vector<double> ipCoords = {0, 0, 0};

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
           m_PARAMVirtualIPCoorindates,
           "only valid if nEntries == 3, excpects X, Y, and Z coordinates for virtual IP in global coordinates. Only used if addVirtualIP == true",
           ipCoords);
}



void SegmentNetworkProducerModule::event()
{
  m_eventCounter++;
  B2INFO("\n" << "SegmentNetworkProducerModule:event: event " << m_eventCounter << "\n")

  // make sure that network exists:
  if (! m_network) { m_network.create(); }

  vector< RawSectorData > collectedData = matchSpacePointToSectors();

  buildActiveSectorNetwork(collectedData);

  buildTrackNodeNetwork();

  buildSegmentNetwork();

  // TODO debug output with counters!
}



void SegmentNetworkProducerModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;

  B2INFO("SegmentNetworkProducerModule:endRun: events: " << m_eventCounter << " and invEvents: " << invEvents)

  // TODO debug output with counters!
}





/** for each SpacePoint given, find according sector and store them in a fast and intermediate way* */
std::vector< SegmentNetworkProducerModule::RawSectorData > SegmentNetworkProducerModule::matchSpacePointToSectors()
{
  // TODO add counters!
  std::vector< RawSectorData > collectedData;
  std::vector<TrackNode* >& trackNodes = m_network->accessTrackNodes(); // collects trackNodes

  for (StoreArray<SpacePoint>& storeArray : m_spacePoints) {

    // match all SpacePoints with the sectors:
    for (SpacePoint& aSP : storeArray) {

      StaticSectorDummy* sectorFound = findSectorForSpacePoint(aSP);

      if (sectorFound == NULL) {
        B2WARNING("no sector found, SpacePoint discarded!");
        continue;
      }

      TrackNode* trackNode = new TrackNode();
      trackNode->spacePoint = &aSP;
      trackNodes.push_back(trackNode);

      // sector for SpacePoint exists:
      FullSecID foundSecID = sectorFound->getFullSecID();

      vector<RawSectorData>::iterator iter =
        std::find_if(
          collectedData.begin(),
          collectedData.end(),
          [&](const RawSectorData & entry) -> bool
      { return entry.secID == foundSecID; }
        );

      // if secID not in collectedData:
      if (iter == collectedData.end()) {
        collectedData.push_back({ foundSecID , false, NULL, sectorFound, {trackNode}});
      } else {
        iter->hits.push_back(trackNode);
      }

    } // loop over SpacePoints in StoreArray
  } // loop over StoreArrays

  // store IP-coordinates
  if (m_PARAMAddVirtualIP == true) {
    m_network->setVirtualInteractionPoint(); // TODO pass relevant parameters!
    TrackNode* vIP = m_network->getVirtualInteractionPoint();
    StaticSectorDummy* sectorFound = findSectorForSpacePoint(*vIP->spacePoint);
    collectedData.push_back({FullSecID(), false, NULL, sectorFound, {vIP}}); // TODO: which FullSecID for the vIP?
  }

  return std::move(collectedData);
}



/** build a DirectedNodeNetwork< ActiveSector >, where all ActiveSectors are stored which have SpacePoints* and compatible inner- or outer neighbours */
void SegmentNetworkProducerModule::buildActiveSectorNetwork(std::vector< SegmentNetworkProducerModule::RawSectorData >&
                                                            collectedData)
{
  // TODO add counters!

  DirectedNodeNetwork<ActiveSector<StaticSectorDummy, TrackNode>>& activeSectorNetwork = m_network->accessActiveSectorNetwork();
  vector<ActiveSector<StaticSectorDummy, TrackNode>*>& activeSectors = m_network->accessActiveSectors();

  for (RawSectorData& outerSectorData : collectedData) {
    ActiveSector<StaticSectorDummy, TrackNode>* outerSector = new ActiveSector<StaticSectorDummy, TrackNode>
    (outerSectorData.staticSector);

    // skip double-adding of nodes into the network after first iteration -> speeding up the code:
    bool isFirstIteration = true;
    // find innerSectors of outerSector and add them to the network:
    const std::vector<FullSecID>& innerSecIDs = outerSector->getInnerSecIDs();

    for (const FullSecID innerSecID : innerSecIDs) {
      vector<RawSectorData>::iterator pos =
        std::find_if(
          collectedData.begin(),
          collectedData.end(),
          [&](const RawSectorData & entry) -> bool
      { return (entry.wasCreated == false) and (entry.secID == innerSecID); }
        );

      // current inner sector has no SpacePoints in this event:
      if (pos == collectedData.end()) { continue; }

      // take care of inner sector first:
      ActiveSector<StaticSectorDummy, TrackNode>* innerSector = NULL;
      if (pos->wasCreated) { // was already there
        innerSector = pos->sector;
      } else {
        innerSector = new ActiveSector<StaticSectorDummy, TrackNode>(pos->staticSector);
        pos->wasCreated = true;
        pos->sector = innerSector;
        for (auto* hit : pos->hits) { hit->sector = innerSector; }
        // add all SpacePoints of this sector to ActiveSector:
        innerSector->addHits(pos->hits);
        activeSectors.push_back(innerSector);
      }

      // when accepting combination the first time, take care of outer sector:
      if (isFirstIteration) {
        outerSectorData.wasCreated = true;
        outerSectorData.sector = outerSector;
        for (auto* hit : outerSectorData.hits) { hit->sector = outerSector; }
        // add all SpacePoints of this sector to ActiveSector:
        outerSector->addHits(outerSectorData.hits);
        activeSectors.push_back(outerSector);

        activeSectorNetwork.linkTheseEntries(*outerSector, *innerSector);
        isFirstIteration = false;
        continue;
      }
      activeSectorNetwork.addInnerToLastOuterNode(*innerSector);
    } // inner sector loop

    // discard outerSector if no valid innerSector could be found
    if (isFirstIteration == true) { delete outerSector; }
    /** WARNING this could lead to the situation, that a sector can be created twice! (but time consuming hit-adding will only be done once)
     * - once as an innerEnd of a network, where it gets deleted here, since no valid inner Sectors exist
     * - then again when it is the innerSector of another outer one! shall this be optimized?
     * */
  } // outer sector loop
}


/** old name: segFinder. use SpacePoints stored in ActiveSectors to store and link them in a DirectedNodeNetwork< SpacePoint > */
void SegmentNetworkProducerModule::buildTrackNodeNetwork()
{
  // TODO add counters!
  DirectedNodeNetwork<ActiveSector<StaticSectorDummy, Belle2::TrackNode>>& activeSectorNetwork =
        m_network->accessActiveSectorNetwork();
  DirectedNodeNetwork<Belle2::TrackNode>& hitNetwork = m_network->accessHitNetwork();

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
        // skip double-adding of nodes into the network after first iteration -> speeding up the code:
        bool isFirstIteration = true;

        for (TrackNode* innerHit : innerHits) {
          // applying filters provided by the sectorMap:
          bool accepted = outerSector->getEntry().acceptTwoHitCombination(
                            innerSector->getEntry().getFullSecID(),
                            *outerHit,
                            *innerHit);

          if (accepted == false) continue; // skip combinations which weren't accepted

          // store combination of hits in network:
          if (isFirstIteration) {
            hitNetwork.linkTheseEntries(*outerHit, *innerHit);
            isFirstIteration = false;
            continue;
          }
          hitNetwork.addInnerToLastOuterNode(*innerHit);
        } // inner hit loop
      } // outer hit loop
    } // inner sector loop
  } // outer sector loop
}



/** old name: nbFinder. use connected SpacePoints to form segments which will stored and linked in a DirectedNodeNetwork< Segment > */
void SegmentNetworkProducerModule::buildSegmentNetwork()
{
  // TODO add counters!
  DirectedNodeNetwork<Belle2::TrackNode>& hitNetwork = m_network->accessHitNetwork();
  DirectedNodeNetwork<Segment< Belle2::TrackNode>>& segmentNetwork = m_network->accessSegmentNetwork();
  vector<Belle2::Segment<Belle2::TrackNode>* >& segments = m_network->accessSegments();

  for (DirectedNode<TrackNode>* outerHit : hitNetwork.getNodes()) {
    const vector<DirectedNode<TrackNode>*>& centerHits = outerHit->getInnerNodes();
    if (centerHits.empty()) continue; // go to next outerHit

    for (DirectedNode<TrackNode>* centerHit : centerHits) {
      const vector<DirectedNode<TrackNode>*>& innerHits = centerHit->getInnerNodes();
      if (innerHits.empty()) continue; // go to next centerHit

      // skip double-adding of nodes into the network after first iteration -> speeding up the code:
      bool isFirstIteration = true;
      for (DirectedNode<TrackNode>* innerHit : innerHits) {

        // applying filters provided by the sectorMap:
        bool accepted = outerHit->getEntry().sector->acceptThreeHitCombination(
                          centerHit->getEntry().sector->getFullSecID(),
                          innerHit->getEntry().sector->getFullSecID(),
                          outerHit->getEntry(),
                          centerHit->getEntry(),
                          innerHit->getEntry());

        if (accepted == false) continue; // skip combinations which weren't accepted

        // create innerSegment first (order of storage in vector<segments> is irrelevant):
        Segment<TrackNode>* innerSegment = new Segment<TrackNode>(
          outerHit->getEntry().sector->getFullSecID(),
          centerHit->getEntry().sector->getFullSecID(),
          &outerHit->getEntry(),
          &centerHit->getEntry());
        DirectedNode<Segment<TrackNode>>* tempSegmentnode = segmentNetwork.getNode(*innerSegment);
        if (tempSegmentnode == NULL) {
          segments.push_back(innerSegment);
        } else {
          delete innerSegment;
          innerSegment = &tempSegmentnode->getEntry();
        }

        // store combination of hits in network:
        if (isFirstIteration) {
          // create outerSector:
          Segment<TrackNode>* outerSegment = new Segment<TrackNode>(
            outerHit->getEntry().sector->getFullSecID(),
            centerHit->getEntry().sector->getFullSecID(),
            &outerHit->getEntry(),
            &centerHit->getEntry());
          DirectedNode<Segment<TrackNode>>* tempSegmentnode = segmentNetwork.getNode(*innerSegment);
          if (tempSegmentnode == NULL) {
            segments.push_back(outerSegment);
          } else {
            delete outerSegment;
            outerSegment = &tempSegmentnode->getEntry();
          }

          segmentNetwork.linkTheseEntries(*outerSegment, *innerSegment);
          isFirstIteration = false;
          continue;
        }
        segmentNetwork.addInnerToLastOuterNode(*innerSegment);
      } // innerHit-loop
    } // centerHit-loop
  } // outerHit-loop
}
