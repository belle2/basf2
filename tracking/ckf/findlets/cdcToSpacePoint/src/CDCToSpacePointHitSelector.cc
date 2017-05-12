/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/cdcToSpacePoint/CDCToSpacePointHitSelector.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/ckf/utilities/SelectionAlgorithms.h>

using namespace Belle2;

CDCToSpacePointHitSelector::CDCToSpacePointHitSelector() : TrackFindingCDC::CompositeProcessingSignalListener()
{
  addProcessingSignalListener(&m_hitMatcher);
  addProcessingSignalListener(&m_firstFilter);
  addProcessingSignalListener(&m_secondFilter);
  addProcessingSignalListener(&m_thirdFilter);
  addProcessingSignalListener(&m_advanceAlgorithm);
  addProcessingSignalListener(&m_fitterAlgorithm);
}

void CDCToSpacePointHitSelector::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_firstFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "first"));
  m_secondFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "second"));
  m_thirdFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "third"));

  m_advanceAlgorithm.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "advance"));
  m_fitterAlgorithm.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "fitter"));

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "makeHitJumpingPossible"), m_param_makeHitJumpingPossible,
                                "", m_param_makeHitJumpingPossible);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "advance"), m_param_advance,
                                "Do the advance step.", m_param_advance);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "fit"), m_param_fit,
                                "Do the fit step.", m_param_fit);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useNResults"), m_param_useNResults,
                                "Do only use the best N results.", m_param_useNResults);
}

/// Main function of this findlet: return a range of selected child states for a given current state.
TrackFindingCDC::VectorRange<CKFCDCToVXDStateObject> CDCToSpacePointHitSelector::getChildStates(
  CKFCDCToVXDStateObject& currentState)
{
  auto childStates = fillChildStates(currentState);

  applyAndFilter(childStates, m_firstFilter, 2 * m_param_useNResults);

  if (m_param_advance) {
    applyAndFilter(childStates, m_advanceAlgorithm);
  }

  applyAndFilter(childStates, m_secondFilter, m_param_useNResults);

  if (m_param_fit) {
    applyAndFilter(childStates, m_fitterAlgorithm);
  }

  applyAndFilter(childStates, m_thirdFilter, m_param_useNResults);

  return childStates;
}

TrackFindingCDC::VectorRange<CKFCDCToVXDStateObject> CDCToSpacePointHitSelector::fillChildStates(
  CKFCDCToVXDStateObject& currentState)
{
  // TODO: move out

  const auto& matchingHits = m_hitMatcher.getMatchingHits(currentState);
  auto& temporaryStates = m_temporaryStates[currentState.getNumber()];
  temporaryStates.resize(matchingHits.size() + 1);

  auto lastState = temporaryStates.begin();
  for (const auto& hit : matchingHits) {
    lastState->set(&currentState, hit);
    lastState = std::next(lastState);
  }

  if (m_param_makeHitJumpingPossible) {
    lastState->set(&currentState, nullptr);
  }

  return TrackFindingCDC::VectorRange<CKFCDCToVXDStateObject>(temporaryStates.begin(), temporaryStates.end());
}