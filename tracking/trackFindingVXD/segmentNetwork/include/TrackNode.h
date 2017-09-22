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

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/segmentNetwork/StaticSector.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <tracking/trackFindingVXD/segmentNetwork/ActiveSector.h>

#include <string>

namespace Belle2 {


  /** minimal class to store combination of sector and spacePoint, since SpacePoint can not carry sectorConnection */
  struct TrackNode {

    /** to improve readability of the code, here the definition of the static sector type. */
    using StaticSectorType = VXDTFFilters<SpacePoint>::staticSector_t;

    /** pointer to sector */
    ActiveSector<StaticSectorType, TrackNode>* sector;

    /** pointer to spacePoint */
    SpacePoint* m_spacePoint;

    /** unique integer identifier */
    int m_identifier;

    /** overloaded '=='-operator
     * TODO JKL: pretty ugly operator overload, should be fixed ASAP! (solution for null-ptr-issue needed)
     * WARNING TODO write a test for that one!
    * TODO find good reasons why one would like to create TrackNodes without Hits and ActiveSectors linked to them! */
    bool operator==(const TrackNode& b) const
    {
      // simple case: no null-ptrs interfering:
      if (m_spacePoint != nullptr and b.m_spacePoint != nullptr and sector != nullptr and b.sector != nullptr) {
        // compares objects:
        return (*m_spacePoint == *(b.m_spacePoint)) and (*sector == *(b.sector));
      }

      // case: at least one of the 2 nodes has no null-ptrs:
      if (m_spacePoint != nullptr and sector != nullptr) return false; // means: this Node has no null-Ptrs -> the other one has
      if (b.m_spacePoint != nullptr and b.sector != nullptr) return false; // means: the other Node has no null-Ptrs -> this one has

      // case: both nodes have got at least one null-ptr:
      bool spacePointsAreEqual = false;
      if (m_spacePoint != nullptr and b.m_spacePoint != nullptr) {
        spacePointsAreEqual = (*m_spacePoint == *(b.m_spacePoint));
      } else {
        spacePointsAreEqual = (m_spacePoint == b.m_spacePoint);
      }
      bool sectorsAreEqual = false;
      if (sector != nullptr and b.sector != nullptr) {
        sectorsAreEqual = (*sector == *(b.sector));
      } else {
        sectorsAreEqual = (sector == b.sector);
      }
      return (spacePointsAreEqual == true and sectorsAreEqual == true);
    }


    /** overloaded '!='-operator */
    bool operator!=(const TrackNode& b) const
    {
      if (m_spacePoint == nullptr) B2FATAL("TrackNode::operator !=: spacePoint for Tracknode not set - aborting run.");
      return !(*this == b);
    }


    /** returns reference to hit. */
    const SpacePoint& getHit() const
    {
      if (m_spacePoint == nullptr) B2FATAL("TrackNode::getHit: spacePoint for Tracknode not set - aborting run.");
      return *m_spacePoint;
    }

    /** returns reference to hit. */
    ActiveSector<StaticSectorType, TrackNode>& getActiveSector()
    {
      if (sector == nullptr) B2FATAL("TrackNode::getActiveSector: ActiveSector for Tracknode not set - aborting run.");
      return *sector;
    }

    /** constructor WARNING: sector-pointing has still to be decided! */
    TrackNode() : sector(nullptr), m_spacePoint(nullptr), m_identifier(-1) {}

    TrackNode(SpacePoint* spacePoint) :      // Get unique identifier from SP ArrayIndex
      sector(nullptr), m_spacePoint(spacePoint)
    {
      m_identifier = m_spacePoint->getArrayIndex();
    }

    /** destructor */
    ~TrackNode() {}

    /** return ID of this node */
    int getID() const { return m_identifier; }

    /** returns longer debugging name of this node */
    std::string getName() const
    {
      if (m_identifier >= 0)
        return "SP: " + m_spacePoint->getName();
      else
        return "SP: missing";
    }

  };

} //Belle2 namespace
