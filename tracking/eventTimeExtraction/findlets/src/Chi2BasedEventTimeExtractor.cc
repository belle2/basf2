/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/eventTimeExtraction/findlets/Chi2BasedEventTimeExtractor.h>

#include <tracking/eventTimeExtraction/utilities/TimeExtractionUtils.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void Chi2BasedEventTimeExtractor::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "maximalExtractedT0"), m_param_maximalExtractedT0,
                                "Hard cut on this value of extracted times in the positive as well as the negative direction.",
                                m_param_maximalExtractedT0);
}

void Chi2BasedEventTimeExtractor::apply(std::vector<RecoTrack*>& recoTracks)
{
  m_wasSuccessful = false;

  // Attention: We do *not* set the dirty flag here! We will assume that the last fit has happened with the current event t0.
  const auto& derivatives = TimeExtractionUtils::getExtractedTimeAndUncertaintyWithFit(recoTracks, false);
  double extractedDeltaT0 = derivatives.first;
  double uncertainty = derivatives.second;

  if (std::isnan(extractedDeltaT0)) {
    B2DEBUG(25, "Extracted delta t0 is nan. Aborting");
    return;
  }

  if (m_eventT0->hasEventT0()) {
    extractedDeltaT0 += m_eventT0->getEventT0();
    const double oldUncertainty = m_eventT0->getEventT0Uncertainty();
    uncertainty = std::sqrt(uncertainty * uncertainty + oldUncertainty * oldUncertainty);
  }

  if (std::abs(extractedDeltaT0) > m_param_maximalExtractedT0) {
    B2DEBUG(25, "Extracted t0 of " << extractedDeltaT0 << " is too large");
    return;
  }

  EventT0::EventT0Component eventT0Component(extractedDeltaT0, uncertainty, Const::CDC, "chi2");
  m_eventT0->setEventT0(eventT0Component);
  m_wasSuccessful = true;
  B2DEBUG(25, "Chi2 gave a result of " << extractedDeltaT0);
}
