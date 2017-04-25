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
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>

//Observer types
#include <tracking/trackFindingVXD/filterMap/filterFramework/VoidObserver.h>
#include <tracking/trackFindingVXD/filterTools/ObserverCheckMCPurity.h>
#include <tracking/trackFindingVXD/filterTools/ObserverCheckFilters.h>

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

  addParam("observerType",
           m_PARAMobserverType,
           "Use this option for debugging ONLY!"
           "0 -> No observer (VoidObserver) This is the default!; "
           "1 -> ObserverCheckMCPurity : observes filter, values are written to a root file;"
           "2 -> ObserverCheckFilters : observes filter, values are stored to the datastore (WARNING creates lots of data)"
           "NOTE: that observing filters (using another option than 0 VoidObserver) makes the code slow!"
           "So only use for debugging purposes.",
           int(SegmentNetworkProducerModule::c_VoidObserver));
}



/** Initializes the Module.
 */
void
SegmentNetworkProducerModule::initialize()
{
  InitializeCounters();

  // searching for correct sectorMap:
  for (auto& setup : m_filtersContainer.getAllSetups()) {
    auto& filters = *(setup.second);

    if (filters.getConfig().secMapName != m_PARAMsecMapName) { continue; }
    B2INFO("SegmentNetworkProducerModule::initialize(): loading mapName: " << m_PARAMsecMapName << " with nCompactSecIDs: " <<
           filters.size());

    m_vxdtfFilters = &filters;
    SecMapHelper::printStaticSectorRelations(filters, filters.getConfig().secMapName + "segNetProducer", 2, true, true);
    if (m_vxdtfFilters == nullptr) B2FATAL("SegmentNetworkProducerModule::initialize(): requested secMapName '" << m_PARAMsecMapName <<
                                             "' does not exist! Can not continue...");
    break; // have found our secMap no need for further searching
  }

  if (m_PARAMCreateNeworks < 1 or m_PARAMCreateNeworks > 3) {
    B2FATAL("SegmentNetworkProducerModule::Initialize(): parameter 'createNeworks' is set to " << m_PARAMCreateNeworks <<
            "which is invalid, please read the documentation (basf2 - m SegmentNetworkProducer)!");
  }

  for (std::string& anArrayName : m_PARAMSpacePointsArrayNames) {
    m_spacePoints.push_back(StoreArray<SpacePoint>(anArrayName));
    m_spacePoints.back().isRequired();
  }

  m_network.registerInDataStore(m_PARAMNetworkOutputName, DataStore::c_DontWriteOut);

  // TODO catch cases when m_network already existed in DataStore!


  // for debugging purposes the filter responses can be observed and stored to a root file or to the datastore
  if (m_PARAMobserverType == SegmentNetworkProducerModule::c_ObserverCheckMCPurity) {
    /** This TFile is used by the observers, at present it is created by default.
      TODO : this might not be a good construction for parallel processing! Replace by something which is good for parallel
      preocessing!
    */

    if (m_tfile) delete m_tfile;
    m_tfile = new TFile("observeFilterSegNetProducer.root", "RECREATE");
    m_tfile->cd();
    TTree* newTree = new TTree("twoHitsTree", "Observers");

    // create a dummy verison of the 2-hit-filter
    VXDTFFilters<SpacePoint>::twoHitFilter_t aFilter;
    // initialize the !observed! verion of the Filter
    bool isinitialized = initializeObservers(aFilter.observe(ObserverCheckMCPurity()) , newTree);
    if (!isinitialized) B2WARNING("Observers not initialized properly! The results of the observation may be faulty!");
  } else {
    m_tfile = NULL;
  }

  // for this observer the results will be dumped into the datastore
  if (m_PARAMobserverType == SegmentNetworkProducerModule::c_ObserverCheckFilters) {
    // needs a StoreArray to store the data
    StoreArray<ObserverInfo> observerInfoArray("observerInfos", DataStore::c_Event);
    observerInfoArray.registerInDataStore();

    VXDTFFilters<SpacePoint>::twoHitFilter_t aFilter;
    bool isinitialized = initializeObservers(aFilter.observe(ObserverCheckFilters()) , observerInfoArray);
    if (!isinitialized) B2WARNING("Observers not initialized properly! The results of the observation may be faulty!");
  }


} // end initialize



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


  // use VoidObserver to deactivate observation of filters
  if (m_PARAMobserverType == c_ObserverCheckMCPurity) buildTrackNodeNetwork<ObserverCheckMCPurity>();
  else if (m_PARAMobserverType == c_ObserverCheckFilters) buildTrackNodeNetwork<ObserverCheckFilters>();
  else buildTrackNodeNetwork<VoidObserver>(); // apply-two-hit-filters

  if (m_PARAMCreateNeworks < 3) { B2DEBUG(10, "SegmentNetworkProducerModule:event: event " << m_eventCounter << ": finished work after creating trackNodeNetwork"); return; }

  // use VoidObserver to deactivate observation, currently we dont observe the three hits so all VoidObserver
  if (m_PARAMobserverType == c_ObserverCheckMCPurity) buildSegmentNetwork<VoidObserver>();
  else if (m_PARAMobserverType == c_ObserverCheckFilters) buildSegmentNetwork<VoidObserver>();
  else buildSegmentNetwork<VoidObserver>(); // apply-three-hit-filters

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

      TrackNode* trackNode = new TrackNode(&aSP);
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
  int nChecked = 0;

  // loop over all raw sectors found so far:
  for (RawSectorData& outerSectorData : collectedData) {
    ActiveSector<StaticSectorType, TrackNode>* outerSector = new ActiveSector<StaticSectorType, TrackNode>
    (outerSectorData.staticSector);
    std::string outerEntryID = outerSector->getFullSecID().getFullSecString();

    // skip double-adding of nodes into the network after first time found -> speeding up the code:
    bool wasAnythingFoundSoFar = false;
    // find innerSectors of outerSector and add them to the network:
    const std::vector<FullSecID>& innerSecIDs = outerSector->getInner2spSecIDs();

    for (const FullSecID innerSecID : innerSecIDs) {
      nChecked++;
      std::string innerEntryID = innerSecID.getFullSecString();
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
        activeSectorNetwork.addNode(innerEntryID, *innerSector);

      }

      // when accepting combination the first time, take care of outer sector:
      if (!wasAnythingFoundSoFar) {
        outerSectorData.wasCreated = true;
        outerSectorData.sector = outerSector;
        for (Belle2::TrackNode* hit : outerSectorData.hits) { hit->sector = outerSector; }
        // add all SpacePoints of this sector to ActiveSector:
        outerSector->addHits(outerSectorData.hits);
        activeSectors.push_back(outerSector);
        activeSectorNetwork.addNode(outerEntryID, *outerSector);

        B2DEBUG(5, "SegmentNetworkProducerModule::buildActiveSectorNetwork(): sector " << outerSector->getName() <<
                " will be combined with inner sector " << innerSector->getName());

        activeSectorNetwork.linkNodes(outerEntryID, innerEntryID);
        wasAnythingFoundSoFar = true;
        nSectorsLinked++;
        continue;
      }
      activeSectorNetwork.addInnerToLastOuterNode(innerEntryID);
    } // inner sector loop
    // discard outerSector if no valid innerSector could be found
    if (wasAnythingFoundSoFar == false) { B2DEBUG(5, "SegmentNetworkProducerModule::buildActiveSectorNetwork(): sector " << outerSectorData.secID.getFullSecString() << " had no matching inner sectors of " << innerSecIDs.size() << " stored in secMap - discarding as outer sector..."); nNoValidOfAllInnerSectors++; if (innerSecIDs.empty()) { nNoInnerExisting++; }; delete outerSector; }
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
  //DNN::printNetwork<ActiveSector<StaticSectorType, TrackNode>, VoidMetaInfo>(activeSectorNetwork, fileName);
}


/** old name: segFinder. use SpacePoints stored in ActiveSectors to store and link them in a DirectedNodeNetwork< SpacePoint > */
template < class ObserverType >
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

    // get the point to the static sector
    const StaticSectorType* outerStaticSector = outerSector->getEntry().getAttachedStaticSector();
    // should not happen, but just in case:
    if (outerStaticSector == NULL) {
      B2WARNING("Static sector not found. This should not happen!");
      continue;
    }

    // loop over inner sectors to get their hits(->innerHits) and check their compatibility
    for (auto* innerSector : outerSector->getInnerNodes()) {
      const vector<TrackNode*>& innerHits = innerSector->getEntry().getHits();
      if (innerHits.empty()) continue;


      //retrieve the filter, a null pointer is returned if there is no filter
      const auto* filter2sp = outerStaticSector->getFilter2sp(innerSector->getEntry().getFullSecID());
      if (filter2sp == NULL) continue;

      for (TrackNode* outerHit : outerHits) {
        // skip double-adding of nodes into the network after first time found -> speeding up the code:
        bool wasAnythingFoundSoFar = false;


        std::string outerNodeID = outerHit->getName();
        hitNetwork.addNode(outerNodeID, *outerHit);

        for (TrackNode* innerHit : innerHits) {
          // applying filters provided by the sectorMap:
          // ->observe() gives back an observed version of the filter (the default filter has the VoidObserver)
          bool accepted = (filter2sp->observe(ObserverType())).accept(outerHit->getHit(), innerHit->getHit());

          if (m_PARAMallFiltersOff) accepted = true; // bypass all filters

          if (accepted == false) { nRejected++; continue; } // skip combinations which weren't accepted
          nAccepted++;


          std::string innerNodeID = innerHit->getName();
          hitNetwork.addNode(innerNodeID, *innerHit);
          // store combination of hits in network:
          if (!wasAnythingFoundSoFar) {
            hitNetwork.linkNodes(outerNodeID, innerNodeID);
            nLinked++;
            wasAnythingFoundSoFar = true;
            continue;
          }
          hitNetwork.addInnerToLastOuterNode(innerNodeID);
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
  //DNN::printNetwork<Belle2::TrackNode, VoidMetaInfo>(hitNetwork, fileName);

}



/** old name: nbFinder. use connected SpacePoints to form segments which will stored and linked in a DirectedNodeNetwork< Segment > */
template < class ObserverType >
void SegmentNetworkProducerModule::buildSegmentNetwork()
{
  DirectedNodeNetwork<Belle2::TrackNode, VoidMetaInfo>& hitNetwork = m_network->accessHitNetwork();
  DirectedNodeNetwork<Segment< Belle2::TrackNode>, CACell>& segmentNetwork = m_network->accessSegmentNetwork();
  vector<Belle2::Segment<Belle2::TrackNode>* >& segments = m_network->accessSegments();
  int nAccepted = 0, nRejected = 0, nLinked = 0;;
  B2WARNING("buildSegmentNetwork");
  for (auto* outerHit : hitNetwork.getNodes()) {
    const vector<DirectedNode<TrackNode, VoidMetaInfo>*>& centerHits = outerHit->getInnerNodes();
    if (centerHits.empty()) continue; // go to next outerHit

    // get the point to the static sector
    const StaticSectorType* outerStaticSector = outerHit->getEntry().sector->getAttachedStaticSector();
    // should not happen, but just in case:
    if (outerStaticSector == NULL) {
      B2WARNING("Static sector not found. This should not happen!");
      continue;
    }

    for (DirectedNode<TrackNode, VoidMetaInfo>* centerHit : centerHits) {
      const vector<DirectedNode<TrackNode, VoidMetaInfo>*>& innerHits = centerHit->getInnerNodes();
      if (innerHits.empty()) continue; // go to next centerHit

      // skip double-adding of nodes into the network after first time found -> speeding up the code:
      bool wasAnythingFoundSoFar = false;
      for (DirectedNode<TrackNode, VoidMetaInfo>* innerHit : innerHits) {

        //retrieve the filter
        const auto* filter3sp = outerStaticSector->getFilter3sp(centerHit->getEntry().sector->getFullSecID(),
                                                                innerHit->getEntry().sector->getFullSecID());
        if (filter3sp == NULL) continue;

        // the filter accepts spacepoint combinations
        // ->observe gives back an observed version of the filter
        bool accepted = (filter3sp->observe(ObserverType())).accept(outerHit->getEntry().getHit(), centerHit->getEntry().getHit(),
                                                                    innerHit->getEntry().getHit());

        B2DEBUG(5, "buildSegmentNetwork: outer/Center/Inner: " << outerHit->getEntry().getName() << "/" << centerHit->getEntry().getName()
                << "/" << innerHit->getEntry().getName() << ", accepted: " << std::to_string(accepted));

        if (m_PARAMallFiltersOff) accepted = true; // bypass all filters

        if (accepted == false) { nRejected++; continue; } // skip combinations which weren't accepted
        nAccepted++;

        std::string innerSegmentID = centerHit->getEntry().getName() + innerHit->getEntry().getName();
        if (not segmentNetwork.isNodeInNetwork(innerSegmentID)) {
          // create innerSegment first (order of storage in vector<segments> is irrelevant):
          Segment<TrackNode>* innerSegment = new Segment<TrackNode>(
            centerHit->getEntry().sector->getFullSecID(),
            innerHit->getEntry().sector->getFullSecID(),
            &centerHit->getEntry(),
            &innerHit->getEntry()
          );
          segments.push_back(innerSegment);
          segmentNetwork.addNode(innerSegmentID, *innerSegment);
        }

        std::string outerSegmentID = outerHit->getEntry().getName() + centerHit->getEntry().getName();
        if (not segmentNetwork.isNodeInNetwork(outerSegmentID)) {
          // create innerSegment first (order of storage in vector<segments> is irrelevant):
          Segment<TrackNode>* outerSegment = new Segment<TrackNode>(
            outerHit->getEntry().sector->getFullSecID(),
            centerHit->getEntry().sector->getFullSecID(),
            &outerHit->getEntry(),
            &centerHit->getEntry()
          );
          segments.push_back(outerSegment);
          segmentNetwork.addNode(outerSegmentID, *outerSegment);
        }
        // store combination of hits in network:
        if (!wasAnythingFoundSoFar) {
          segmentNetwork.linkNodes(outerSegmentID, innerSegmentID);
          nLinked++;
          wasAnythingFoundSoFar = true;
          continue;
        }
        segmentNetwork.addInnerToLastOuterNode(innerSegmentID);
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
  //DNN::printNetwork<Segment< Belle2::TrackNode>, CACell>(segmentNetwork, fileName);
  //DNN::printCANetwork<Segment< Belle2::TrackNode>>(segmentNetwork, "CA" + fileName);
}
