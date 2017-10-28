/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    spVector.reserve(networkPath->size());
    if (networkPath->empty()) {
      return SpacePointTrackCand();
    }

    auto family = networkPath->at(0)->getFamily();
    for (auto aNodeIt = networkPath->rbegin(); aNodeIt != networkPath->rend();  ++aNodeIt) {
      insertSpacePoints(spVector, (*aNodeIt)->getEntry());
    }

    auto sptc = SpacePointTrackCand(spVector);
    sptc.setFamily(family);
    return sptc;
  }

  inline void insertSpacePoint(std::vector<const SpacePoint*>& target, TrackNode source)
  {
    target.push_back(source.m_spacePoint);
  }


  inline void insertSpacePoints(std::vector<const SpacePoint*>& target, Segment<TrackNode> source)
  {
    if (target.empty()) {
      insertSpacePoint(target, *(source.getInnerHit()));
      insertSpacePoint(target, *(source.getOuterHit()));
    } else {
      insertSpacePoint(target, *(source.getOuterHit()));
    }
  }

}
