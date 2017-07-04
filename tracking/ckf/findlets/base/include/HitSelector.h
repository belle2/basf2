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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <tracking/ckf/filters/base/LayerToggledFilter.h>
#include <tracking/ckf/findlets/base/AdvanceAlgorithm.h>
#include <tracking/ckf/findlets/base/KalmanUpdateFitter.h>

#include <tracking/ckf/utilities/SelectionAlgorithms.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  /**
   * Main findlet for the CKF for CDC RecoTracks and SpacePoints from the VXD.
   *
   * For a given state on a given number (~ layer), a list of child states (one for each next hit = space point in the track
   * candidate) is filtered. This is done in several steps:
   * * filter out only the best 2 * N candidates using a configurable filter based on this geometrical information
   * * extrapolate the mSoP in each state to its corresponding hit
   * * filter again using the best N candidates
   * * do a Kalman update step for each hit separately
   * * filter a last time
   *
   * The filters can be configured independently.
   *
   * For performance reasons, we do only return a vector of pointers to states.
   * The states are actually constructed in the subfindlet SpacePointMatcher and are kept as
   * long as needed. This means the ownership still belongs to this module.
   */
  template <class AFilterFactory>
  class HitSelector : public TrackFindingCDC::Findlet<typename AFilterFactory::CreatedFilter::Object*> {
  private:
    /// The parent class
    using Super = TrackFindingCDC::Findlet<typename AFilterFactory::CreatedFilter::Object*>;
  public:
    /// Constructor adding the subfindlets as listeners
    HitSelector();

    /// Expose the parameters of the filters and our own parameters
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Main function of this findlet: return a range of selected child states for a given current state
    void apply(std::vector<typename AFilterFactory::CreatedFilter::Object*>& childStates) override;

  private:
    /// Parameter: do the advance step
    bool m_param_advance = true;
    /// Parameter: do the fit step
    bool m_param_fit = true;
    /// Parameter: use only the best N results
    unsigned int m_param_useNResults = 5;

    /// Subfindlet: Filter 1
    LayerToggledFilter<AFilterFactory> m_firstFilter;
    /// Subfindlet: Filter 2
    LayerToggledFilter<AFilterFactory> m_secondFilter;
    /// Subfindlet: Filter 3
    LayerToggledFilter<AFilterFactory> m_thirdFilter;
    /// Subfindlet: Advancer
    AdvanceAlgorithm m_advanceAlgorithm;
    /// Subfindlet: Fitter
    KalmanUpdateFitter m_fitterAlgorithm;
  };

  template <class AFilterFactory>
  HitSelector<AFilterFactory>::HitSelector() : Super()
  {
    Super::addProcessingSignalListener(&m_firstFilter);
    Super::addProcessingSignalListener(&m_secondFilter);
    Super::addProcessingSignalListener(&m_thirdFilter);
    Super::addProcessingSignalListener(&m_advanceAlgorithm);
    Super::addProcessingSignalListener(&m_fitterAlgorithm);
  }

  template <class AFilterFactory>
  void HitSelector<AFilterFactory>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_firstFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "first"));
    m_secondFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "second"));
    m_thirdFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "third"));

    m_advanceAlgorithm.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "advance"));
    m_fitterAlgorithm.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "fitter"));

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "advance"), m_param_advance,
                                  "Do the advance step.", m_param_advance);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "fit"), m_param_fit,
                                  "Do the fit step.", m_param_fit);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useNResults"), m_param_useNResults,
                                  "Do only use the best N results.", m_param_useNResults);
  }

  template <class AFilterFactory>
  void HitSelector<AFilterFactory>::apply(std::vector<typename AFilterFactory::CreatedFilter::Object*>& childStates)
  {
    B2DEBUG(50, "Starting with " << childStates.size() << " states");

    applyAndFilter(childStates, m_firstFilter, 2 * m_param_useNResults);
    B2DEBUG(50, "First filter has found " << childStates.size() << " states");

    if (m_param_advance) {
      applyAndFilter(childStates, m_advanceAlgorithm);
      B2DEBUG(50, "Advance has found " << childStates.size() << " states");
    }

    applyAndFilter(childStates, m_secondFilter, m_param_useNResults);
    B2DEBUG(50, "Second filter has found " << childStates.size() << " states");

    if (m_param_fit) {
      applyAndFilter(childStates, m_fitterAlgorithm);
      B2DEBUG(50, "Fit filter has found " << childStates.size() << " states");
    }

    applyAndFilter(childStates, m_thirdFilter, m_param_useNResults);
    B2DEBUG(50, "Third filter has found " << childStates.size() << " states");
  }
}