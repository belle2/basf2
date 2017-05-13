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
    /// Return a range of all possible next child states on the next layer.
    TrackFindingCDC::VectorRange<CKFCDCToVXDStateObject> getChildStates(CKFCDCToVXDStateObject& currentState);

    /// Fill the cache for each event
    void initializeEventCache(std::vector<RecoTrack*>& seedsVector, std::vector<const SpacePoint*>& filteredHitVector);

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

    /// Parameter: make hit jumps possible
    bool m_param_makeHitJumpingPossible = true;

    /// return the next possible hits for a given state.
    TrackFindingCDC::VectorRange<const SpacePoint*> getMatchingHits(CKFCDCToVXDStateObject& currentState);
  };
}
