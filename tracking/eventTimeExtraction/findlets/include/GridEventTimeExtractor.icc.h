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

#include <tracking/eventTimeExtraction/findlets/GridEventTimeExtractor.dcl.h>
#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.icc.h>
#include <tracking/eventTimeExtraction/utilities/TimeExtractionUtils.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  template <class AFindlet>
  GridEventTimeExtractor<AFindlet>::GridEventTimeExtractor()
  {
    Super::addProcessingSignalListener(&m_findlet);
  }

  template <class AFindlet>
  void GridEventTimeExtractor<AFindlet>::apply(std::vector<RecoTrack*>& recoTracks)
  {
    m_wasSuccessful = false;

    m_eventT0WithQuality.clear();

    TimeExtractionUtils::addEventT0WithQuality(recoTracks, m_eventT0, m_eventT0WithQuality);

    const double eventT0Delta = (m_param_maximalT0Value - m_param_minimalT0Value) / static_cast<double>(m_param_gridSteps);

    for (unsigned int gridIndex = 0; gridIndex <= m_param_gridSteps; gridIndex++) {
      const double eventT0Hypothesis = eventT0Delta * static_cast<double>(gridIndex) + m_param_minimalT0Value;

      m_eventT0->setEventT0(EventT0::EventT0Component(eventT0Hypothesis, NAN, Const::CDC, "grid"));
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
    }

    if (not m_eventT0WithQuality.empty()) {
      m_wasSuccessful = true;
      // Look for the best event t0 (with the smallest chi2)
      const auto& bestChi2 = std::max_element(m_eventT0WithQuality.begin(), m_eventT0WithQuality.end(),
      [](const auto & lhs, const auto & rhs) {
        return lhs.quality < rhs.quality;
      });
      m_eventT0->setEventT0(*bestChi2);
    } else {
      // We have changes the event t0, so lets switch it back
      resetEventT0();
    }
  }

  template <class AFindlet>
  void GridEventTimeExtractor<AFindlet>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    Super::exposeParameters(moduleParamList, prefix);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "iterations"),
                                  m_param_iterations,
                                  "How many iterations should be done?",
                                  m_param_iterations);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "abortOnUnsuccessfulStep"),
                                  m_param_abortOnUnsuccessfulStep,
                                  "Abort on a single unsuccessful step. Otherwise, the success is defined by the last step",
                                  m_param_abortOnUnsuccessfulStep);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalT0Value"),
                                  m_param_maximalT0Value,
                                  "Maximal Grid Value of the T0 extraction",
                                  m_param_maximalT0Value);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalT0Value"),
                                  m_param_minimalT0Value,
                                  "Maximal Grid Value of the T0 extraction",
                                  m_param_minimalT0Value);
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "gridSteps"),
                                  m_param_gridSteps,
                                  "Number of steps in the grid",
                                  m_param_gridSteps);

    m_findlet.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("sub", prefix));
  }
}
