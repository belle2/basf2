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
  /** Minimal class to store combination of sector and spacePoint, since SpacePoint can not carry sectorConnection */
  struct TrackNode {
    /** To improve readability of the code, here the definition of the static sector type. */
    using StaticSectorType = VXDTFFilters<SpacePoint>::staticSector_t;

    /** Constructor */
    TrackNode() : m_sector(nullptr), m_spacePoint(nullptr), m_identifier(-1) {}

    /** Constructor with information from SpacePoint */
    TrackNode(SpacePoint* spacePoint) :
      m_sector(nullptr), m_spacePoint(spacePoint), m_identifier(spacePoint->getArrayIndex())
    {}

    /** Destructor */
    ~TrackNode() {}


    /** Pointer to sector */
    ActiveSector<StaticSectorType, TrackNode>* m_sector;

    /** Pointer to spacePoint */
    SpacePoint* m_spacePoint;

    /** Unique integer identifier */
    const std::int32_t m_identifier;


    /** Overloaded '=='-operator
     * TODO JKL: pretty ugly operator overload, should be fixed ASAP! (solution for null-ptr-issue needed)
     * TODO write a test for that one!
     * TODO find good reasons why one would like to create TrackNodes without Hits and ActiveSectors linked to them! */
    bool operator==(const TrackNode& b) const
    {
      // simple case: no null-ptrs interfering:
      if (m_spacePoint != nullptr and b.m_spacePoint != nullptr and m_sector != nullptr and b.m_sector != nullptr) {
        // compares objects:
        return (*m_spacePoint == *(b.m_spacePoint)) and (*m_sector == *(b.m_sector));
      }

      // case: at least one of the 2 nodes has no null-ptrs:
      if (m_spacePoint != nullptr and m_sector != nullptr) return false; // means: this Node has no null-Ptrs -> the other one has
      if (b.m_spacePoint != nullptr and b.m_sector != nullptr) return false; // means: the other Node has no null-Ptrs -> this one has

      // case: both nodes have got at least one null-ptr:
      bool spacePointsAreEqual = false;
      if (m_spacePoint != nullptr and b.m_spacePoint != nullptr) {
        spacePointsAreEqual = (*m_spacePoint == *(b.m_spacePoint));
      } else {
        spacePointsAreEqual = (m_spacePoint == b.m_spacePoint);
      }
      bool sectorsAreEqual = false;
      if (m_sector != nullptr and b.m_sector != nullptr) {
        sectorsAreEqual = (*m_sector == *(b.m_sector));
      } else {
        sectorsAreEqual = (m_sector == b.m_sector);
      }
      return (spacePointsAreEqual == true and sectorsAreEqual == true);
    }


    /** Overloaded '!='-operator */
    bool operator!=(const TrackNode& b) const
    {
      if (m_spacePoint == nullptr) {
        B2FATAL("TrackNode::operator !=: m_spacePoint for Tracknode not set - aborting run.");
      }
      return !(*this == b);
    }


    /** returns reference to hit. */
    const SpacePoint& getHit() const
    {
      if (m_spacePoint == nullptr) {
        B2FATAL("TrackNode::getHit: m_spacePoint for Tracknode not set - aborting run.");
      }
      return *m_spacePoint;
    }


    /** returns reference to hit. */
    ActiveSector<StaticSectorType, TrackNode>& getActiveSector()
    {
      if (m_sector == nullptr) {
        B2FATAL("TrackNode::getActiveSector: ActiveSector for Tracknode not set - aborting run.");
      }
      return *m_sector;
    }


    /** Return ID of this node */
    std::int32_t getID() const { return m_identifier; }


    /** Returns longer debugging name of this node */
    std::string getName() const
    {
      if (m_identifier >= 0)
        return "SP: " + m_spacePoint->getName();
      else
        return "SP: missing";
    }
  };
}
