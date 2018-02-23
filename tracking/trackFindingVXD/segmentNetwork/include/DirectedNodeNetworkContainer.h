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
      m_ActiveSectorNetwork(DirectedNodeNetwork<ActiveSector<StaticSectorType, Belle2::TrackNode>, Belle2::VoidMetaInfo >()),
      m_HitNetwork(DirectedNodeNetwork<Belle2::TrackNode, Belle2::VoidMetaInfo>()),
      m_SegmentNetwork(DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >()),
      m_VirtualInteractionPoint(NULL),
      m_VIPSpacePoint(NULL) {}


    /** Destructor */
    ~DirectedNodeNetworkContainer()
    {
      if (m_VirtualInteractionPoint != NULL) { delete m_VirtualInteractionPoint; }
      if (m_VIPSpacePoint != NULL) { delete m_VIPSpacePoint; }
      for (auto* aSector : m_activeSectors) { delete aSector; }
      for (auto* aSegment : m_segments) { delete aSegment; }
      for (auto* aNode : m_trackNodes) { delete aNode; }
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
    /// Getters
    /** Returns reference to the ActiveSectorNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::ActiveSector<StaticSectorType, Belle2::TrackNode>, Belle2::VoidMetaInfo>&
    accessActiveSectorNetwork() { return m_ActiveSectorNetwork; }


    /** Returns reference to the actual ActiveSectors stored in this container, intended for read and write access */
    std::vector<Belle2::ActiveSector<StaticSectorType, Belle2::TrackNode>* >& accessActiveSectors() { return m_activeSectors; }

    /** Returns reference to the HitNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::TrackNode, Belle2::VoidMetaInfo>& accessHitNetwork() { return m_HitNetwork; }

    /** Returns reference to the SegmentNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >& accessSegmentNetwork() { return m_SegmentNetwork; }

    /** Returns reference to the actual segments stored in this container, intended for read and write access */
    std::vector<Belle2::Segment<Belle2::TrackNode>* >& accessSegments() { return m_segments; }

    /** Returns reference to the actual trackNodes stored in this container, intended for read and write access */
    std::vector<Belle2::TrackNode* >& accessTrackNodes() { return m_trackNodes; }


    int sizeSegments() { return m_segments.size(); }
    int sizeActiveSectors() { return m_activeSectors.size(); }
    int sizeTrackNodes() { return m_trackNodes.size(); }

    int get_trackNodeConnections() { return m_trackNodeConnections; }
    int get_activeSectorConnections() { return m_activeSectorConnections; }
    int get_segmentConnections() { return m_segmentConnections; }
    int get_collectedPaths() { return m_collectedPaths; }

    void set_trackNodeConnections(int in) { m_trackNodeConnections = in; }
    void set_activeSectorConnections(int in) { m_activeSectorConnections = in; }
    void set_segmentConnections(int in) { m_segmentConnections = in; }
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
    int m_trackNodeConnections = 0;
    int m_activeSectorConnections = 0;
    int m_segmentConnections = 0;
    int m_collectedPaths = 0;

    /** ************************* DATA MEMBERS ************************* */
    /** Stores the full network of activeSectors, which contain hits in that event and have compatible Sectors with hits too*/
    DirectedNodeNetwork<ActiveSector<StaticSectorType, TrackNode>, Belle2::VoidMetaInfo> m_ActiveSectorNetwork;//!

    /** Stores the actual ActiveSectors, since the ActiveSectorNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<ActiveSector<StaticSectorType, TrackNode>* > m_activeSectors;//!

    /** Stores the full network of TrackNode< SpaacePoint>, which were accepted by activated two-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<TrackNode, Belle2::VoidMetaInfo> m_HitNetwork;//!

    /** Stores the full network of Segments, which were accepted by activated three-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<Segment<TrackNode>, Belle2::CACell> m_SegmentNetwork;//!

    /** Stores the actual Segments, since the SegmentNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<Segment<TrackNode>* > m_segments;//!

    /** Stores the actual trackNodes, since the SegmentNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<TrackNode* > m_trackNodes;//!

    /** Stores a SpacePoint representing the virtual interaction point if set, NULL if not. */
    Belle2::TrackNode* m_VirtualInteractionPoint;//!

    /** Stores the SpacePoint needed for the virtual IP */
    SpacePoint* m_VIPSpacePoint;//!
  };
}
