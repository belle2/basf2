/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vector>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/segmentNetwork/Segment.h>
#include <tracking/trackFindingVXD/segmentNetwork/TrackNode.h>

namespace Belle2 {

  /** Create new SPTC from network path. */
  template<class NetworkPath>
  inline SpacePointTrackCand convertNetworkPath(NetworkPath networkPath)
  {
    std::vector <const SpacePoint*> spVector;
    spVector.reserve(networkPath.size());
    if (networkPath.empty()) {
      return SpacePointTrackCand();
    }

    auto family = networkPath[0]->getFamily();
    for (auto aNodeIt = networkPath.rbegin(); aNodeIt != networkPath.rend();  ++aNodeIt) {
      insertSpacePoints(spVector, (*aNodeIt)->getEntry());
    }

    auto sptc = SpacePointTrackCand(spVector);
    sptc.setFamily(family);
    return sptc;
  }


  /// Convert TrackNode to SpaePoint an add to a SpacePoint path.
  inline void insertSpacePoint(std::vector<const SpacePoint*>& target, TrackNode source)
  {
    target.push_back(source.m_spacePoint);
  }


  /// Insert of inner and outer TrackNodes of a Segment as SpacePoints into path of SpacePoints
  inline void insertSpacePoints(std::vector<const SpacePoint*>& target, const Segment<TrackNode>& source)
  {
    if (target.empty()) {
      insertSpacePoint(target, *(source.getInnerHit()));
      insertSpacePoint(target, *(source.getOuterHit()));
    } else {
      insertSpacePoint(target, *(source.getOuterHit()));
    }
  }
}
