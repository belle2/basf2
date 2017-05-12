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

#include <tracking/trackFindingCDC/utilities/SortedVectorRange.h>
#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CDCToSpacePointMatcher : public TrackFindingCDC::ProcessingSignalListener {
  public:
    /// Main function: return the next possible hits for a given state.
    TrackFindingCDC::SortedVectorRange<const SpacePoint*> getMatchingHits(CKFCDCToVXDStateObject& currentState);

    /// Fill the cache for each event
    //void beginEvent(std::vector<RecoTrack*>& seedsVector, std::vector<const SpacePoint*>& filteredHitVector) final;

  private:
    /// Cache for sorted hits
    std::map<unsigned int, TrackFindingCDC::SortedVectorRange<const SpacePoint*>> m_cachedHitMap;

    /// Maximal number of ladders per layer
    std::vector<unsigned int> m_maximumLadderNumbers = {8, 12, 7, 10, 12, 16};
  };
}
