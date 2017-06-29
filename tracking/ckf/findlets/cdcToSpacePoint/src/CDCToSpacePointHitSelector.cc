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

CDCToSpacePointHitSelector::CDCToSpacePointHitSelector() : TrackFindingCDC::Findlet<CKFCDCToVXDStateObject * >()
{
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

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "advance"), m_param_advance,
                                "Do the advance step.", m_param_advance);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "fit"), m_param_fit,
                                "Do the fit step.", m_param_fit);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useNResults"), m_param_useNResults,
                                "Do only use the best N results.", m_param_useNResults);
}

void CDCToSpacePointHitSelector::apply(std::vector<CKFCDCToVXDStateObject*>& childStates)
{
  // TODO: Also make those a findlet!
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