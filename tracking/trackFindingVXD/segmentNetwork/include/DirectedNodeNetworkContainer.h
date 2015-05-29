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

    /** forward declaration since we have no Static sectorMap at the moment */
    class SectorMapDummy {};

    /** Stores the full network of activeSectors, which contain hits in that event and have compatible Sectors with hits too*/
    DirectedNodeNetwork<ActiveSector<SectorMapDummy, SpacePoint> > m_ActiveSectorNetwork;

    /** Stores the full network of SpacePoints, which were accepted by activated two-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<SpacePoint> m_SpacePointNetwork;

    /** Stores the full network of Segments, which were accepted by activated three-hit-filters of the assigned sectorMap */
    DirectedNodeNetwork<Segment<SpacePoint> > m_SegmentNetwork;

    /** stores a SpacePoint representing the virtual interaction point if set, NULL if not. */
    SpacePoint* m_VirtualInteractionPoint;


  public:
    /** ************************* CONSTRUCTORS ************************* */

    /** standard constructor */
    DirectedNodeNetworkContainer() :
      m_ActiveSectorNetwork(DirectedNodeNetwork<ActiveSector<SectorMapDummy, SpacePoint> >()),
      m_SpacePointNetwork(DirectedNodeNetwork<SpacePoint>()),
      m_SegmentNetwork(DirectedNodeNetwork<Segment<SpacePoint> >()),
      m_VirtualInteractionPoint(NULL) {}


    /** destructor */
    ~DirectedNodeNetworkContainer()
    { if (m_VirtualInteractionPoint != NULL) { delete m_VirtualInteractionPoint; } }


    /** ************************* PUBLIC MEMBER FUNCTIONS ************************* */
/// getters

    /** returns reference to the ActiveSectorNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::ActiveSector<Belle2::DirectedNodeNetworkContainer::SectorMapDummy, Belle2::SpacePoint> >&
    accessActiveSectorNetwork() { return m_ActiveSectorNetwork; }


    /** returns reference to the SpacePointNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::SpacePoint>& accessSpacePointNetwork() { return m_SpacePointNetwork; }


    /** returns reference to the SegmentNetwork stored in this container, intended for read and write access */
    DirectedNodeNetwork<Belle2::Segment<Belle2::SpacePoint> >& accessSegmentNetwork() { return m_SegmentNetwork; }


    /** passes parameters for creating a virtual interaction point */
    void setVirtualInteractionPoint() { m_VirtualInteractionPoint = new SpacePoint(); }


    ClassDef(DirectedNodeNetworkContainer,
             5) // last member changed: added DirectedNodeNetworks for ActiveSectors, SpacePoints and Segments, a link to the virtual IP too...
  };

} //Belle2 namespace