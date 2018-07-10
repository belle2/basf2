/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/eventTimeExtraction/findlets/FullGridChi2TrackTimeExtractor.h>
#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.icc.h>
#include <tracking/eventTimeExtraction/findlets/GridEventTimeExtractor.icc.h>
#include <tracking/eventTimeExtraction/findlets/IterativeEventTimeExtractor.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

FullGridChi2TrackTimeExtractor::FullGridChi2TrackTimeExtractor()
{
  addProcessingSignalListener(&m_finalExtractor);
  addProcessingSignalListener(&m_gridExtractor);
}

/// Copy the parameters
void FullGridChi2TrackTimeExtractor::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_gridExtractor.exposeParameters(moduleParamList, prefixed("Grid", prefix));
  m_finalExtractor.exposeParameters(moduleParamList, prefixed("Refiner", prefix));

  moduleParamList->getParameter<unsigned int>("GridIterations").setDefaultValue(1);
  moduleParamList->getParameter<unsigned int>("RefinerIterations").setDefaultValue(1);
  moduleParamList->getParameter<bool>("RefinerUseLastEventT0").setDefaultValue(true);
}

/// Timing extraction for this findlet
void FullGridChi2TrackTimeExtractor::apply(std::vector<RecoTrack*>& recoTracks)
{
  const auto& temporaryT0Extractions = m_eventT0->getTemporaryEventT0s(Const::CDC);
  B2ASSERT("There should only be a single or none extraction at this stage!", temporaryT0Extractions.size() <= 1);

  if (temporaryT0Extractions.empty()) {
    m_gridExtractor.apply(recoTracks);
    // if neither the hit based nor the grid based extraction finds something, we can not do anything
    if (not m_gridExtractor.wasSuccessful()) {
      m_wasSuccessful = false;
      return;
    }
  } else {
    // Make sure to actually use the temporary hit based extraction result
    m_eventT0->setEventT0(temporaryT0Extractions[0]);
  }

  m_finalExtractor.apply(recoTracks);
  m_wasSuccessful = m_finalExtractor.wasSuccessful();

  if (not wasSuccessful()) {
    B2DEBUG(50, "Resetting the event t0 as the final extraction was not successful.");
    resetEventT0();
  }
}