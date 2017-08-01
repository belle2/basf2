/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/pxdSpacePoint/PXDSpacePointMatcher.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void PXDSpacePointMatcher::initializeEventCache(std::vector<RecoTrack*>& seedsVector,
                                                std::vector<const SpacePoint*>& filteredHitVector)
{
  m_cachedHitMapOnLadder.clear();
  m_cachedHitMapOnLayer.clear();

  const auto& hitSorterByID = [](const SpacePoint * lhs, const SpacePoint * rhs) {
    return lhs->getVxdID() < rhs->getVxdID();
  };

  std::sort(filteredHitVector.begin(), filteredHitVector.end(), hitSorterByID);

  const auto& getLayerAndLadder = [](const SpacePoint * lhs) {
    const auto& lhsID = lhs->getVxdID();
    return std::make_pair(lhsID.getLayerNumber(), lhsID.getLadderNumber());
  };

  const auto& getLayer = [](const SpacePoint * lhs) {
    const auto& lhsID = lhs->getVxdID();
    return lhsID.getLayerNumber();
  };

  const auto& groupedByLayerAndLadder = adjacent_groupby(filteredHitVector.begin(), filteredHitVector.end(), getLayerAndLadder);
  for (const auto& group : groupedByLayerAndLadder) {
    const auto& commonLayerAndLadder = getLayerAndLadder(group.front());
    m_cachedHitMapOnLadder.emplace(commonLayerAndLadder, group);
  }

  const auto& groupedByLayer = adjacent_groupby(filteredHitVector.begin(), filteredHitVector.end(), getLayer);
  for (const auto& group : groupedByLayer) {
    const auto& commonLayer = getLayer(group.front());
    m_cachedHitMapOnLayer.emplace(commonLayer, group);
  }
}
