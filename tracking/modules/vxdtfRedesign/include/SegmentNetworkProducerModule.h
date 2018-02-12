/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Felix Metzner                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// stl
#include <string>
#include <vector>
#include <utility>

//root
#include <TFile.h>

// fw
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <framework/geometry/B2Vector3.h>

// tracking
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>
#include <tracking/trackFindingVXD/segmentNetwork/StaticSector.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/FullSecID.h>
#include <tracking/dataobjects/SectorMapConfig.h>
#include <tracking/trackFindingVXD/filterMap/map/FiltersContainer.h>
#include <tracking/trackFindingVXD/environment/VXDTFFiltersHelperFunctions.h>


namespace Belle2 {

  /** The Segment Network Producer Module.
   *
   * This module takes a provided sectorMap and StoreArrays of spacePoints and creates an activeSectorNetwork,
   * a TrackNodeNetwork and a segmentNetwork by evaluating the given set of SpacePoints using the filters stored
   * in the sectorMap to reduce the possible combinations.
   * The output of the module is a StoreObjPtr to a DirectedNodeNetworkContainer which contains the three networks.
   */
  class SegmentNetworkProducerModule : public Module {

  public:
    /// Definition of the static sector type.
    using StaticSectorType = VXDTFFilters<SpacePoint>::staticSector_t;

    /// Simple struct for collecting raw data for a single sector
    struct RawSectorData {
      /// secID of rawSector
      FullSecID secID;

      /// Whether the sector was already added to the network
      bool wasCreated;

      /// Stores a sector if one is found, NULL else
      ActiveSector<StaticSectorType, TrackNode>* sector;

      /// Stores a static sector
      const StaticSectorType* staticSector;

      /// collects the hits found on this sector
      std::vector<Belle2::TrackNode*> hits;
    };


    /// Constructor of the module.
    SegmentNetworkProducerModule();


    /// Modul initialization: performing checks on input parameter and registration of network container in data store.
    void initialize() override;

    /// Begin Run which load the filters from the provided SectorMap and checks if this was successful.
    void beginRun() override
    {
      m_vxdtfFilters = m_filtersContainer.getFilters(m_PARAMsecMapName);
      if (m_vxdtfFilters == nullptr) B2FATAL("Requested secMapName '" << m_PARAMsecMapName
                                               << "' does not exist! Can not continue...");
    }


    /** Event function
     *  - Creation of TrackNodes form SpacePoints
     *  - Filling the activeSectorNetwork with all sectors with hits in the current event
     *  - Filling the trackNodeNetwork by building combinations of 2 TrackNodes (= 2 SpacePoints)
     *  - Filling the segmentNetwork by building combinations of segments (= 3 SpacePoints)
     */
    void event() override;


    /// Initialize counters.
    void InitializeCounters()
    {
      m_nSpacePointsTotal = 0;
      m_nSpacePointsInSectors = 0;
      m_nSpacePointsAsNodes = 0;
      m_nSectorsAsNodes = 0;
      m_nSegmentsAsNodes = 0;
      m_nOuterSectorLinks = 0;
      m_nInnerSectorLinks = 0;
      m_nOuterSpacePointLinks  = 0;
      m_nInnerSpacePointLinks = 0;
      m_nOuterSegmentLinks = 0;
      m_nInnerSegmentLinks = 0;
    }


    /// Create TrackNodes from SpacePoints and collect fullSecIDs of 'active' sectors with SpacePoints for the event.
    std::vector<RawSectorData> matchSpacePointToSectors();


    /// Returns pointer to static sector of a provided SpacePoint; returns NULL-ptr if no sector could be found.
    const StaticSectorType* findSectorForSpacePoint(const SpacePoint& aSP)
    {
      if (m_vxdtfFilters->areCoordinatesValid(aSP.getVxdID(), aSP.getNormalizedLocalU(), aSP.getNormalizedLocalV()) == false) {
        return nullptr;
      }

      FullSecID spSecID = m_vxdtfFilters->getFullID(aSP.getVxdID(), aSP.getNormalizedLocalU(), aSP.getNormalizedLocalV());
      const StaticSectorType* secPointer =  m_vxdtfFilters->getStaticSector(spSecID);
      return secPointer;
    }


    /** Builds a DirectedNodeNetwork<ActiveSector>, containing ActiveSectors which have SpacePoints
     *  and compatible inner- or outer neighbours */
    void buildActiveSectorNetwork(std::vector<RawSectorData>& collectedData);


    /** Evaluate TrackNodes in the ActiveSectors and link them if they fulfill the filter criteria of the SectorMap.
     *  The linked TrackNodes are stored in the DirectedNodeNetwork<TrackNode>.
     *  The boolean return value will be false, if the network cration is aborted prematurely.
     *  @param ObserverType: type of the observer which is used to monitor the Filters, use VoidObserver for deactivating observation
     */
    template <class ObserverType>
    bool buildTrackNodeNetwork();


    /** Use connected SpacePoints to form segments which will stored and linked in a DirectedNodeNetwork<Segment>
    @param ObserverType : type  of the observer which is used to monitor the Filters, use VoidObserver for deactivating observation
    */
    template <class ObserverType>
    void buildSegmentNetwork();


  protected:
    /** Module Parameters */
    /// Vector with SpacePoint storeArray names.
    std::vector<std::string> m_PARAMSpacePointsArrayNames = {"SVDSpacePoints", "PXDSpacePoints"};

    /// Name for network container data store object created by this module.
    std::string m_PARAMNetworkOutputName;

    /// Boolean to set whether to add an additional SpacePoint as a virtual interaction point.
    bool m_PARAMAddVirtualIP = false;

    /// Coordinates for virtual interaction point SpacePoint.
    std::vector<double> m_PARAMVirtualIPCoordinates = {0, 0, 0};

    /// Errors on coordinates for virtual interaction point SpacePoint.
    std::vector<double> m_PARAMVirtualIPErrors = {0.2, 0.2, 1.};

    /// Name of SectorMap used for this instance.
    std::string m_PARAMsecMapName = "testMap";

    /// If true for each event and each network a file with a graph of the network is created.
    bool m_PARAMprintNetworks = false;

    /** If true a file containing Mathematica code to generate a graph of the segment network is created. */
    bool m_PARAMprintToMathematica = false;

    /// If true, all filters are deactivated for all hit-combinations and therefore all combinations are accepted.
    bool m_PARAMallFiltersOff = false;

    /** Maximal size of SegmentNetwork; if exceeded, filling of the SegmentNetwork will be stopped, the
    *   SegmentNetwork will be cleared and the respective event will be skipped.
    */
    unsigned short m_PARAMmaxNetworkSize = 50000;

    /** Maximal size of hit network; if exceeded, filling of the HitNetwork will be stopped, the
    *  HitsNetwork will be cleared and the respective event will be skipped.
    */
    unsigned short m_PARAMmaxHitNetworkSize = 3000;


    /** Member Variables */
    /// Vector for coordinates of virtual IP.
    B2Vector3D m_virtualIPCoordinates;

    /// Vector for errors on coordinates of virtual IP.
    B2Vector3D m_virtualIPErrors;

    /// Reference to container which contains all the sector to filter maps and with it the VXDTFFilters.
    FiltersContainer<SpacePoint>& m_filtersContainer = FiltersContainer<SpacePoint>::getInstance();

    /// Pointer to the current filters, contains all sectorCombinations and Filters including cuts.
    VXDTFFilters<SpacePoint>* m_vxdtfFilters = nullptr;

    /// Contains all SPacePoint storeArrays to be evaluated.
    std::vector<StoreArray<Belle2::SpacePoint>> m_spacePoints;

    /// Access to the DirectedNodeNetwork, which will be produced by this module
    StoreObjPtr<DirectedNodeNetworkContainer> m_network;


    /** Counters */
    /// Current event number.
    unsigned int m_eventCounter = 0;

    // spacePoint-matching:
    /// Counts number of spacePoints accepted by secMap (spacepoint-to-sector-matching only).
    unsigned int m_nSPsFound = 0;
    /// Counts number of spacePoints rejected by secMap (spacepoint-to-sector-matching only).
    unsigned int m_nSPsLost = 0;
    /// Counts total number of Sectors with SpacePoints attached to them found (no double counting).
    unsigned int m_nRawSectorsFound = 0;

    // buildActiveSectorNetwork:
    /// Counts accepted number of Sectors with SpacePoints attached to them found (no double counting).
    unsigned int m_nGoodSectorsFound = 0;
    /// Number of sectorCombinations which were successfully linked.
    unsigned int m_nSectorsLinked = 0;
    /// Counts number of times a sector had an inner sector without hits (other inner sectors with hits still possible).
    unsigned int m_nBadSectorInnerNotActive = 0;
    /// Counts number of times a sector had inner sectors but none of them had any spacePoints.
    unsigned int m_nBadSectorNoInnerActive = 0;
    /// Counts number of times a sector had spacePoints but no inner sectors was attached at all.
    unsigned int m_nBadSectorNoInnerExisting = 0;

    // buildTrackNodeNetwork:
    /// Counts number of times a trackNode was accepted (same trackNode can be accepted/rejected more than once).
    unsigned int m_nTrackNodesAccepted = 0;
    /// Counts number of times a trackNode was rejected (same trackNode can be accepted/rejected more than once).
    unsigned int m_nTrackNodesRejected = 0;
    /// Counts number of times a link between tracknodes was created (unique per combination and map).
    unsigned int m_nTrackNodeLinksCreated = 0;

    // buildSegmentNetwork:
    /// Counts number of times a Segment was accepted (same Segment can be accepted/rejected more than once).
    unsigned int m_nSegmentsAccepted = 0;
    /// Counts number of times a Segment was rejected (same Segment can be accepted/rejected more than once).
    unsigned int m_nSegmentsRejected = 0;
    /// Counts number of times a link between tracknodes was created (unique per combination and map).
    unsigned int m_nSegmentsLinksCreated = 0;
    /// Counts number of Sectors which could be woven into the network
    unsigned int m_nSectorsAsNodes;
    /// Counts number of links between Sectors which could be woven into the network - outerDirection
    unsigned int m_nOuterSectorLinks;
    /// Counts number of links between Sectors which could be woven into the network - innerDirection
    unsigned int m_nInnerSectorLinks;
    /// Counts total number of SpacePoints occurred
    unsigned int m_nSpacePointsTotal;
    /// Counts number of SpacePoints which had sectors sectors
    unsigned int m_nSpacePointsInSectors;
    /// Counts number of SpacePoints which could be woven into the network
    unsigned int m_nSpacePointsAsNodes;
    /// Counts number of links between SpacePoints which could be woven into the network - outerDirection
    unsigned int m_nOuterSpacePointLinks;
    /// Counts number of links between SpacePoints which could be woven into the network - innerDirection
    unsigned int m_nInnerSpacePointLinks;
    /// Counts number of Segments which could be woven into the network
    unsigned int m_nSegmentsAsNodes;
    /// Counts number of links between Segments which could be woven into the network - outerDirection
    unsigned int m_nOuterSegmentLinks;
    /// Counts number of links between Segments which could be woven into the network - innerDirection
    unsigned int m_nInnerSegmentLinks;
  };
}

