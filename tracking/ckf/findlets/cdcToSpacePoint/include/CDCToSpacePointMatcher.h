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
#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class CDCToSpacePointMatcher : public TrackFindingCDC::ProcessingSignalListener {
  public:
    /**
     * Return a range of all possible next child states on the next layer or all hits on the next segment on the same
     * layer for overlaps.
     *
     * Returned is actually not a vector of states, but pointers to temporarily created states
     * (precisely, they are not recalculated but the ones from events/iterations before
     * are reused und reset). As it is very important to keep those states in memory until they are fully
     * processed, we keep a different vector of states for each number ( = two per layer).
     */
    std::vector<CKFCDCToVXDStateObject*> getChildStates(CKFCDCToVXDStateObject& currentState);

    /// Fill the cache of hits for each event
    void initializeEventCache(std::vector<RecoTrack*>& seedsVector, std::vector<const SpacePoint*>& filteredHitVector);

    /// Expose the hit jump parameters
    void exposeParameters(ModuleParamList* moduleParamList,  const std::string& prefix)
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "makeHitJumpingPossible"), m_param_makeHitJumpingPossible,
                                    "", m_param_makeHitJumpingPossible);
    }

  private:
    /// Cache for sorted hits
    std::map<unsigned int, TrackFindingCDC::VectorRange<const SpacePoint*>> m_cachedHitMap;

    /// Temporary object pool for finding the next state
    std::array < std::vector<CKFCDCToVXDStateObject>, CKFCDCToVXDStateObject::N + 1 > m_temporaryStates;

    /// Maximal number of ladders per layer
    std::vector<unsigned int> m_maximumLadderNumbers = {8, 12, 7, 10, 12, 16};

    /// Parameter: make hit jumps possible (missing hits on a layer)
    bool m_param_makeHitJumpingPossible = true;

    /// return the next hits for a given state, which are the hits on the next layer (or the same for overlaps)
    TrackFindingCDC::VectorRange<const SpacePoint*> getMatchingHits(CKFCDCToVXDStateObject& currentState);
  };
}
