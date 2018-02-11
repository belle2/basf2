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


//root
#include <TFile.h>

// stl
#include <string>
#include <vector>
#include <utility>      // std::pair, std::move


namespace Belle2 {

  /** The segment network producer module.
   *
   * This module takes a given sectorMap and storeArrays of spacePoints and creates a segmentNetwork (if activated).
   * The output: a StoreObjPtr of DirectedNodeNetworkContainer:
   * - will contain a DirectedNodeNetwork< ActiveSector> (if parameter 'createNeworks' is [1;3])
   * - will contain a DirectedNodeNetwork< SpacePoint > (if parameter 'createNeworks' is  [2;3])
   * - will contain a DirectedNodeNetwork< Segment > (if parameter 'createNeworks' is [3])
   */
  class SegmentNetworkProducerModule : public Module {

  public:
    /// Definition of the static sector type.
    using StaticSectorType = VXDTFFilters<SpacePoint>::staticSector_t;

    /** simple struct for collecting raw data for a single sector */
    struct RawSectorData {
      /** secID of rawSector */
      FullSecID secID;

      /** needed for creating ActiveSectorNetwork: if yes, the sector was already added to the network */
      bool wasCreated;

      /** stores a sector if one is found, NULL else */
      ActiveSector<StaticSectorType, TrackNode>* sector;

      /** stores a static sector */
      const StaticSectorType* staticSector;

      /** collects the hits found on this sector */
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


    /** builds the SegmentNetwork and the TrackNodeNetwork  */
    void event() override;


    /** initialize variables to avoid nondeterministic behavior */
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


    /** for each SpacePoint given, find according sector and store them in a fast and intermediate way* */
    std::vector< RawSectorData > matchSpacePointToSectors();


    /** returns a NULL-ptr if no sector found, returns pointer to the static sector if found */
    const StaticSectorType* findSectorForSpacePoint(const SpacePoint& aSP)
    {
      if (m_vxdtfFilters->areCoordinatesValid(aSP.getVxdID(), aSP.getNormalizedLocalU(), aSP.getNormalizedLocalV()) == false) {
        B2DEBUG(1, "SegmentNetworkProducerModule()::findSectorForSpacePoint(): spacepoint " << aSP.getArrayIndex() <<
                " has no valid FullSecID: " << aSP);
        return nullptr;
      }

      FullSecID spSecID = m_vxdtfFilters->getFullID(aSP.getVxdID(), aSP.getNormalizedLocalU(), aSP.getNormalizedLocalV());
      const StaticSectorType* secPointer =  m_vxdtfFilters->getStaticSector(spSecID);
      B2DEBUG(1, "SegmentNetworkProducerModule()::findSectorForSpacePoint(): spacepoint " << aSP.getArrayIndex() <<
              " got valid FullSecID of " << spSecID.getFullSecString() <<
              " with pointer-adress (if this is a nullptr, vxdtffilters does not have anything stored): " << secPointer);

      return secPointer;
    }


    /** build a DirectedNodeNetwork< ActiveSector >, where all ActiveSectors are stored which have SpacePoints* and compatible inner- or outer neighbours */
    void buildActiveSectorNetwork(std::vector< RawSectorData >& collectedData);


    /** Evaluate TrackNodes in the ActiveSectors and link them if they fulfill the filter criteria of the SectorMap.
     *  The linked TrackNodes are stored in the DirectedNodeNetwork<TrackNode>.
     *  The boolean return value will be false, if the network cration is aborted prematurely.
     *  @param ObserverType: type of the observer which is used to monitor the Filters, use VoidObserver for deactivating observation
     */
    template < class ObserverType >
    bool buildTrackNodeNetwork();


    /** Use connected SpacePoints to form segments which will stored and linked in a DirectedNodeNetwork< Segment >
    @param ObserverType : type  of the observer which is used to monitor the Filters, use VoidObserver for deactivating observation
    */
    template < class ObserverType >
    void buildSegmentNetwork();


  protected:
    /// module parameters
    /// Vector with SpacePoint storeArray names.
    std::vector<std::string> m_PARAMSpacePointsArrayNames = {"SVDSpacePoints", "PXDSpacePoints"};

    /// Name for network container data store object created by this module.
    std::string m_PARAMNetworkOutputName;

    /** if true, to the given SpacePoints a virtual interaction point at given coordinates with parameter 'virtualIPCoorindates' will be added */
    bool m_PARAMAddVirtualIP = false;

    /**  expects X, Y, and Z coordinates for virtual IP in global coordinates (only lists with 3 coordinates are allowed!). Only used if addVirtualIP == true. */
    std::vector<double> m_PARAMVirtualIPCoordinates = {0, 0, 0};

    /** expects errors for X, Y, and Z coordinates for virtual IP in global coordinates (only lists with 3 coordinates are allowed!). Only used if addVirtualIP == true. */
    std::vector<double> m_PARAMVirtualIPErrors = {0.2, 0.2, 1.};

    /** the name of the SectorMap used for this instance. */
    std::string m_PARAMsecMapName = "testMap";

    /** If true for each event and each network created a file with a graph is created. */
    bool m_PARAMprintNetworks = false;

    /** If true a file containing Mathematica code to generate a graph of the segment network is created. */
    bool m_PARAMprintToMathematica = false;

    /** For debugging purposes: if true, all filters are deactivated for all hit-combinations and therefore all combinations are accepted. */
    bool m_PARAMallFiltersOff = false;

    /** Maximal size of SegmentNetwork; if exceeded, filling of the SegmentNetwork will be stopped, the
    *   SegmentNetwork will be cleared and the respective event will be skipped.
    */
    unsigned short m_PARAMmaxNetworkSize = 50000;

    /** Maximal size of hit network; if exceeded, filling of the HitNetwork will be stopped, the
    *  HitsNetwork will be cleared and the respective event will be skipped.
    */
    unsigned short m_PARAMmaxHitNetworkSize = 3000;

    /** integer switch to decide which observer to use: see enum eObserverTypes
      NOTE: that observing filters make the code slow! So only use for debugging purposes
    */
    int m_PARAMobserverType;

// member variables

    /** pointer to a root file needed to store the filter-variable responses if filter are observed */
    TFile* m_tfile = nullptr;

    /** vector containing global coordinates for virtual IP, is set using module parameters. */
    B2Vector3D m_virtualIPCoordinates;

    /** vector containing global errors for virtual IP, is set using module parameters. */
    B2Vector3D m_virtualIPErrors;

    // input containers
    /** reference to the container which contains all the sector to filter maps and with it the VXDTFFilters. */
    FiltersContainer<SpacePoint>& m_filtersContainer = FiltersContainer<SpacePoint>::getInstance();

    /** pointer to the current filter, contains all sectorCombinations and Filters including cuts. */
    // WARNING: the pointer will change if the DB object is updated!
    VXDTFFilters<SpacePoint>* m_vxdtfFilters = nullptr;

    /** contains storeArrays with SpacePoints in it */
    std::vector<StoreArray<Belle2::SpacePoint> > m_spacePoints;

    // output containers
    /** access to the DirectedNodeNetwork, which will be produced by this module */
    StoreObjPtr<DirectedNodeNetworkContainer> m_network;


// counters

    /** knows current event number */
    unsigned int m_eventCounter = 0;

    // spacePoint-matching:

    /** counts number of spacePoints accepted by secMap (spacepoint-to-sector-matching only). */
    unsigned int m_nSPsFound = 0;

    /** counts number of spacePoints rejected by secMap (spacepoint-to-sector-matching only). */
    unsigned int m_nSPsLost = 0;

    /** counts total number of Sectors with SpacePoints attached to them found (double entries between several events are ignored.) - spacepoint-matching only */
    unsigned int m_nRawSectorsFound = 0;

    // buildActiveSectorNetwork:

    /** counts accepted number of Sectors with SpacePoints attached to them found (double entries between several events are ignored.) - activeSector-creation only - this is one step after spacePointMatching. */
    unsigned int m_nGoodSectorsFound = 0;

    /** number of sectorCombinations which were successfully linked. */
    unsigned int m_nSectorsLinked = 0;

    /** counts number of times a sector had an inner sector without hits (does not mean that this sector had no other inner sectors with hits). */
    unsigned int m_nBadSectorInnerNotActive = 0;

    /** counts number of times a sector had inner sectors bot none of them had any spacePoints. */
    unsigned int m_nBadSectorNoInnerActive = 0;

    /** counts number of times a sector had spacePoints but no inner sectors was attached at all. */
    unsigned int m_nBadSectorNoInnerExisting = 0;

    // buildTrackNodeNetwork:
    /** counts number of times a trackNode was accepted (same trackNode can be accepted/rejected more than once -> combinations relevant). */
    unsigned int m_nTrackNodesAccepted = 0;

    /** counts number of times a trackNode was rejected (same trackNode can be accepted/rejected more than once -> combinations relevant). */
    unsigned int m_nTrackNodesRejected = 0;

    /** counts number of times a link between tracknodes was created (unique per combination and map). */
    unsigned int m_nTrackNodeLinksCreated = 0;

    // buildSegmentNetwork:
    /** counts number of times a Segment was accepted (same Segment can be accepted/rejected more than once -> combinations relevant). */
    unsigned int m_nSegmentsAccepted = 0;

    /** counts number of times a Segment was rejected (same Segment can be accepted/rejected more than once -> combinations relevant). */
    unsigned int m_nSegmentsRejected = 0;

    /** counts number of times a link between tracknodes was created (unique per combination and map). */
    unsigned int m_nSegmentsLinksCreated = 0;

    /** counts number of Sectors which could be woven into the network */
    unsigned int m_nSectorsAsNodes;

    /** counts number of links between Sectors which could be woven into the network - outerDirection */
    unsigned int m_nOuterSectorLinks;

    /** counts number of links between Sectors which could be woven into the network - innerDirection */
    unsigned int m_nInnerSectorLinks;

    /** counts total number of SpacePoints occurred */
    unsigned int m_nSpacePointsTotal;

    /** counts number of SpacePoints which had sectors sectors */
    unsigned int m_nSpacePointsInSectors;

    /** counts number of SpacePoints which could be woven into the network */
    unsigned int m_nSpacePointsAsNodes;

    /** counts number of links between SpacePoints which could be woven into the network - outerDirection */
    unsigned int m_nOuterSpacePointLinks;

    /** counts number of links between SpacePoints which could be woven into the network - innerDirection */
    unsigned int m_nInnerSpacePointLinks;

    /** counts number of Segments which could be woven into the network */
    unsigned int m_nSegmentsAsNodes;

    /** counts number of links between Segments which could be woven into the network - outerDirection */
    unsigned int m_nOuterSegmentLinks;

    /** counts number of links between Segments which could be woven into the network - innerDirection */
    unsigned int m_nInnerSegmentLinks;

  private:
  };
}

