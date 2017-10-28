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
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <vector>

namespace Belle2 {

  template<class NetworkPath>
  inline SpacePointTrackCand convertNetworkPath(NetworkPath networkPath)
  {
    std::vector <const SpacePoint*> spVector;
    spVector.reserve(networkPath.size());
    if (networkPath.empty()) {
      return SpacePointTrackCand();
    }

    auto family = networkPath.at(0).getFamily();
    for (auto aNodeIt = networkPath.rbegin(); aNodeIt != networkPath.rend();  ++aNodeIt) {
      insertSpacePoints(spVector, *aNodeIt);
    }

    auto sptc = SpacePointTrackCand(spVector);
    sptc.setFamily(family);
    return sptc;
  }

  inline void insertSpacePoints(std::vector<const SpacePoint*>& target, Segment<TrackNode> source)
  {
    if (target.empty()) {
      insertSpacePoints(target, *(source.getInnerHit()));
      insertSpacePoints(target, *(source.getOuterHit()));
    } else {
      insertSpacePoints(target, *(source.getOuterHit()));
    }
  }

  inline void insertSpacePoints(std::vector<const SpacePoint*>& target, TrackNode source)
  {
    target.push_back(source.spacePoint);
  }

}
