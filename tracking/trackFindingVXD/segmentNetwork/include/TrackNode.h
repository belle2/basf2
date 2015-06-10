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
#include <tracking/trackFindingVXD/segmentNetwork/StaticSectorDummy.h>
#include <tracking/trackFindingVXD/segmentNetwork/ActiveSector.h>



namespace Belle2 {


  /** minimal class to store combination of sector and spacePoint, since SpacePoint can not carry sectorConnection */
  struct TrackNode {
    /** pointer to sector */
    ActiveSector<StaticSectorDummy, TrackNode>* sector;

    /** pointer to spacePoint */
    SpacePoint* spacePoint;

    /** overloaded '=='-operator
     * TODO JKL: pretty ugly operator overload, should be fixed ASAP! (solution for null-ptr-issue needed)
     * WARNING TODO write a test for that one! */
    bool operator==(const TrackNode& b) const
    {
      // simple case: no null-ptrs interfering:
      if (spacePoint != NULL and b.spacePoint != NULL and sector != NULL and b.sector != NULL) {
        // compares objects:
        return (*spacePoint == *(b.spacePoint)) and (*sector == *(b.sector));
      }

      // case: at least one of the 2 nodes has no null-ptrs:
      if (spacePoint != NULL and sector != NULL) return false; // means: this Node has no null-Ptrs -> the other one has
      if (b.spacePoint != NULL and b.sector != NULL) return false; // means: the other Node has no null-Ptrs -> this one has

      // case: both nodes have got at least one null-ptr:
      bool spacePointsAreEqual = false;
      if (spacePoint != NULL and b.spacePoint != NULL) {
        spacePointsAreEqual = (*spacePoint == *(b.spacePoint));
      } else {
        spacePointsAreEqual = (spacePoint == b.spacePoint);
      }
      bool sectorsAreEqual = false;
      if (sector != NULL and b.sector != NULL) {
        sectorsAreEqual = (*sector == *(b.sector));
      } else {
        sectorsAreEqual = (sector == b.sector);
      }
      return (spacePointsAreEqual == true and sectorsAreEqual == true);
    }

    /** overloaded '!='-operator */
    bool operator!=(const TrackNode& b) const { return !(*this == b); }

    /** constructor WARNING: sector-pointing has still to be decided! */
    TrackNode() : sector(NULL), spacePoint(NULL) {}

    /** destructor */
    ~TrackNode() {}
  };

} //Belle2 namespace