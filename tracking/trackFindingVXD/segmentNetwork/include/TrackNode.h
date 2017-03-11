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

    std::string m_ID;

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


    /** overloaded '<<' stream operator. Print secID to stream by converting it to string */
    friend std::ostream& operator<< (std::ostream& out, const TrackNode& aNode)
    {
      out << aNode.getName();
      return out;
    }


    /** returns reference to hit. */
    const SpacePoint& getHit() const
    {
      if (spacePoint == nullptr) B2FATAL("TrackNode::getHit: spacePoint for Tracknode not set - aborting run.");
      return *spacePoint;
    }

    std::string getID() const
    {
      return m_ID;
    }


//  /** returns pointer to hit of the tracknode if set, returns nullptr if not. */
//  const SpacePoint* getHit() const
//  {
//    if (spacePoint == nullptr) B2WARNING("TrackNode::getHit: spacePoint for Tracknode not set - returning nullptr instead!")
//    return spacePoint;
//  }

//  /** returns pointer to activeSector of the tracknode if set, returns nullptr if not. */
//  ActiveSector<StaticSectorType, TrackNode>* getActiveSector()
//  {
//    if (sector == nullptr) B2WARNING("TrackNode::getActiveSector: ActiveSector for Tracknode not set - returning nullptr instead!")
//    return sector;
//  }

    /** returns reference to hit. */
    ActiveSector<StaticSectorType, TrackNode>& getActiveSector()
    {
      if (sector == nullptr) B2FATAL("TrackNode::getActiveSector: ActiveSector for Tracknode not set - aborting run.");
      return *sector;
    }


    /** constructor WARNING: sector-pointing has still to be decided! */
    TrackNode(short index) : sector(nullptr), spacePoint(nullptr), m_ID(std::to_string(index)) {}


    /** destructor */
    ~TrackNode() {}


    /** returns secID of this sector */
    std::string getName() const
    {
      std::string out;
      if (sector != nullptr) { out += "Sec: " + sector->getName(); } else {out += "no sector attached "; }
      if (spacePoint != nullptr) { out += ", SP: " + spacePoint->getName(); } else {out += "no hit attached"; }
      return out;
    }
  };

} //Belle2 namespace
