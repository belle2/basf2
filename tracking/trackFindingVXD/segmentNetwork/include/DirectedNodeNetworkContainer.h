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

#include <tracking/trackFindingVXD/segmentNetwork/StaticSectorDummy.h>

#include <tracking/trackFindingVXD/segmentNetwork/Segment.h>
#include <tracking/trackFindingVXD/segmentNetwork/ActiveSector.h>

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
  class DirectedNodeNetworkContainer : public RelationsObject {
  protected:
    /** ************************* DATA MEMBERS ************************* */



    /** Stores the full network of activeSectors, which contain hits in that event and have compatible Sectors with hits too*/
    DirectedNodeNetwork<ActiveSector<StaticSectorDummy, SpacePoint> > m_ActiveSectorNetwork;

    /** stores the actual ActiveSectors, since the ActiveSectorNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<ActiveSector<StaticSectorDummy, SpacePoint>* > m_activeSectors;

    /** Stores the full network of SpacePoints, which were accepted by activated two-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<SpacePoint> m_SpacePointNetwork;

    /** Stores the full network of Segments, which were accepted by activated three-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<Segment<SpacePoint> > m_SegmentNetwork;

    /** stores the actual Segments, since the SegmentNetwork does only keep references - TODO switch to unique pointers! */
    std::vector<Segment<SpacePoint>* > m_segments;

    /** stores a SpacePoint representing the virtual interaction point if set, NULL if not. */
    SpacePoint* m_VirtualInteractionPoint;


  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** standard constructor */
    DirectedNodeNetworkContainer() :
      m_ActiveSectorNetwork(DirectedNodeNetwork<ActiveSector<Belle2::StaticSectorDummy, Belle2::SpacePoint> >()),
      m_SpacePointNetwork(DirectedNodeNetwork<Belle2::SpacePoint>()),
      m_SegmentNetwork(DirectedNodeNetwork<Belle2::Segment<Belle2::SpacePoint> >()),
      m_VirtualInteractionPoint(NULL) {}


    /** destructor */
    ~DirectedNodeNetworkContainer()
    {
      if (m_VirtualInteractionPoint != NULL) { delete m_VirtualInteractionPoint; }
      for (auto* aSector : m_activeSectors) { delete aSector; }
      for (auto* aSegment : m_segments) { delete aSegment; }
    }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
/// getters

    /** returns reference to the ActiveSectorNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::ActiveSector<Belle2::StaticSectorDummy, Belle2::SpacePoint> >&
    accessActiveSectorNetwork() { return m_ActiveSectorNetwork; }


    /** returns reference to the actual ActiveSectors stored in this container, intended for read and write access */
    std::vector<Belle2::ActiveSector<Belle2::StaticSectorDummy, Belle2::SpacePoint>* >& accessActiveSectors() { return m_activeSectors; }


    /** returns reference to the SpacePointNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::SpacePoint>& accessSpacePointNetwork() { return m_SpacePointNetwork; }


    /** returns reference to the SegmentNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::Segment<Belle2::SpacePoint> >& accessSegmentNetwork() { return m_SegmentNetwork; }


    /** returns reference to the actual segments stored in this container, intended for read and write access */
    std::vector<Belle2::Segment<Belle2::SpacePoint>* >& accessSegments() { return m_segments; }


    /** passes parameters for creating a virtual interaction point TODO pass coordinates-parameter (and all the other stuff needed for a proper vIP. */
    void setVirtualInteractionPoint() { m_VirtualInteractionPoint = new SpacePoint(); }


    /** returns reference to the Virtual interactionPoint stored here */
    SpacePoint* getVirtualInteractionPoint() { return m_VirtualInteractionPoint; }


    // last member changed: replaced internal SectorDummy with more sophisticated mockup 'StaticSectorDummy'
    ClassDef(DirectedNodeNetworkContainer, 6)
  };

} //Belle2 namespace