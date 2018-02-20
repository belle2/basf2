/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Felix Metzner                       *
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
  //Set module properties
  setDescription("The segment network producer module. "
                 "\n This module takes a given sectorMap and storeArrays of spacePoints and creates a "
                 "activeSectorNetwork, a trackNodeNetwork and a segmentNetwork."
                 "These are filled and stored in a StoreObjPtr of DirectedNodeNetworkContainer:\n");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SpacePointsArrayNames",
           m_PARAMSpacePointsArrayNames,
           "List of SpacePoint StoreArray names to be evaluated.",
           m_PARAMSpacePointsArrayNames);

  addParam("NetworkOutputName",
           m_PARAMNetworkOutputName,
           "Unique name for the DirectedNodeNetworkContainer Store Object Pointer created and filled by this module.",
           string(""));

  addParam("addVirtualIP",
           m_PARAMAddVirtualIP,
           "Whether to add a SpacePoint for a virtual interaction point to be considered by the network creation.",
           m_PARAMAddVirtualIP);

  addParam("virtualIPCoorindates",
           m_PARAMVirtualIPCoordinates,
           "Coordinates as list [x,z,y] to be used for the virtual interaction point SpacePoint, if turned on.",
           m_PARAMVirtualIPCoordinates);

  addParam("virtualIPErrors",
           m_PARAMVirtualIPErrors,
           "Errors on coordinates as list [Ex,Ez,Ey] to be used for the virtual interaction point SpacePoint, if turned on.",
           m_PARAMVirtualIPErrors);

  addParam("sectorMapName",
           m_PARAMsecMapName,
           "Name of the SectorMap to be used by this instance.",
           m_PARAMsecMapName);

  addParam("printNetworks",
           m_PARAMprintNetworks,
           "If true for each event and each network a file containing the networks as graphs is created.",
           m_PARAMprintNetworks);

  addParam("printNetworkToMathematica",
           m_PARAMprintToMathematica,
           "If true a file containing Mathematica code to generate a graph of the segment network is created.",
           m_PARAMprintToMathematica);

  addParam("allFiltersOff",
           m_PARAMallFiltersOff,
           "For debugging purposes: if true, all filters are deactivated for all hit-combinations and therefore all combinations are accepted.",
           m_PARAMallFiltersOff);

  addParam("maxNetworkSize",
           m_PARAMmaxNetworkSize,
           "Maximal size of the SegmentNetwork; if exceeded, the event execution will be skipped.",
           m_PARAMmaxNetworkSize);

  addParam("maxHitNetworkSize",
           m_PARAMmaxHitNetworkSize,
           "Maximal size of the HitNetwork; if exceeded, the event execution will be skipped.",
           m_PARAMmaxHitNetworkSize);
}


void SegmentNetworkProducerModule::initialize()
{
  if (m_PARAMVirtualIPCoordinates.size() != 3 or m_PARAMVirtualIPErrors.size() != 3) {
    B2FATAL("Parameters for virtualIP are wrong!");
  }

  // Get pointer to current filters to check if they exist. They must be reloaded for every run,
  // as the pointer will change if the DB object changes (see SectorMapBootStrapModule).
  auto filters = m_filtersContainer.getFilters(m_PARAMsecMapName);
  if (filters == nullptr) B2FATAL("Requested secMapName '" << m_PARAMsecMapName
                                    << "' does not exist! Can not continue...");

  m_virtualIPCoordinates = B2Vector3D(m_PARAMVirtualIPCoordinates.at(0),
                                      m_PARAMVirtualIPCoordinates.at(1),
                                      m_PARAMVirtualIPCoordinates.at(2));
  m_virtualIPErrors = B2Vector3D(m_PARAMVirtualIPErrors.at(0),
                                 m_PARAMVirtualIPErrors.at(1),
                                 m_PARAMVirtualIPErrors.at(2));

  InitializeCounters();

  if (m_PARAMprintToMathematica) {
    SecMapHelper::printStaticSectorRelations(*filters, filters->getConfig().secMapName + "segNetProducer", 2, m_PARAMprintToMathematica,
                                             true);
  }

  for (std::string& anArrayName : m_PARAMSpacePointsArrayNames) {
    m_spacePoints.push_back(StoreArray<SpacePoint>(anArrayName));
    m_spacePoints.back().isRequired();
  }

  m_network.registerInDataStore(m_PARAMNetworkOutputName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
}


void SegmentNetworkProducerModule::event()
{
  m_eventCounter++;

  if (m_vxdtfFilters == nullptr) B2FATAL("Requested secMapName '" << m_PARAMsecMapName << "' does not exist! Can not continue...");

  // make sure that network exists:
  if (!m_network) {
    m_network.create();
  }

  vector<RawSectorData> collectedData = matchSpacePointToSectors();

  buildActiveSectorNetwork(collectedData);

  if (not buildTrackNodeNetwork<VoidObserver>()) {
    return;
  }

  buildSegmentNetwork<VoidObserver>();
}


std::vector<SegmentNetworkProducerModule::RawSectorData> SegmentNetworkProducerModule::matchSpacePointToSectors()
{
  std::vector<RawSectorData> collectedData; // contains the raw sectors to be activated
  std::vector<TrackNode* >& trackNodes = m_network->accessTrackNodes(); // collects trackNodes
  int nSPsFound = 0, nSPsLost = 0;

  for (StoreArray<SpacePoint>& storeArray : m_spacePoints) {
    // match all SpacePoints with the sectors:
    for (SpacePoint& aSP : storeArray) {
      if (aSP.getAssignmentState()) {
        continue;
      }

      const StaticSectorType* sectorFound = findSectorForSpacePoint(aSP);

      if (sectorFound == nullptr) {
        B2WARNING("SpacePoint in sensor " << aSP.getVxdID() << " no sector found, SpacePoint discarded!");
        nSPsLost++;
        continue;
      }
      nSPsFound++;

      TrackNode* trackNode = new TrackNode(&aSP);
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
        collectedData.push_back({ foundSecID , false, nullptr, sectorFound, {trackNode}});
      } else {
        iter->hits.push_back(trackNode);
      }
    }
  }
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


void SegmentNetworkProducerModule::buildActiveSectorNetwork(std::vector<SegmentNetworkProducerModule::RawSectorData>&
                                                            collectedData)
{
  // access (yet) empty activeSectorNetwork:
  DirectedNodeNetwork<ActiveSector<StaticSectorType, TrackNode>, VoidMetaInfo>& activeSectorNetwork =
    m_network->accessActiveSectorNetwork();
  // activeSectors are to be stored separately:
  vector<ActiveSector<StaticSectorType, TrackNode>* >& activeSectors = m_network->accessActiveSectors();
  int nLinked = 0;

  // loop over all raw sectors found so far:
  for (RawSectorData& outerSectorData : collectedData) {
    ActiveSector<StaticSectorType, TrackNode>* outerSector = new ActiveSector<StaticSectorType, TrackNode>
    (outerSectorData.staticSector);
    std::int32_t outerEntryID = outerSector->getID();

    // skip double-adding of nodes into the network after first time found -> speeding up the code:
    bool wasAnythingFoundSoFar = false;
    // find innerSectors of outerSector and add them to the network:
    const std::vector<FullSecID>& innerSecIDs = outerSector->getInner2spSecIDs();

    for (const FullSecID innerSecID : innerSecIDs) {
      std::int32_t innerEntryID = innerSecID;
      vector<RawSectorData>::iterator innerRawSecPos =
        std::find_if(
          collectedData.begin(),
          collectedData.end(),
          [&](const RawSectorData & entry) -> bool
      { return (entry.secID == innerSecID); }
        );

      // current inner sector has no SpacePoints in this event:
      if (innerRawSecPos == collectedData.end()) {
        continue;
      }

      // take care of inner sector first:
      ActiveSector<StaticSectorType, TrackNode>* innerSector = nullptr;
      if (innerRawSecPos->wasCreated) { // was already there
        innerSector = innerRawSecPos->sector;
      } else {
        innerSector = new ActiveSector<StaticSectorType, TrackNode>(innerRawSecPos->staticSector);
        innerRawSecPos->wasCreated = true;
        innerRawSecPos->sector = innerSector;
        for (Belle2::TrackNode* hit : innerRawSecPos->hits) { hit->m_sector = innerSector; }
        // add all SpacePoints of this sector to ActiveSector:
        innerSector->addHits(innerRawSecPos->hits);
        activeSectors.push_back(innerSector);
        activeSectorNetwork.addNode(innerEntryID, *innerSector);
      }

      // when accepting combination the first time, take care of outer sector:
      if (!wasAnythingFoundSoFar) {
        outerSectorData.wasCreated = true;
        outerSectorData.sector = outerSector;
        for (Belle2::TrackNode* hit : outerSectorData.hits) { hit->m_sector = outerSector; }
        // add all SpacePoints of this sector to ActiveSector:
        outerSector->addHits(outerSectorData.hits);
        activeSectors.push_back(outerSector);
        activeSectorNetwork.addNode(outerEntryID, *outerSector);

        if (activeSectorNetwork.linkNodes(outerEntryID, innerEntryID)) {
          wasAnythingFoundSoFar = true;
          nLinked++;
        }
      } else {
        activeSectorNetwork.addInnerToLastOuterNode(innerEntryID);
      }
    }
    // discard outerSector if no valid innerSector could be found
    if (wasAnythingFoundSoFar == false) {
      delete outerSector;
    }
  }

  B2WARNING(">>>> :event:" << m_eventCounter << ": >> :activSectorConections:" << nLinked);

  if (m_PARAMprintNetworks) {
    std::string fileName = m_vxdtfFilters->getConfig().secMapName + "_ActiveSector_Ev" + std::to_string(m_eventCounter);
    DNN::printNetwork<ActiveSector<StaticSectorType, TrackNode>, VoidMetaInfo>(activeSectorNetwork, fileName);
  }
}


template <class ObserverType>
bool SegmentNetworkProducerModule::buildTrackNodeNetwork()
{
  DirectedNodeNetwork<ActiveSector<StaticSectorType, Belle2::TrackNode>, VoidMetaInfo>& activeSectorNetwork =
    m_network->accessActiveSectorNetwork();
  DirectedNodeNetwork<Belle2::TrackNode, VoidMetaInfo>& hitNetwork = m_network->accessHitNetwork();

  int nAccepted = 0, nRejected = 0, nLinked = 0;;

  // loop over outer sectors to get their hits(->outerHits) and inner sectors
  for (auto* outerSector : activeSectorNetwork.getNodes()) {
    if (outerSector->getInnerNodes().empty()) {
      continue;
    }
    const vector<TrackNode*>& outerHits = outerSector->getEntry().getHits();
    if (outerHits.empty()) {
      continue;
    }

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
      if (innerHits.empty()) {
        continue;
      }

      //retrieve the filter, a null pointer is returned if there is no filter
      const auto* filter2sp = outerStaticSector->getFilter2sp(innerSector->getEntry().getFullSecID());
      if (filter2sp == NULL) {
        continue;
      }

      for (TrackNode* outerHit : outerHits) {
        // skip double-adding of nodes into the network after first time found -> speeding up the code:
        bool wasAnythingFoundSoFar = false;

        std::int32_t outerNodeID = outerHit->getID();
        hitNetwork.addNode(outerNodeID, *outerHit);

        for (TrackNode* innerHit : innerHits) {
          // applying filters provided by the sectorMap:
          // ->observe() gives back an observed version of the filter (the default filter has the VoidObserver)
          bool accepted = (filter2sp->observe(ObserverType())).accept(outerHit->getHit(), innerHit->getHit());

          if (m_PARAMallFiltersOff) accepted = true; // bypass all filters

          if (accepted == false) {
            nRejected++;
            continue;
          }
          nAccepted++;

          std::int32_t innerNodeID = innerHit->getID();
          hitNetwork.addNode(innerNodeID, *innerHit);
          // store combination of hits in network:
          if (!wasAnythingFoundSoFar) {
            if (hitNetwork.linkNodes(outerNodeID, innerNodeID)) {
              nLinked++;
              wasAnythingFoundSoFar = true;
            }
          } else {
            hitNetwork.addInnerToLastOuterNode(innerNodeID);
          }

          if (hitNetwork.size() > m_PARAMmaxHitNetworkSize) {
            //if (nLinked > 10000) {
            B2ERROR("HitNetwork has exceeded maximal size limit of " << m_PARAMmaxHitNetworkSize
                    << "! Processing of the event will be aborted. The HitNetwork size was = " << hitNetwork.size());
            return false;
          }
        }
      }
    }
  }
  m_nTrackNodesAccepted += nAccepted;
  m_nTrackNodesRejected += nRejected;
  m_nTrackNodeLinksCreated += nLinked;

  B2WARNING(">>>> :event:" << m_eventCounter << ": >> :trackNodeConnections:" << nLinked);

  if (m_PARAMprintNetworks) {
    std::string fileName = m_vxdtfFilters->getConfig().secMapName + "_TrackNode_Ev" + std::to_string(m_eventCounter);
    DNN::printNetwork<Belle2::TrackNode, VoidMetaInfo>(hitNetwork, fileName);
  }

  return true;
}


template <class ObserverType>
void SegmentNetworkProducerModule::buildSegmentNetwork()
{
  DirectedNodeNetwork<Belle2::TrackNode, VoidMetaInfo>& hitNetwork = m_network->accessHitNetwork();
  DirectedNodeNetwork<Segment<Belle2::TrackNode>, CACell>& segmentNetwork = m_network->accessSegmentNetwork();
  vector<Belle2::Segment<Belle2::TrackNode>* >& segments = m_network->accessSegments();
  int nAccepted = 0, nRejected = 0, nLinked = 0;
  for (DirectedNode<TrackNode, VoidMetaInfo>* outerHit : hitNetwork.getNodes()) {
    const vector<DirectedNode<TrackNode, VoidMetaInfo>*>& centerHits = outerHit->getInnerNodes();

    if (centerHits.empty()) {
      continue;
    }

    // get the point to the static sector
    const StaticSectorType* outerStaticSector = outerHit->getEntry().m_sector->getAttachedStaticSector();
    // should not happen, but just in case:
    if (outerStaticSector == NULL) {
      B2WARNING("Static sector not found. This should not happen!");
      continue;
    }

    for (DirectedNode<TrackNode, VoidMetaInfo>* centerHit : centerHits) {
      const vector<DirectedNode<TrackNode, VoidMetaInfo>*>& innerHits = centerHit->getInnerNodes();
      if (innerHits.empty()) {
        continue;
      }

      // skip double-adding of nodes into the network after first time found -> speeding up the code:
      bool wasAnythingFoundSoFar = false;
      for (DirectedNode<TrackNode, VoidMetaInfo>* innerHit : innerHits) {

        //retrieve the filter
        const auto* filter3sp = outerStaticSector->getFilter3sp(centerHit->getEntry().m_sector->getFullSecID(),
                                                                innerHit->getEntry().m_sector->getFullSecID());
        if (filter3sp == NULL) {
          continue;
        }

        // the filter accepts spacepoint combinations
        // ->observe gives back an observed version of the filter
        bool accepted = false;
        // there is an uncaught exception thrown by the CircleCenterXY filter variable if the points are on a straight line
        try {
          accepted = (filter3sp->observe(ObserverType())).accept(outerHit->getEntry().getHit(),
                                                                 centerHit->getEntry().getHit(),
                                                                 innerHit->getEntry().getHit());
        } catch (...) {
          B2WARNING("SegmentNetworkProducerModule: exception caught thrown by one of the three hit filters");
        }

        if (m_PARAMallFiltersOff) accepted = true; // bypass all filters

        if (accepted == false) {
          nRejected++;
          continue;
        }
        nAccepted++;

        std::int64_t innerSegmentID = static_cast<std::int64_t>(centerHit->getEntry().getID()) << 32 | static_cast<std::int64_t>
                                      (innerHit->getEntry().getID());
        if (not segmentNetwork.isNodeInNetwork(innerSegmentID)) {
          // create innerSegment first (order of storage in vector<segments> is irrelevant):
          Segment<TrackNode>* innerSegment = new Segment<TrackNode>(
            centerHit->getEntry().m_sector->getFullSecID(),
            innerHit->getEntry().m_sector->getFullSecID(),
            &centerHit->getEntry(),
            &innerHit->getEntry()
          );
          segments.push_back(innerSegment);
          segmentNetwork.addNode(innerSegmentID, *innerSegment);
        }

        std::int64_t outerSegmentID = static_cast<std::int64_t>(outerHit->getEntry().getID()) << 32 | static_cast<std::int64_t>
                                      (centerHit->getEntry().getID());
        if (not segmentNetwork.isNodeInNetwork(outerSegmentID)) {
          // create innerSegment first (order of storage in vector<segments> is irrelevant):
          Segment<TrackNode>* outerSegment = new Segment<TrackNode>(
            outerHit->getEntry().m_sector->getFullSecID(),
            centerHit->getEntry().m_sector->getFullSecID(),
            &outerHit->getEntry(),
            &centerHit->getEntry()
          );
          segments.push_back(outerSegment);
          segmentNetwork.addNode(outerSegmentID, *outerSegment);
        }

        // store combination of hits in network:
        if (!wasAnythingFoundSoFar) {
          if (segmentNetwork.linkNodes(outerSegmentID, innerSegmentID)) {
            nLinked++;
            wasAnythingFoundSoFar = true;
          }
        } else {
          segmentNetwork.addInnerToLastOuterNode(innerSegmentID);
        }

        if (segments.size() > m_PARAMmaxNetworkSize) {
          B2ERROR("SegmentNetwork size exceeds the limit of " << m_PARAMmaxNetworkSize
                  << ". Network size is " << segmentNetwork.size()
                  << ". VXDTF2 will abort the processing ot the event and the SegmentNetwork is cleared.");
          m_network->clear();
          return;
        }
      }
    }
  }
  B2WARNING(">>>> :event:" << m_eventCounter << ": >> :segmentConections:" << nLinked);

  if (m_PARAMprintNetworks) {
    std::string fileName = m_vxdtfFilters->getConfig().secMapName + "_Segment_Ev" + std::to_string(m_eventCounter);
    DNN::printNetwork<Segment<Belle2::TrackNode>, CACell>(segmentNetwork, fileName);
    DNN::printCANetwork<Segment<Belle2::TrackNode>>(segmentNetwork, "CA" + fileName);
  }
}
