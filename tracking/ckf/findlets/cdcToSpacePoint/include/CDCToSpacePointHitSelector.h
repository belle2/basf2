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

#include <tracking/trackFindingCDC/utilities/CompositeProcessingSignalListener.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <tracking/ckf/states/CKFCDCToVXDStateObject.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/CDCToSpacePointMatcher.h>
#include <tracking/ckf/filters/base/LayerToggledFilter.h>
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/CDCTrackSpacePointCombinationFilterFactory.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointAdvanceAlgorithm.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointKalmanUpdateFitter.h>

namespace Belle2 {
  class CDCToSpacePointHitSelector : public TrackFindingCDC::CompositeProcessingSignalListener {
  public:
    /// Constructor adding the subfindlets as listeners
    CDCToSpacePointHitSelector();

    /// Expose the parameters of the filters and our own parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

    /// Main function of this findlet: return a range of selected child states for a given current state.
    TrackFindingCDC::VectorRange<CKFCDCToVXDStateObject> getChildStates(CKFCDCToVXDStateObject& currentState);

  private:
    /// Temporary object pool for finding the next state
    std::array < std::vector<CKFCDCToVXDStateObject>, CKFCDCToVXDStateObject::N + 1 > m_temporaryStates;

    /// Parameter: make hit jumps possible
    bool m_param_makeHitJumpingPossible = true;
    /// Parameter: do the advance step
    bool m_param_advance = true;
    /// Parameter: do the fit step
    bool m_param_fit = true;
    /// Parameter: use only the best N results
    unsigned int m_param_useNResults = 5;

    /// Subfindlet: Hit Matcher
    CDCToSpacePointMatcher m_hitMatcher;
    /// Subfindlet: Filter 1
    LayerToggledFilter<CDCTrackSpacePointCombinationFilterFactory> m_firstFilter;
    /// Subfindlet: Filter 2
    LayerToggledFilter<CDCTrackSpacePointCombinationFilterFactory> m_secondFilter;
    /// Subfindlet: Filter 3
    LayerToggledFilter<CDCTrackSpacePointCombinationFilterFactory> m_thirdFilter;
    /// Subfindlet: Advancer
    SpacePointAdvanceAlgorithm m_advanceAlgorithm;
    /// Subfindlet: Fitter
    SpacePointKalmanUpdateFitter m_fitterAlgorithm;

    // TODO: move away
    TrackFindingCDC::VectorRange<CKFCDCToVXDStateObject> fillChildStates(CKFCDCToVXDStateObject& currentState);
  };
}