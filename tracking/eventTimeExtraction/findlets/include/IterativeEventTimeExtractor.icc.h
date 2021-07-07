/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/eventTimeExtraction/findlets/IterativeEventTimeExtractor.dcl.h>
#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.icc.h>
#include <tracking/eventTimeExtraction/utilities/TimeExtractionUtils.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

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

    unsigned int iteration = 0;
    for (; iteration < m_param_maxIterations; iteration++) {
      // The findlet will set the final event t0, but will probably not add any temporary event t0s, which is fine as we will do so.
      m_findlet.apply(recoTracks);

      bool breakLoop = false;
      if (m_findlet.wasSuccessful()) {

        if (not m_eventT0WithQuality.empty()) {
          const double deltaT0 = std::abs(m_eventT0->getEventT0() - m_eventT0WithQuality.back().eventT0);
          if (deltaT0 < m_param_minimalDeltaT0 and iteration >= m_param_minIterations) {
            breakLoop = true;
          }
        }
        TimeExtractionUtils::addEventT0WithQuality(recoTracks, m_eventT0, m_eventT0WithQuality);
      } else if (m_param_abortOnUnsuccessfulStep) {
        B2DEBUG(50, "Aborting because time extraction was not successful.");
        breakLoop = true;
      }

      if (breakLoop) {
        break;
      }
    }

    if (not m_eventT0WithQuality.empty() and iteration != m_param_maxIterations) {
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

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maxIterations"),
                                  m_param_maxIterations,
                                  "How many iterations should be done maximally?",
                                  m_param_maxIterations);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minIterations"),
                                  m_param_minIterations,
                                  "How many iterations should be done minimally?",
                                  m_param_minIterations);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalDeltaT0"),
                                  m_param_minimalDeltaT0,
                                  "What is the final precision?",
                                  m_param_minimalDeltaT0);
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
