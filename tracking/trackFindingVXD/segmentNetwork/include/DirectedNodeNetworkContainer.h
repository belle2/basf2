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

#include <framework/datastore/RelationsObject.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>

#include <tracking/trackFindingVXD/segmentNetwork/StaticSector.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h> // needed for the correct typedef of the StaticSector
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/Segment.h>
#include <tracking/trackFindingVXD/segmentNetwork/ActiveSector.h>
#include <tracking/trackFindingVXD/segmentNetwork/VoidMetaInfo.h>
#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>


// C++-std:
#include <vector>


namespace Belle2 {


  /** The Container stores the output produced by the SegmentNetworkProducerModule.
   *
   * Is coupled to an assigned static sectorMap, which influences the structure of the DirectedNodeNetworks stored in this container.
   * is intended to be filled event-wise and not to be stored in root-files.
   *
   * TODO: create constructor for vIPs in SpacePoint. What about activeSectors for vIP? -> solution dependent of treatment in static sectorMap.
   */

  // WARNING: all data members which are followed by "//!" will not be streamed by root (so no I/O for them)
  class DirectedNodeNetworkContainer : public RelationsObject {
  public:
    /** to improve readability of the code, here the definition of the static sector type. */
    using StaticSectorType = VXDTFFilters<SpacePoint>::staticSector_t;
  protected:
    /** ************************* DATA MEMBERS ************************* */


    /** Stores the full network of activeSectors, which contain hits in that event and have compatible Sectors with hits too*/
    DirectedNodeNetwork<ActiveSector<StaticSectorType, TrackNode>, Belle2::VoidMetaInfo > m_ActiveSectorNetwork;//!

    /** stores the actual ActiveSectors, since the ActiveSectorNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<ActiveSector<StaticSectorType, TrackNode>* > m_activeSectors;//!

    /** Stores the full network of TrackNode< SpaacePoint>, which were accepted by activated two-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<TrackNode, Belle2::VoidMetaInfo> m_HitNetwork;//!

    /** Stores the full network of Segments, which were accepted by activated three-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<Segment<TrackNode>, Belle2::CACell > m_SegmentNetwork;//!

    /** stores the actual Segments, since the SegmentNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<Segment<TrackNode>* > m_segments;//!

    /** stores the actual trackNodes, since the SegmentNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<TrackNode* > m_trackNodes;//!

    /** stores a SpacePoint representing the virtual interaction point if set, NULL if not. */
    Belle2::TrackNode* m_VirtualInteractionPoint;//!

    /** stores the SpacePoint needed for the virtual IP */
    SpacePoint* m_VIPSpacePoint;//!

  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** standard constructor */
    DirectedNodeNetworkContainer() :
      m_ActiveSectorNetwork(DirectedNodeNetwork<ActiveSector<StaticSectorType, Belle2::TrackNode>, Belle2::VoidMetaInfo >()),
      m_HitNetwork(DirectedNodeNetwork<Belle2::TrackNode, Belle2::VoidMetaInfo>()),
      m_SegmentNetwork(DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >()),
      m_VirtualInteractionPoint(NULL),
      m_VIPSpacePoint(NULL) {}


    /** destructor */
    ~DirectedNodeNetworkContainer()
    {
      if (m_VirtualInteractionPoint != NULL) { delete m_VirtualInteractionPoint; }
      if (m_VIPSpacePoint != NULL) { delete m_VIPSpacePoint; }
      for (auto* aSector : m_activeSectors) { delete aSector; }
      for (auto* aSegment : m_segments) { delete aSegment; }
      for (auto* aNode : m_trackNodes) { delete aNode; }
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
/// getters

    /** returns reference to the ActiveSectorNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::ActiveSector<StaticSectorType, Belle2::TrackNode>, Belle2::VoidMetaInfo >&
    accessActiveSectorNetwork() { return m_ActiveSectorNetwork; }


    /** returns reference to the actual ActiveSectors stored in this container, intended for read and write access */
    std::vector<Belle2::ActiveSector<StaticSectorType, Belle2::TrackNode>* >& accessActiveSectors() { return m_activeSectors; }


    /** returns reference to the HitNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::TrackNode, Belle2::VoidMetaInfo>& accessHitNetwork() { return m_HitNetwork; }


    /** returns reference to the SegmentNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::Segment<Belle2::TrackNode>, Belle2::CACell >& accessSegmentNetwork() { return m_SegmentNetwork; }


    /** returns reference to the actual segments stored in this container, intended for read and write access */
    std::vector<Belle2::Segment<Belle2::TrackNode>* >& accessSegments() { return m_segments; }


    /** returns reference to the actual trackNodes stored in this container, intended for read and write access */
    std::vector<Belle2::TrackNode* >& accessTrackNodes() { return m_trackNodes; }

    /// Clear segment network
    void clear()
    {
      m_activeSectors.clear();
      m_segments.clear();
      m_trackNodes.clear();
      m_SegmentNetwork.clear();
    }

    /** passes parameters for creating a virtual interaction point */
    void setVirtualInteractionPoint(B2Vector3D& pos, B2Vector3D& posError)
    {
      m_VIPSpacePoint = new SpacePoint(pos, posError, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
      m_VirtualInteractionPoint = new Belle2::TrackNode(m_VIPSpacePoint);
    }


    /** returns reference to the Virtual interactionPoint stored here */
    Belle2::TrackNode* getVirtualInteractionPoint() { return m_VirtualInteractionPoint; }


    // last member changed: added metaInfo for DirectedNodeNetwork
    ClassDef(DirectedNodeNetworkContainer, 10)
  };

} //Belle2 namespace
