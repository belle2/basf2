/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Jonas Wagner, Felix Metzner         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <tracking/trackFindingVXD/algorithms/CellularAutomaton.h>
#include <tracking/trackFindingVXD/algorithms/PathCollectorRecursive.h>
#include <tracking/trackFindingVXD/algorithms/NodeFamilyDefiner.h>
#include <tracking/trackFindingVXD/algorithms/SPTCSelectorXBestPerFamily.h>

#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetworkContainer.h>

#include <tracking/trackFindingVXD/tcTools/SpacePointTrackCandCreator.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/trackFindingVXD/algorithms/CAValidator.h>
#include <tracking/trackFindingVXD/algorithms/NodeCompatibilityCheckerBase.h>


namespace Belle2 {
  /** The TrackFinderVXDBasicPathFinder is a low momentum Si-only trackfinder.
   * It uses the output produced by the SegmentNetworkProducerModule to create SpacePointTrackCands
   * by simply storing all possible paths stored in the SegmentNetwork.
   */
  class TrackFinderVXDBasicPathFinderModule final : public Module {
  private:
    using NodeType = Belle2::DirectedNode<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell>;
    using NodeNetworkType = Belle2::DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell>;
    using Path = std::vector<NodeType*>;

  public:

    /** Constructor */
    TrackFinderVXDBasicPathFinderModule();

    /** initialize */
    void initialize() override;

    /** beginRun */
    void beginRun() override;

    /** event */
    void event() override;


  protected:

    /// module parameters
    /** name for StoreArray<SpacePointTrackCand> to be filled */
    std::string m_PARAMSpacePointTrackCandArrayName;

    /** name for StoreObjPtr<DirectedNodeNetwork> which contains the networks needed */
    std::string m_PARAMNetworkName;

    /** the name of the SectorMap used for this instance. */
    std::string m_PARAMsecMapName;

    /** Path collection obtained from evaluation of the provided segment network. */
    std::vector<Path> m_collectedPaths;

    /** If true for each event and each network created a file with a graph is created. */
    bool m_PARAMprintNetworks;

    /** Regulates if every node with enough nodes below it is used as a seed or only the outermost nodes. */
    bool m_PARAMstrictSeeding;

    /** Regulates if every subset of sufficient length of a path shall be collected as separate path or not. */
    bool m_PARAMstoreSubsets;

    /** If true additionally assign a common family identifier to all Tracks that are share a node. */
    bool m_PARAMsetFamilies;

    /** If true create track candidate only for the best candidate of a family. */
    bool m_PARAMselectBestPerFamily;

    /** Maximal number of best candidates to be created per family. */
    unsigned short m_PARAMxBestPerFamily = 5;

    /** Maximal number of families in event; if exceeded, the execution of the trackfinder will be stopped. */
    unsigned short m_PARAMmaxFamilies = 10000;

    /** Maximal number of paths per event; if exceeded, the execution of the trackfinder will be stopped. */
    unsigned int m_PARAMmaxPaths = 300000;

    /// member variables
    /** CA algorithm */
    CellularAutomaton<NodeNetworkType, Belle2::CAValidator<Belle2::CACell>> m_cellularAutomaton;

    /** Algorithm for finding paths of segments. */
    PathCollectorRecursive<NodeNetworkType, NodeType, Path, Belle2::NodeCompatibilityCheckerBase<NodeType>> m_pathCollector;

    /** Tool for creating SPTCs, which fills storeArray directly. */
    SpacePointTrackCandCreator<StoreArray<Belle2::SpacePointTrackCand>> m_sptcCreator;

    /** Class to evaluate connected nodes, in this case for the directed node network, and assigns a family to each
     *  cluster of connected nodes. */
    NodeFamilyDefiner<NodeNetworkType, NodeType, Path> m_familyDefiner;

    /// input containers
    /** Access to the DirectedNodeNetwork, which contains the network needed for creating TrackCandidates. */
    StoreObjPtr<Belle2::DirectedNodeNetworkContainer> m_network;

    /// output containers
    /** StoreArray for the TCs created in this module. */
    StoreArray<Belle2::SpacePointTrackCand> m_TCs;

    /** Pointer to SPTC selector class which performes the x best candidate selection. */
    std::unique_ptr<SPTCSelectorXBestPerFamily> m_sptcSelector;

    /** Event number counter. */
    unsigned int m_eventCounter = 0;
  };
}
