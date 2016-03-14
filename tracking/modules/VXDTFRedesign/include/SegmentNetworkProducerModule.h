/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// fw
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

// tracking
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>
#include <tracking/trackFindingVXD/segmentNetwork/StaticSectorDummy.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/FullSecID.h>


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

    /** dummy declaration to get the module compiling without real static sectorMaps */
    struct StaticSectorMap {

      std::vector<StaticSectorDummy*> sectors;

      ~StaticSectorMap()
      {
        for (auto* aSector : sectors) { delete aSector; }
      }
      FullSecID getSecID() {return FullSecID(); }

      /** returns pointerTo a Static Sector */
      StaticSectorDummy* getSector(FullSecID aSecID)
      {
        sectors.push_back(new StaticSectorDummy(aSecID));
        return sectors.back();
      }
    };


    /** simple struct for collecting raw data for a single sector */
    struct RawSectorData {
      /** secID of rawSector */
      FullSecID secID;

      /** needed for creating ActiveSectorNetwork: if yes, the sector was already added to the network */
      bool wasCreated;

      /** stores a sector if one is found, NULL else */
      ActiveSector<StaticSectorDummy, TrackNode>* sector;

      /** stores a static sector */
      StaticSectorDummy* staticSector;

      /** collects the hits found on this sector */
      std::vector<Belle2::TrackNode*> hits;
    };


    /**
     * Constructor of the module.
     */
    SegmentNetworkProducerModule();


    /** Initializes the Module.
     */
    virtual void initialize()
    {
      InitializeCounters();

      if (m_PARAMCreateNeworks < 1 or m_PARAMCreateNeworks > 3) {
        B2FATAL("SegmentNetworkProducerModule::Initialize(): parameter 'createNeworks' is set to " << m_PARAMCreateNeworks <<
                "which is invalid, please read the documentation (basf2 - m SegmentNetworkProducer)!")
      }

      for (std::string& anArrayName : m_PARAMSpacePointsArrayNames) {
        m_spacePoints.push_back(StoreArray<SpacePoint>(anArrayName));
        m_spacePoints.back().isRequired();
      }
//    m_secMap.isRequired(m_PARAMStaticSectorMapName); // TODO add real SecMap!
      m_network.registerInDataStore(m_PARAMNetworkOutputName, DataStore::c_DontWriteOut);

      // TODO catch cases when m_network already existed in DataStore!
    }


    /**
     * Prints a header for each new run.
     */
    virtual void beginRun()
    {
      InitializeCounters();
    }


    /** Applies the Greedy algorithm at given sets of TCs. */
    virtual void event();


    /** Prints a footer for each run which ended. */
    virtual void endRun();


    /** initialize variables to avoid nondeterministic behavior */
    void InitializeCounters()
    {
      m_eventCounter = 0;
      m_nSpacePointsTotal = 0;
      m_nSpacePointsInSectors = 0;
      m_nSpacePointsAsNodes = 0;
      m_nSectorsFound = 0;
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
    Belle2::StaticSectorDummy* findSectorForSpacePoint(SpacePoint& aSP)
    {
      // TODO function
      /** Pseudo code of actual function:
       *
       * for (StaticSectorDummy& aSector : m_secMap)
       *   if aSP->getVxdID() != aSector.getVxdID() {continue;}
       *
       *   if (aSector.getFullSecID(aSP->getNormalizedLocalU(), aSP->getNormalizedLocalV()) != isValid) { continue;}
       *
       *   return &aSector;
       *
       * return NULL;
       */

      // dummy function here (for mockup):
      for (StaticSectorDummy* aSector : m_secMap.sectors) {
        if (FullSecID(aSP.getVxdID()) == *aSector) return aSector;
      }

      m_secMap.sectors.push_back(new StaticSectorDummy(FullSecID(aSP.getVxdID())));
      return m_secMap.sectors.back();
    }


    /** build a DirectedNodeNetwork< ActiveSector >, where all ActiveSectors are stored which have SpacePoints* and compatible inner- or outer neighbours */
    void buildActiveSectorNetwork(std::vector< RawSectorData >& collectedData);


    /** old name: segFinder. use SpacePoints stored in ActiveSectors to build SpacePoints which will stored and linked in a DirectedNodeNetwork< TrackNode > */
    void buildTrackNodeNetwork();


    /** old name: nbFinder. use connected SpacePoints to form segments which will stored and linked in a DirectedNodeNetwork< Segment > */
    void buildSegmentNetwork();



  protected:

// module parameters

    /** The output: a StoreObjPtr of DirectedNodeNetworkContainer:
     * - will contain a DirectedNodeNetwork< ActiveSector> (if parameter 'CreateNeworks' is [1;3])
     * - will contain a DirectedNodeNetwork< SpacePoint > (if parameter 'CreateNeworks' is  [2;3])
     * - will contain a DirectedNodeNetwork< Segment > (if parameter 'CreateNeworks' is [3])
     * - if 'CreateNeworks' is <1 or > 3 it will abort the run.
     */
    int m_PARAMCreateNeworks;

    /** contains names for storeArray with spacePoints in it */
    std::vector<std::string> m_PARAMSpacePointsArrayNames;

    /** sets the name of the sectorMap to be used. */
    //  std::string m_PARAMStaticSectorMapName;  // TODO add real SecMap!

    /** defines the unique name given to the output of this module - WARNING two instances of this module with the same name set in this parameter will abort the run! */
    std::string m_PARAMNetworkOutputName;

    /** if true, to the given SpacePoints a virtual interaction point at given coordinates with parameter 'virtualIPCoorindates' will be added */
    bool m_PARAMAddVirtualIP;

    /** only valid if nEntries == 3, excpects X, Y, and Z coordinates for virtual IP in global coordinates. Only used if addVirtualIP == true */
    std::vector<double> m_PARAMVirtualIPCoorindates;


// member variables

    // input containers

    /** contains storeArrays with SpacePoints in it */
    std::vector<StoreArray<Belle2::SpacePoint> > m_spacePoints;

    /** access to the static sectorMap, which will be used in this module */
    //  StoreObjPtr<StaticSectorMap> m_secMap;  // TODO add real SecMap!
    StaticSectorMap m_secMap; // WARNING temporal dummy


    // output containers

    /** access to the DirectedNodeNetwork, which will be produced by this module */
    StoreObjPtr<Belle2::DirectedNodeNetworkContainer> m_network;


// counters

    /** knows current event number */
    unsigned int m_eventCounter;

    /** counts total number of Sectors with SpacePoints attached to them found */
    unsigned int m_nSectorsFound;

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
