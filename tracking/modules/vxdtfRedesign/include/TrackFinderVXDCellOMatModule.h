/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// fw
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>

// tracking
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <tracking/dataobjects/FullSecID.h>
#include <tracking/dataobjects/SectorMapConfig.h>

#include <tracking/trackFindingVXD/algorithms/CellularAutomaton.h>
#include <tracking/trackFindingVXD/algorithms/PathCollectorRecursive.h>
#include <tracking/trackFindingVXD/algorithms/CALogger.h>
#include <tracking/trackFindingVXD/algorithms/CAValidator.h>
#include <tracking/trackFindingVXD/algorithms/NodeCompatibilityCheckerPathCollector.h>

#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>

#include <tracking/trackFindingVXD/filterMap/map/FiltersContainer.h>

#include <tracking/trackFindingVXD/tcTools/SpacePointTrackCandCreator.h>


namespace Belle2 {
  /** The TrackFinderVXDCellOMatModule is a low momentum Si-only trackfinder.
   *
   * It uses the output produced by the SegmentNetworkProducerModule to create SpacePointTrackCands using a Cellular Automaton algorithm implementation.
   */
  class TrackFinderVXDCellOMatModule : public Module {

  public:


    /** Constructor */
    TrackFinderVXDCellOMatModule();


    /** Destructor */
    virtual ~TrackFinderVXDCellOMatModule() {}


    /** initialize */
    virtual void initialize();


    /** beginRun */
    virtual void beginRun() {}


    /** event */
    virtual void event();


    /** endRun */
    virtual void endRun() {}


    /** terminate */
    virtual void terminate() {}



    /** *************************************+************************************* **/
    /** ***********************************+ + +*********************************** **/
    /** *******************************+ functions +******************************* **/
    /** ***********************************+ + +*********************************** **/
    /** *************************************+************************************* **/



  protected:

/// module parameters


    /** name for StoreArray< SpacePointTrackCand> to be filled */
    std::string m_PARAMSpacePointTrackCandArrayName;


    /** name for StoreObjPtr< DirectedNodeNetwork> which contains the networks needed */
    std::string m_PARAMNetworkName;


    /** the name of the SectorMap used for this instance. */
    std::string m_PARAMsecMapName;


    /** If true for each event and each network created a file with a graph is created. */
    bool m_PARAMprintNetworks;


    /** Regulates if every subset of sufficient length of a path shall be collected as separate path or not.
     *
     * if true, only one path per possibility is collected, if false subsets are collected too. */
    bool m_PARAMstrictSeeding;

    /** If true, the virtual interaction Point will be removed from the track candidates. */
    bool m_PARAMremoveVirtualIP = false;

/// member variables

    /** CA algorithm */
    CellularAutomaton<Belle2::DirectedNodeNetwork< Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >, Belle2::CAValidator<Belle2::CACell>, Belle2::CALogger>
    m_cellularAutomaton;

    /** algorithm for finding paths of segments */
    PathCollectorRecursive <
    Belle2::DirectedNodeNetwork< Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >,
           Belle2::DirectedNode<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell>,
           std::vector<Belle2::DirectedNode<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell>*>,
           Belle2::NodeCompatibilityCheckerPathCollector<Belle2::DirectedNode<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell>>
           > m_pathCollector;

    /** tool for creating SPTCs, fills storeArray directly */
    SpacePointTrackCandCreator<StoreArray<Belle2::SpacePointTrackCand>> m_sptcCreator;


    /// input containers

    /** access to the DirectedNodeNetwork, which contains the network needed for creating TrackCandidates */
    StoreObjPtr<Belle2::DirectedNodeNetworkContainer> m_network;


    /** contains the maps from sectors to filters (only needed for loading the configuration). */
    FiltersContainer<SpacePoint>& m_filtersContainer = FiltersContainer<SpacePoint>::getInstance();


    /** contains the configuration-settings for this run. */
    SectorMapConfig m_config;


    /// output containers

    /** StoreArray for the TCs created in this module */
    StoreArray<Belle2::SpacePointTrackCand> m_TCs;


    /// counters and other debug stuff:
    /** counts event numbers */
    unsigned int m_eventCounter = 0;

  private:
  };
} // Belle2 namespace
