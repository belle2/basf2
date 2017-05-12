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

SortedVectorRange<const SpacePoint*> CDCToSpacePointMatcher::getMatchingHits(CKFCDCToVXDStateObject& currentState)
{
  const unsigned int currentNumber = currentState.getNumber();

  if (currentNumber == CKFCDCToVXDStateObject::N or currentState.isOnOverlapLayer()) {
    // next layer is not an overlap one, so we can just return all hits of this layer.
    const unsigned int nextLayer = currentState.extractGeometryLayer() - 1;
    return m_cachedHitMap[nextLayer];
  } else {
    // next layer is an overlap one, so lets return all hits from the same layer, that are on a
    // ladder which is one below the last added hit.
    const SpacePoint* lastAddedSpacePoint = currentState.getHit();
    if (not lastAddedSpacePoint) {
      // No hit was added on the layer, so no overlap can occur.
      return SortedVectorRange<const SpacePoint*>();
    }

    const unsigned int ladderNumber = lastAddedSpacePoint->getVxdID().getLadderNumber();
    const unsigned int currentLayer = currentState.extractGeometryLayer();
    const unsigned int maximumLadderNumber = m_maximumLadderNumbers[currentLayer];

    // the reason for this strange formula is the numbering scheme in the VXD.
    // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
    // then we subtract one to get to the next (overlapping) ladder and % N, to also cope for the
    // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
    const unsigned int overlappingLadder = ((ladderNumber - 1) - 1) % maximumLadderNumber + 1;

    B2DEBUG(100, "Overlap check on " << ladderNumber << " using from " << overlappingLadder);

    const auto& onNextLadderCheck = [overlappingLadder](const SpacePoint * spacePoint) {

      return spacePoint->getVxdID().getLadderNumber() == overlappingLadder;
    };

    const auto& hitsOfCurrentLayer = m_cachedHitMap[currentLayer];
    const auto first = std::find_if(hitsOfCurrentLayer.begin(), hitsOfCurrentLayer.end(), onNextLadderCheck);
    const auto last = std::find_if_not(first, hitsOfCurrentLayer.end(), onNextLadderCheck);

    B2DEBUG(100, "Overlap " << currentLayer << " " << lastAddedSpacePoint->getVxdID() <<  " " << std::distance(first, last));
    for (auto it = first; it != last; it++) {
      B2DEBUG(100, (*it)->getVxdID());
    }
    return SortedVectorRange<const SpacePoint*>(first, last);
  }

}

/*void CDCToSpacePointSelector::initializeEventCache(std::vector<RecoTrack*>& seedsVector,
                                                            std::vector<const SpacePoint*>& filteredHitVector)
{
  m_cachedHitMap.clear();

  const auto& hitSorterByID = [](const SpacePoint * lhs, const SpacePoint * rhs) {
    return lhs->getVxdID() < rhs->getVxdID();
  };

  std::sort(filteredHitVector.begin(), filteredHitVector.end(), hitSorterByID);

  for (unsigned int layerID = 3; layerID <= 6; ++layerID) {
    const auto& onGivenLayerCheck = [layerID](const SpacePoint * spacePoint) {
      return spacePoint->getVxdID().getLayerNumber() == layerID;
    };

    const auto first = std::find_if(filteredHitVector.begin(), filteredHitVector.end(), onGivenLayerCheck);
    const auto last = std::find_if_not(first, filteredHitVector.end(), onGivenLayerCheck);

    m_cachedHitMap.emplace(layerID, SortedVectorRange<const SpacePoint*>(first, last));

    B2DEBUG(100, "Storing in " << layerID << " " << std::distance(first, last));
    for (auto it = first; it != last; it++) {
      B2DEBUG(100, (*it)->getVxdID());
    }
  }

  // Include the PXD layers as empty
  m_cachedHitMap.emplace(2, SortedVectorRange<const SpacePoint*>());
  m_cachedHitMap.emplace(1, SortedVectorRange<const SpacePoint*>());
}*/