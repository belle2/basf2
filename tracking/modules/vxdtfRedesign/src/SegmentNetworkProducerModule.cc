/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/vxdtfRedesign/SegmentNetworkProducerModule.h>
#include <tracking/trackFindingVXD/segmentNetwork/NodeNetworkHelperFunctions.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/trackFindingVXD/environment/VXDTFFiltersHelperFunctions.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/VoidObserver.h>

using namespace Belle2;

REG_MODULE(SegmentNetworkProducer);

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
           std::string(""));

  addParam("EventLevelTrackingInfoName",
           m_PARAMEventLevelTrackingInfoName,
           "Name of the EventLevelTrackingInfo that should be used (different one for ROI-finding).",
           std::string("EventLevelTrackingInfo"));

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

  addParam("maxConnections",
           m_PARAMmaxSegmentConnections,
           "Maximal number of Segment connections; if exceeded, the event execution will be skipped.",
           m_PARAMmaxSegmentConnections);

  addParam("maxAddedConnections",
           m_PARAMmaxSegmentAddedConnections,
           "Maximal number of added Segment connections; if exceeded, the event execution will be skipped.",
           m_PARAMmaxSegmentAddedConnections);

  addParam("maxHitConnections",
           m_PARAMmaxTrackNodeConnections,
           "Maximal number of Hit connections; if exceeded, the event execution will be skipped.",
           m_PARAMmaxTrackNodeConnections);

  addParam("maxAddedHitConnections",
           m_PARAMmaxTrackNodeAddedConnections,
           "Maximal number of added Hit connections; if exceeded, the event execution will be skipped.",
           m_PARAMmaxTrackNodeAddedConnections);
}

void SegmentNetworkProducerModule::initialize()
{
  if (m_PARAMVirtualIPCoordinates.size() != 3 or m_PARAMVirtualIPErrors.size() != 3) {
    B2FATAL("Parameters for virtualIP are wrong!");
  }

  // Get pointer to current filters to check if they exist. They must be reloaded for every run,
  // as the pointer will change if the DB object changes (see SectorMapBootStrapModule).
  auto filters = m_filtersContainer.getFilters(m_PARAMsecMapName);
  if (filters == nullptr) {
    B2FATAL("Requested secMapName '" << m_PARAMsecMapName << "' does not exist! Can not continue...");
  }

  m_virtualIPCoordinates = B2Vector3D(m_PARAMVirtualIPCoordinates.at(0),
                                      m_PARAMVirtualIPCoordinates.at(1),
                                      m_PARAMVirtualIPCoordinates.at(2));
  m_virtualIPErrors = B2Vector3D(m_PARAMVirtualIPErrors.at(0),
                                 m_PARAMVirtualIPErrors.at(1),
                                 m_PARAMVirtualIPErrors.at(2));

  if (m_PARAMprintToMathematica) {
    SecMapHelper::printStaticSectorRelations(*filters, filters->getConfig().secMapName + "segNetProducer", 2, m_PARAMprintToMathematica,
                                             true);
  }

  for (std::string& anArrayName : m_PARAMSpacePointsArrayNames) {
    m_spacePoints.push_back(StoreArray<SpacePoint>(anArrayName));
    m_spacePoints.back().isRequired();
  }

  m_network.registerInDataStore(m_PARAMNetworkOutputName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);

  m_eventLevelTrackingInfo.isRequired(m_PARAMEventLevelTrackingInfoName);
}


void SegmentNetworkProducerModule::event()
{
  m_eventCounter++;

  if (m_vxdtfFilters == nullptr) {
    B2FATAL("Requested secMapName '" << m_PARAMsecMapName << "' does not exist! Can not continue...");
  }

  // make sure that network exists:
  if (!m_network) {
    m_network.create();
  }

  std::vector<RawSectorData> collectedData = matchSpacePointToSectors();

  m_network->set_trackNodeConnections(0);
  m_network->set_activeSectorConnections(0);
  m_network->set_segmentConnections(0);
  m_network->set_trackNodeAddedConnections(0);
  m_network->set_activeSectorAddedConnections(0);
  m_network->set_segmentAddedConnections(0);
  m_network->set_collectedPaths(0);


  buildActiveSectorNetwork(collectedData);

  if (not buildTrackNodeNetwork<VoidObserver>()) {
    return;
  }

  buildSegmentNetwork<VoidObserver>();
}


std::vector<SegmentNetworkProducerModule::RawSectorData> SegmentNetworkProducerModule::matchSpacePointToSectors()
{
  std::vector<RawSectorData> collectedData; // contains the raw sectors to be activated
  std::deque<TrackNode>& trackNodes = m_network->accessTrackNodes(); // collects trackNodes
  int nCollected = 0;

  for (StoreArray<SpacePoint>& storeArray : m_spacePoints) {
    // match all SpacePoints with the sectors:
    for (SpacePoint& aSP : storeArray) {
      if (aSP.getAssignmentState()) {
        continue;
      }

      const StaticSectorType* sectorFound = findSectorForSpacePoint(aSP);

      if (sectorFound == nullptr) {
        B2WARNING("SpacePoint in sensor " << aSP.getVxdID() << " no sector found, SpacePoint discarded!");
        continue;
      }

      trackNodes.emplace_back(&aSP);

      // sector for SpacePoint exists:
      FullSecID foundSecID = sectorFound->getFullSecID();

      std::vector<RawSectorData>::iterator iter =
        std::find_if(collectedData.begin(), collectedData.end(),
                     [&](const RawSectorData & entry) -> bool { return entry.secID == foundSecID; }
                    );

      // if secID not in collectedData:
      if (iter == collectedData.end()) {
        collectedData.push_back({ foundSecID, false, nullptr, sectorFound, { & (trackNodes.back())}});
        nCollected++;
      } else {
        iter->hits.push_back(&(trackNodes.back()));
        nCollected++;
      }
    }
  }

  // store IP-coordinates
  if (m_PARAMAddVirtualIP) {
    m_network->setVirtualInteractionPoint(m_virtualIPCoordinates, m_virtualIPErrors);
    TrackNode* vIP = m_network->getVirtualInteractionPoint();
    const StaticSectorType* sectorFound = findSectorForSpacePoint((vIP->getHit()));
    collectedData.push_back({FullSecID(), false, nullptr, sectorFound, {vIP}}); // TODO: which FullSecID for the vIP?
  }

  m_network->set_trackNodesCollected(nCollected);
  return collectedData;
}


void SegmentNetworkProducerModule::buildActiveSectorNetwork(std::vector<SegmentNetworkProducerModule::RawSectorData>&
                                                            collectedData)
{
  // access (yet) empty activeSectorNetwork:
  DirectedNodeNetwork<ActiveSector<StaticSectorType, TrackNode>, VoidMetaInfo>& activeSectorNetwork =
    m_network->accessActiveSectorNetwork();
  // activeSectors are to be stored separately:
  std::deque<ActiveSector<StaticSectorType, TrackNode>>& activeSectors = m_network->accessActiveSectors();
  unsigned int nLinked = 0, nAdded = 0;

  // loop over all raw sectors found so far:
  for (RawSectorData& outerSectorData : collectedData) {
    ActiveSector<StaticSectorType, TrackNode> outerSector(outerSectorData.staticSector);
    std::int32_t outerEntryID = outerSector.getID();

    // skip double-adding of nodes into the network after first time found -> speeding up the code:
    bool wasAnythingFoundSoFar = false;
    // find innerSectors of outerSector and add them to the network:
    const std::vector<FullSecID>& innerSecIDs = outerSector.getInner2spSecIDs();

    for (const FullSecID& innerSecID : innerSecIDs) {
      std::int32_t innerEntryID = innerSecID;
      std::vector<RawSectorData>::iterator innerRawSecPos =
        std::find_if(collectedData.begin(), collectedData.end(),
                     [&](const RawSectorData & entry) -> bool { return (entry.secID == innerSecID); }
                    );

      // current inner sector has no SpacePoints in this event:
      if (innerRawSecPos == collectedData.end()) {
        continue;
      }

      // take care of inner sector first:
      if (!innerRawSecPos->wasCreated) { // was already there
        activeSectors.emplace_back(innerRawSecPos->staticSector);
        innerRawSecPos->wasCreated = true;
        innerRawSecPos->sector = &activeSectors.back();
        for (Belle2::TrackNode* hit : innerRawSecPos->hits) {
          hit->m_sector = &activeSectors.back();
        }
        // add all SpacePoints of this sector to ActiveSector:
        activeSectors.back().addHits(innerRawSecPos->hits);
        activeSectorNetwork.addNode(innerEntryID, activeSectors.back());
      }

      // when accepting combination the first time, take care of outer sector:
      if (!wasAnythingFoundSoFar) {
        activeSectors.push_back(outerSector);
        outerSectorData.wasCreated = true;
        outerSectorData.sector = &activeSectors.back();
        for (Belle2::TrackNode* hit : outerSectorData.hits) {
          hit->m_sector = &activeSectors.back();
        }
        // add all SpacePoints of this sector to ActiveSector:
        activeSectors.back().addHits(outerSectorData.hits);
        activeSectorNetwork.addNode(outerEntryID, activeSectors.back());

        if (activeSectorNetwork.linkNodes(outerEntryID, innerEntryID)) {
          wasAnythingFoundSoFar = true;
          nLinked++;
          nAdded++;
        }
      } else {
        if (activeSectorNetwork.addInnerToLastOuterNode(innerEntryID)) {
          nAdded++;
        }
      }
    }
  }

  m_network->set_activeSectorConnections(nLinked);
  m_network->set_activeSectorAddedConnections(nAdded);

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

  unsigned int nLinked = 0, nAdded = 0;

  // loop over outer sectors to get their hits(->outerHits) and inner sectors
  for (auto* outerSector : activeSectorNetwork.getNodes()) {
    if (outerSector->getInnerNodes().empty()) {
      continue;
    }
    const std::vector<TrackNode*>& outerHits = outerSector->getEntry().getHits();
    if (outerHits.empty()) {
      continue;
    }

    // get the point to the static sector
    const StaticSectorType* outerStaticSector = outerSector->getEntry().getAttachedStaticSector();
    // should not happen, but just in case:
    if (outerStaticSector == nullptr) {
      B2WARNING("Static sector not found. This should not happen!");
      continue;
    }

    // loop over inner sectors to get their hits(->innerHits) and check their compatibility
    for (auto* innerSector : outerSector->getInnerNodes()) {
      const std::vector<TrackNode*>& innerHits = innerSector->getEntry().getHits();
      if (innerHits.empty()) {
        continue;
      }

      //retrieve the filter, a null pointer is returned if there is no filter
      const auto* filter2sp = outerStaticSector->getFilter2sp(innerSector->getEntry().getFullSecID());
      if (filter2sp == nullptr) {
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

          if (!accepted) {
            continue;
          }

          std::int32_t innerNodeID = innerHit->getID();
          hitNetwork.addNode(innerNodeID, *innerHit);
          // store combination of hits in network:
          if (!wasAnythingFoundSoFar) {
            if (hitNetwork.linkNodes(outerNodeID, innerNodeID)) {
              nLinked++;
              nAdded++;
              wasAnythingFoundSoFar = true;
            }
          } else {
            if (hitNetwork.addInnerToLastOuterNode(innerNodeID)) {
              nAdded++;
            }
          }

          if (nLinked > m_PARAMmaxTrackNodeConnections) {
            B2WARNING("Number of TrackNodeConnections has exceeded maximal size limit of " << m_PARAMmaxTrackNodeConnections
                      << "! The event will be skipped and not be processed. The number of connections was = " << nLinked);
            m_eventLevelTrackingInfo->setVXDTF2AbortionFlag();
            m_network->set_trackNodeConnections(nLinked);
            m_network->set_trackNodeAddedConnections(nAdded);
            return false;
          }
          if (nAdded > m_PARAMmaxTrackNodeAddedConnections) {
            B2WARNING("Number of added TrackNodeConnections has exceeded maximal size limit of " << m_PARAMmaxTrackNodeAddedConnections
                      << "! The event will be skipped and not be processed. The number of connections was = " << nAdded);
            m_eventLevelTrackingInfo->setVXDTF2AbortionFlag();
            m_network->set_trackNodeConnections(nLinked);
            m_network->set_trackNodeAddedConnections(nAdded);
            return false;
          }
        }
      }
    }
  }
  m_network->set_trackNodeConnections(nLinked);
  m_network->set_trackNodeAddedConnections(nAdded);

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
  std::deque<Belle2::Segment<Belle2::TrackNode>>& segments = m_network->accessSegments();
  unsigned int nLinked = 0, nAdded = 0;

  for (DirectedNode<TrackNode, VoidMetaInfo>* outerHit : hitNetwork.getNodes()) {
    const std::vector<DirectedNode<TrackNode, VoidMetaInfo>*>& centerHits = outerHit->getInnerNodes();

    if (centerHits.empty()) {
      continue;
    }

    // get the point to the static sector
    const StaticSectorType* outerStaticSector = outerHit->getEntry().m_sector->getAttachedStaticSector();
    // should not happen, but just in case:
    if (outerStaticSector == nullptr) {
      B2WARNING("Static sector not found. This should not happen!");
      continue;
    }

    for (DirectedNode<TrackNode, VoidMetaInfo>* centerHit : centerHits) {
      const std::vector<DirectedNode<TrackNode, VoidMetaInfo>*>& innerHits = centerHit->getInnerNodes();
      if (innerHits.empty()) {
        continue;
      }

      // skip double-adding of nodes into the network after first time found -> speeding up the code:
      bool wasAnythingFoundSoFar = false;
      for (DirectedNode<TrackNode, VoidMetaInfo>* innerHit : innerHits) {

        //retrieve the filter
        const auto* filter3sp = outerStaticSector->getFilter3sp(centerHit->getEntry().m_sector->getFullSecID(),
                                                                innerHit->getEntry().m_sector->getFullSecID());
        if (filter3sp == nullptr) {
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

        if (!accepted) {
          continue;
        }

        std::int64_t innerSegmentID = static_cast<std::int64_t>(centerHit->getEntry().getID()) << 32 | static_cast<std::int64_t>
                                      (innerHit->getEntry().getID());

        if (not segmentNetwork.isNodeInNetwork(innerSegmentID)) {
          // create innerSegment first (order of storage in vector<segments> is irrelevant):
          segments.emplace_back(centerHit->getEntry().m_sector->getFullSecID(),
                                innerHit->getEntry().m_sector->getFullSecID(),
                                &centerHit->getEntry(),
                                &innerHit->getEntry());
          segmentNetwork.addNode(innerSegmentID, segments.back());
        }

        std::int64_t outerSegmentID = static_cast<std::int64_t>(outerHit->getEntry().getID()) << 32 | static_cast<std::int64_t>
                                      (centerHit->getEntry().getID());
        if (not segmentNetwork.isNodeInNetwork(outerSegmentID)) {
          segments.emplace_back(outerHit->getEntry().m_sector->getFullSecID(),
                                centerHit->getEntry().m_sector->getFullSecID(),
                                &outerHit->getEntry(),
                                &centerHit->getEntry());
          segmentNetwork.addNode(outerSegmentID, segments.back());
        }

        // store combination of hits in network:
        if (!wasAnythingFoundSoFar) {
          if (segmentNetwork.linkNodes(outerSegmentID, innerSegmentID)) {
            nLinked++;
            nAdded++;
            wasAnythingFoundSoFar = true;
          }
        } else {
          if (segmentNetwork.addInnerToLastOuterNode(innerSegmentID)) {
            nAdded++;
          }
        }

        if (nLinked > m_PARAMmaxSegmentConnections) {
          B2WARNING("Number of SegmentConnections exceeds the limit of " << m_PARAMmaxSegmentConnections
                    << ". VXDTF2 will skip the event and the SegmentNetwork is cleared.");
          m_eventLevelTrackingInfo->setVXDTF2AbortionFlag();
          m_network->set_segmentConnections(nLinked);
          m_network->set_segmentAddedConnections(nAdded);
          m_network->clear();
          return;
        }
        if (nAdded > m_PARAMmaxSegmentAddedConnections) {
          B2WARNING("Number of added SegmentConnections exceeds the limit of " << m_PARAMmaxSegmentAddedConnections
                    << ". VXDTF2 will skip the event and the SegmentNetwork is cleared.");
          m_eventLevelTrackingInfo->setVXDTF2AbortionFlag();
          m_network->set_segmentConnections(nLinked);
          m_network->set_segmentAddedConnections(nAdded);
          m_network->clear();
          return;
        }
        if (segments.size() > m_PARAMmaxNetworkSize) {
          B2WARNING("SegmentNetwork size exceeds the limit of " << m_PARAMmaxNetworkSize
                    << ". Network size is " << segmentNetwork.size()
                    << ". VXDTF2 will skip the event and the SegmentNetwork is cleared.");
          m_eventLevelTrackingInfo->setVXDTF2AbortionFlag();
          m_network->set_segmentConnections(nLinked);
          m_network->set_segmentAddedConnections(nAdded);
          m_network->clear();
          return;
        }
      }
    }
  }
  m_network->set_segmentConnections(nLinked);
  m_network->set_segmentAddedConnections(nAdded);

  if (m_PARAMprintNetworks) {
    std::string fileName = m_vxdtfFilters->getConfig().secMapName + "_Segment_Ev" + std::to_string(m_eventCounter);
    DNN::printNetwork<Segment<Belle2::TrackNode>, CACell>(segmentNetwork, fileName);
    DNN::printCANetwork<Segment<Belle2::TrackNode>>(segmentNetwork, "CA" + fileName);
  }
}
