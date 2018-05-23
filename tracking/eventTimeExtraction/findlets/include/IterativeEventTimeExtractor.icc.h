/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/eventTimeExtraction/findlets/IterativeEventTimeExtractor.dcl.h>
#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.icc.h>
#include <tracking/eventTimeExtraction/utilities/TimeExtractionUtils.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  template <class AFindlet>
  IterativeEventTimeExtractor<AFindlet>::IterativeEventTimeExtractor()
  {
    Super::addProcessingSignalListener(&m_findlet);
  }

  template <class AFindlet>
  void IterativeEventTimeExtractor<AFindlet>::apply(std::vector<RecoTrack*>& recoTracks)
  {
    m_wasSuccessful = false;

    m_eventT0WithQuality.clear();

    TimeExtractionUtils::addEventT0WithQuality(recoTracks, m_eventT0, m_eventT0WithQuality);

    for (unsigned int iteration = 0; iteration < m_param_iterations; iteration++) {
      // The findlet will set the final event t0, but will probably not add any temporary event t0s, which is fine as we will do so.
      m_findlet.apply(recoTracks);

      if (m_findlet.wasSuccessful()) {
        TimeExtractionUtils::addEventT0WithQuality(recoTracks, m_eventT0, m_eventT0WithQuality);
      } else if (m_param_abortOnUnsuccessfulStep) {
        B2DEBUG(50, "Aborting because time extraction was not successful.");
        break;
      }
    }

    if (not m_eventT0WithQuality.empty()) {
      if (m_param_useLastEventT0) {
        m_eventT0->addTemporaryEventT0(m_eventT0WithQuality.back());
        m_eventT0->setEventT0(m_eventT0WithQuality.back());
      } else {
        // Look for the best event t0 (with the smallest chi2)
        const auto& bestEventT0 = std::max_element(m_eventT0WithQuality.begin(), m_eventT0WithQuality.end(),
        [](const auto & lhs, const auto & rhs) {
          return lhs.quality < rhs.quality;
        });
        m_eventT0->setEventT0(*bestEventT0);
      }
      m_wasSuccessful = true;
    }
  }

  template <class AFindlet>
  void IterativeEventTimeExtractor<AFindlet>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    Super::exposeParameters(moduleParamList, prefix);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "iterations"),
                                  m_param_iterations,
                                  "How many iterations should be done?",
                                  m_param_iterations);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useLastEventT0"),
                                  m_param_useLastEventT0,
                                  "Use the last event t0 instead of the best one.",
                                  m_param_useLastEventT0);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "abortOnUnsuccessfulStep"),
                                  m_param_abortOnUnsuccessfulStep,
                                  "Abort on a single unsuccessful step. Otherwise, the success is defined by the last step",
                                  m_param_abortOnUnsuccessfulStep);

    m_findlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("sub", prefix));
  }
}
