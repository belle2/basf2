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
    SpacePoint* spacePoint;

    /** unique integer identifier */
    int m_identifier;

    /** longer name for debugging */
    std::string m_name;

    /** overloaded '=='-operator
     * TODO JKL: pretty ugly operator overload, should be fixed ASAP! (solution for null-ptr-issue needed)
     * WARNING TODO write a test for that one!
    * TODO find good reasons why one would like to create TrackNodes without Hits and ActiveSectors linked to them! */
    bool operator==(const TrackNode& b) const
    {
      // simple case: no null-ptrs interfering:
      if (spacePoint != nullptr and b.spacePoint != nullptr and sector != nullptr and b.sector != nullptr) {
        // compares objects:
        return (*spacePoint == *(b.spacePoint)) and (*sector == *(b.sector));
      }

      // case: at least one of the 2 nodes has no null-ptrs:
      if (spacePoint != nullptr and sector != nullptr) return false; // means: this Node has no null-Ptrs -> the other one has
      if (b.spacePoint != nullptr and b.sector != nullptr) return false; // means: the other Node has no null-Ptrs -> this one has

      // case: both nodes have got at least one null-ptr:
      bool spacePointsAreEqual = false;
      if (spacePoint != nullptr and b.spacePoint != nullptr) {
        spacePointsAreEqual = (*spacePoint == *(b.spacePoint));
      } else {
        spacePointsAreEqual = (spacePoint == b.spacePoint);
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
      if (spacePoint == nullptr) B2FATAL("TrackNode::operator !=: spacePoint for Tracknode not set - aborting run.");
      return !(*this == b);
    }


    /** returns reference to hit. */
    const SpacePoint& getHit() const
    {
      if (spacePoint == nullptr) B2FATAL("TrackNode::getHit: spacePoint for Tracknode not set - aborting run.");
      return *spacePoint;
    }

    /** returns reference to hit. */
    ActiveSector<StaticSectorType, TrackNode>& getActiveSector()
    {
      if (sector == nullptr) B2FATAL("TrackNode::getActiveSector: ActiveSector for Tracknode not set - aborting run.");
      return *sector;
    }

    /** constructor WARNING: sector-pointing has still to be decided! */
    TrackNode() : sector(nullptr), spacePoint(nullptr), m_identifier(-1), m_name("SP: missing") {}

    TrackNode(SpacePoint* new_spacePoint) :      // Get unique identifier from SP ArrayIndex, Get long debugging name from SP
      sector(nullptr), spacePoint(new_spacePoint)
    {
      m_identifier = new_spacePoint->getArrayIndex();
      m_name = "SP: " + new_spacePoint->getName();
    }

    /** destructor */
    ~TrackNode() {}

    /** return ID of this node */
    int getID() { return m_identifier; }

    /** returns longer debugging name of this node */
    const std::string& getName() const { return m_name; }

  };

} //Belle2 namespace
