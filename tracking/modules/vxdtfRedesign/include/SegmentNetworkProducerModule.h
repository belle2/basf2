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

#include <string>
#include <vector>
#include <utility>

#include <TFile.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <framework/geometry/B2Vector3.h>

#include <mdst/dataobjects/EventLevelTrackingInfo.h>

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

      /// Stores a sector if one is found, nullptr else
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
      if (m_vxdtfFilters == nullptr) {
        B2FATAL("Requested secMapName '" << m_PARAMsecMapName << "' does not exist! Can not continue...");
      }
    }

    /** Event function
     *  - Creation of TrackNodes form SpacePoints
     *  - Filling the activeSectorNetwork with all sectors with hits in the current event
     *  - Filling the trackNodeNetwork by building combinations of 2 TrackNodes (= 2 SpacePoints)
     *  - Filling the segmentNetwork by building combinations of segments (= 3 SpacePoints)
     */
    void event() override;


    /// Create TrackNodes from SpacePoints and collect fullSecIDs of 'active' sectors with SpacePoints for the event.
    std::vector<RawSectorData> matchSpacePointToSectors();


    /// Returns pointer to static sector of a provided SpacePoint; returns nullptr if no sector could be found.
    const StaticSectorType* findSectorForSpacePoint(const SpacePoint& aSP)
    {
      if (not m_vxdtfFilters->areCoordinatesValid(aSP.getVxdID(), aSP.getNormalizedLocalU(), aSP.getNormalizedLocalV())) {
        return nullptr;
      }

      FullSecID spSecID = m_vxdtfFilters->getFullID(aSP.getVxdID(), aSP.getNormalizedLocalU(), aSP.getNormalizedLocalV());
      return m_vxdtfFilters->getStaticSector(spSecID);
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

    /// Maximal size of SegmentNetwork; if exceeded, filling of SegmentNetwork will be stopped and the event skipped.
    unsigned short m_PARAMmaxNetworkSize = 40000;

    /// Maximal number of Segment connections; if exceeded, filling of SegmentNetwork will be stopped and the event skipped.
    unsigned int m_PARAMmaxSegmentConnections = 30000;

    /// Maximal number of added Segment connections; if exceeded, filling of SegmentNetwork will be stopped and the event skipped.
    unsigned int m_PARAMmaxSegmentAddedConnections = 300000;

    /// Maximal number of hit connections; if exceeded, filling of HitNetwork will be stopped and the event skipped.
    unsigned short m_PARAMmaxTrackNodeConnections = 8000;

    /// Maximal number of added hit connections; if exceeded, filling of HitNetwork will be stopped and the event skipped.
    unsigned int m_PARAMmaxTrackNodeAddedConnections = 200000;


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

    /// Access to the DirectedNodeNetwork, which will be produced by this module.
    StoreObjPtr<DirectedNodeNetworkContainer> m_network;

    /// Acccess to the EventLevelTrackingInfo object in the datastore.
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;


    /** Counters */
    /// Current event number.
    unsigned int m_eventCounter = 0;
  };
}

