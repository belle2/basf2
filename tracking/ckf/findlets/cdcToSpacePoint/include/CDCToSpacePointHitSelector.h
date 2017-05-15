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
  /**
   * Main findlet for the CKF for CDC RecoTracks and SpacePoints from the VXD (SVD).
   *
   * For a given state on a given number (~ layer), a list of child states (one for each next hit = space point in the track
   * candidate) is returned. This is done in several steps:
   * * select all hits on the next geometrical layer using the CDCToSpacePointMatcher
   * * filter out only the best 2 * N candidates using a configurable filter based on this geometrical information
   * * extrapolate the mSoP in each state to its corresponding hit
   * * filter again using the best N candidates
   * * do a Kalman update step for each hit separately
   * * filter a last time
   *
   * The filters can be configured independently.
   */
  class CDCToSpacePointHitSelector : public TrackFindingCDC::CompositeProcessingSignalListener {
  public:
    /// Constructor adding the subfindlets as listeners
    CDCToSpacePointHitSelector();

    /// Expose the parameters of the filters and our own parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

    /// Main function of this findlet: return a range of selected child states for a given current state
    TrackFindingCDC::VectorRange<CKFCDCToVXDStateObject> getChildStates(CKFCDCToVXDStateObject& currentState);

    /// Initialize the cache of the hit matcher with the hits to be used in this event
    void initializeEventCache(std::vector<RecoTrack*>& seedsVector, std::vector<const SpacePoint*>& filteredHitVector)
    {
      m_hitMatcher.initializeEventCache(seedsVector, filteredHitVector);
    }

  private:
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
  };
}