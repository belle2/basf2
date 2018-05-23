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

#include <vector>
#include <deque>

#include <framework/datastore/RelationsObject.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>
#include <tracking/trackFindingVXD/segmentNetwork/StaticSector.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/Segment.h>
#include <tracking/trackFindingVXD/segmentNetwork/ActiveSector.h>
#include <tracking/trackFindingVXD/segmentNetwork/VoidMetaInfo.h>
#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>


namespace Belle2 {
  /** The Container stores the output produced by the SegmentNetworkProducerModule.
   *
   * Is coupled to an assigned static sectorMap, which influences the structure of the DirectedNodeNetworks
   * stored in this container.
   * Is intended to be filled event-wise and not to be stored in root-files.
   *
   * WARNING: all data members which are followed by "//!" will not be streamed by root (so no I/O for them)
   *
   * TODO: create constructor for vIPs in SpacePoint. What about activeSectors for vIP?
   * -> solution dependent of treatment in static sectorMap.
   */
  class DirectedNodeNetworkContainer : public RelationsObject {
  public:
    /** To improve readability of the code, here the definition of the static sector type. */
    using StaticSectorType = VXDTFFilters<SpacePoint>::staticSector_t;


    /** Standard constructor */
    DirectedNodeNetworkContainer() :
      m_ActiveSectorNetwork(DirectedNodeNetwork<ActiveSector<StaticSectorType, Belle2::TrackNode>, Belle2::VoidMetaInfo>()),
      m_HitNetwork(DirectedNodeNetwork<Belle2::TrackNode, Belle2::VoidMetaInfo>()),
      m_SegmentNetwork(DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell>()),
      m_VirtualInteractionPoint(NULL),
      m_VIPSpacePoint(NULL) {}


    /** Destructor */
    ~DirectedNodeNetworkContainer()
    {
      if (m_VirtualInteractionPoint != NULL) { delete m_VirtualInteractionPoint; }
      if (m_VIPSpacePoint != NULL) { delete m_VIPSpacePoint; }
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
    /// Getters
    /** Returns reference to the ActiveSectorNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::ActiveSector<StaticSectorType, Belle2::TrackNode>, Belle2::VoidMetaInfo>&
    accessActiveSectorNetwork() { return m_ActiveSectorNetwork; }

    /** Returns reference to the actual ActiveSectors stored in this container, intended for read and write access */
    std::deque<Belle2::ActiveSector<StaticSectorType, Belle2::TrackNode>>& accessActiveSectors() { return m_activeSectors; }

    /** Returns reference to the actual trackNodes stored in this container, intended for read and write access */
    std::deque<Belle2::TrackNode>& accessTrackNodes() { return m_trackNodes; }

    /** Returns reference to the HitNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::TrackNode, Belle2::VoidMetaInfo>& accessHitNetwork() { return m_HitNetwork; }

    /** Returns reference to the SegmentNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >& accessSegmentNetwork() { return m_SegmentNetwork; }

    /** Returns reference to the actual segments stored in this container, intended for read and write access */
    std::deque<Belle2::Segment<Belle2::TrackNode>>& accessSegments() { return m_segments; }


    /** Returns number of activeSectors found. */
    int sizeActiveSectors() { return m_activeSectors.size(); }
    /** Returns number of trackNodes found. */
    int sizeTrackNodes() { return m_trackNodes.size(); }
    /** Returns number of segments found. */
    int sizeSegments() { return m_segments.size(); }

    /** Returns number of trackNodes collected. */
    int get_trackNodesCollected() { return m_trackNodesCollected; }
    /** Returns number of activeSectors connections made. */
    int get_activeSectorConnections() { return m_activeSectorConnections; }
    /** Returns number of trackNodes connections made. */
    int get_trackNodeConnections() { return m_trackNodeConnections; }
    /** Returns number of segments connections made. */
    int get_segmentConnections() { return m_segmentConnections; }
    /** Returns number of added activeSectors connections made. */
    int get_activeSectorAddedConnections() { return m_activeSectorAddedConnections; }
    /** Returns number of added trackNodes connections made. */
    int get_trackNodeAddedConnections() { return m_trackNodeAddedConnections; }
    /** Returns number of added segments connections made. */
    int get_segmentAddedConnections() { return m_segmentAddedConnections; }
    /** Returns number of paths found. */
    int get_collectedPaths() { return m_collectedPaths; }

    /** Sets number of trackNodes collected. */
    void set_trackNodesCollected(int in) { m_trackNodesCollected = in; }
    /** Sets number of activeSectors connections made. */
    void set_activeSectorConnections(int in) { m_activeSectorConnections = in; }
    /** Sets number of trackNodes connections made. */
    void set_trackNodeConnections(int in) { m_trackNodeConnections = in; }
    /** Sets number of segments connections made. */
    void set_segmentConnections(int in) { m_segmentConnections = in; }
    /** Sets number of added activeSectors connections made. */
    void set_activeSectorAddedConnections(int in) { m_activeSectorAddedConnections = in; }
    /** Sets number of added trackNodes connections made. */
    void set_trackNodeAddedConnections(int in) { m_trackNodeAddedConnections = in; }
    /** Sets number of added segments connections made. */
    void set_segmentAddedConnections(int in) { m_segmentAddedConnections = in; }
    /** Sets number of paths found. */
    void set_collectedPaths(int in) { m_collectedPaths = in; }

    /** Clear directed node network container
     * Called to clear the directed node network container if the segment network size grows to large.
     * This is necessary to prevent to following modules from processing events with a only partly filled network.
     */
    void clear()
    {
      m_SegmentNetwork.clear();
    }

    /** Passes parameters for creating a virtual interaction point */
    void setVirtualInteractionPoint(B2Vector3D& pos, B2Vector3D& posError)
    {
      m_VIPSpacePoint = new SpacePoint(pos, posError, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
      m_VirtualInteractionPoint = new Belle2::TrackNode(m_VIPSpacePoint);
    }

    /** Returns reference to the Virtual interactionPoint stored here */
    Belle2::TrackNode* getVirtualInteractionPoint() { return m_VirtualInteractionPoint; }


  private:
    /// Making the Class a ROOT class
    /// last member changed: added metaInfo for DirectedNodeNetwork
    ClassDef(DirectedNodeNetworkContainer, 10)

  protected:
    /** Number of trackNodes collected. */
    int m_trackNodesCollected = 0;
    /** Number of activeSectors connections made. */
    int m_activeSectorConnections = 0;
    /** Number of trackNodes connections made. */
    int m_trackNodeConnections = 0;
    /** Number of segments connections made. */
    int m_segmentConnections = 0;
    /** Number of added activeSectors connections made. */
    int m_activeSectorAddedConnections = 0;
    /** Number of added trackNodes connections made. */
    int m_trackNodeAddedConnections = 0;
    /** Number of added segments connections made. */
    int m_segmentAddedConnections = 0;
    /** Number of paths found. */
    int m_collectedPaths = 0;

    /** ************************* DATA MEMBERS ************************* */
    /** Stores the full network of activeSectors, which contain hits in that event and have compatible Sectors with hits too*/
    DirectedNodeNetwork<ActiveSector<StaticSectorType, TrackNode>, Belle2::VoidMetaInfo> m_ActiveSectorNetwork;

    /** Stores the actual ActiveSectors, since the ActiveSectorNetwork does only keep references. */
    std::deque<ActiveSector<StaticSectorType, TrackNode>> m_activeSectors;

    /** Stores the full network of TrackNode< SpaacePoint>, which were accepted by activated two-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<TrackNode, Belle2::VoidMetaInfo> m_HitNetwork;

    /** Stores the actual trackNodes, since the SegmentNetwork does only keep references. */
    std::deque<TrackNode> m_trackNodes;

    /** Stores the full network of Segments, which were accepted by activated three-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<Segment<TrackNode>, Belle2::CACell> m_SegmentNetwork;

    /** Stores the actual Segments, since the SegmentNetwork does only keep references. */
    std::deque<Segment<TrackNode>> m_segments;

    /** Stores a SpacePoint representing the virtual interaction point if set, NULL if not. */
    Belle2::TrackNode* m_VirtualInteractionPoint;

    /** Stores the SpacePoint needed for the virtual IP */
    SpacePoint* m_VIPSpacePoint;
  };
}
