/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/ckf/utilities/StateAlgorithms.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class VXDToCDCMatcher : public TrackFindingCDC::ProcessingSignalListener {
  public:
    /// return the next hits for a given state, which are the hits on the next layer (or the same for overlaps)
    template<class AStateObject>
    TrackFindingCDC::VectorRange<const TrackFindingCDC::CDCRLWireHit*> getMatchingHits(AStateObject& currentState);

    /// Fill the cache of hits for each event
    void initializeEventCache(std::vector<RecoTrack*>& seedsVector,
                              std::vector<const TrackFindingCDC::CDCRLWireHit*>& filteredHitVector);

    /// Expose parameters (if we would have such a thing)
    void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                          const std::string& prefix __attribute__((unused)))
    {
    }

  private:
    /// Cache for sorted hits
    std::map<unsigned int, TrackFindingCDC::VectorRange<const TrackFindingCDC::CDCRLWireHit*>> m_cachedHitMap;
  };

  template<class AStateObject>
  TrackFindingCDC::VectorRange<const TrackFindingCDC::CDCRLWireHit*> VXDToCDCMatcher::getMatchingHits(AStateObject& currentState)
  {
    const unsigned int currentNumber = currentState.getNumber();
    const unsigned int nextLayer = extractGeometryLayer(currentState) + 1;

    B2INFO("Asked for number " << currentNumber << " for layer " << nextLayer << " which is a " << isOnOverlapLayer(currentState));
    B2INFO("Will return all hits from layer " << nextLayer << ", which are " << m_cachedHitMap[nextLayer].size());
    return m_cachedHitMap[nextLayer];
  }

  void VXDToCDCMatcher::initializeEventCache(std::vector<RecoTrack*>& seedsVector __attribute__((unused)),
                                             std::vector<const TrackFindingCDC::CDCRLWireHit*>& filteredHitVector)
  {
    m_cachedHitMap.clear();

    const auto hitSorterByLayer = [](const TrackFindingCDC::CDCRLWireHit * lhs, const TrackFindingCDC::CDCRLWireHit * rhs) {
      return lhs->getWireID().getICLayer() < rhs->getWireID().getICLayer();
    };

    const auto getLayer = [](const TrackFindingCDC::CDCRLWireHit * wireHit) {
      return wireHit->getWireID().getICLayer();
    };

    std::sort(filteredHitVector.begin(), filteredHitVector.end(), hitSorterByLayer);

    const auto& groupedByLayer = TrackFindingCDC::adjacent_groupby(filteredHitVector.begin(), filteredHitVector.end(), getLayer);
    for (const auto& group : groupedByLayer) {
      const auto& layer = getLayer(group.front());
      m_cachedHitMap.emplace(layer, group);
      B2INFO("Storing in " << layer << " " << group.size() << " hits");
    }
  }
}
