/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/cdcToSpacePoint/CDCToSpacePointMatcher.h>

using namespace Belle2;
using namespace TrackFindingCDC;

constexpr unsigned int CDCToSpacePointMatcher::maximumLadderNumbers[6];

void CDCToSpacePointMatcher::initializeEventCache(std::vector<RecoTrack*>& seedsVector,
                                                  std::vector<const SpacePoint*>& filteredHitVector)
{
  m_cachedHitMap.clear();

  const auto& hitSorterByID = [](const SpacePoint * lhs, const SpacePoint * rhs) {
    return lhs->getVxdID() < rhs->getVxdID();
  };

  std::sort(filteredHitVector.begin(), filteredHitVector.end(), hitSorterByID);

  for (unsigned int layerID = 1; layerID <= 6; ++layerID) {
    const auto& onGivenLayerCheck = [layerID](const SpacePoint * spacePoint) {
      return spacePoint->getVxdID().getLayerNumber() == layerID;
    };

    const auto first = std::find_if(filteredHitVector.begin(), filteredHitVector.end(), onGivenLayerCheck);
    const auto last = std::find_if_not(first, filteredHitVector.end(), onGivenLayerCheck);

    m_cachedHitMap.emplace(layerID, VectorRange<const SpacePoint*>(first, last));

    B2DEBUG(100, "Storing in " << layerID << " " << std::distance(first, last));
    for (auto it = first; it != last; it++) {
      B2DEBUG(100, (*it)->getVxdID());
    }
  }
}
