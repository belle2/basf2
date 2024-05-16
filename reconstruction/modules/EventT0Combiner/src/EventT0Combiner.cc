/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/EventT0Combiner/EventT0Combiner.h>

#include <cmath>

using namespace Belle2;

REG_MODULE(EventT0Combiner);

EventT0CombinerModule::EventT0CombinerModule() : Module()
{
  setDescription("Module to combine the EventT0 values from multiple sub-detectors");

  setPropertyFlags(c_ParallelProcessingCertified);
}

void EventT0CombinerModule::event()
{
  if (!m_eventT0.isValid()) {
    B2DEBUG(20, "EventT0 object not created, cannot do EventT0 combination");
    return;
  }

  // NOT checking for m_eventT0.hasEventT0() here, as this would only indicate that so far no EventT0 has been set.
  // However, this does not mean that we can't set one from the temporary values.
  // But of course nothing can be done if no temporary EventT0s are present.
  if (m_eventT0->getTemporaryEventT0s().empty()) {
    B2DEBUG(20, "No temporary EventT0s available, can't chose the best one.");
    return;
  }

  // We have an SVD based EventT0 and it currently is set as *THE* EventT0 -> nothing to do
  if (m_eventT0->isSVDEventT0()) {
    B2DEBUG(20, "EventT0 already based on SVD information, nothing to do.");
    return;
  }

  // If we don't have an SVD based EventT0, the second choice is the EventT0 estimate using CDC information calculabed by the
  // FullGridChi2TrackTimeExtractor method. In principle, this algorithm can create EventT0 estimates using two methods:
  // "grid" and "chi2". We are only interested in the latter one.
  // If no SVD based EventT0 is present, but a CDC based one using the "chi2" algorithm is available -> nothing to do
  if (m_eventT0->isCDCEventT0()) {
    const auto& bestCDCT0 = m_eventT0->getBestCDCTemporaryEventT0();
    if ((*bestCDCT0).algorithm == "chi2") {
      B2DEBUG(20, "Using CDC chi2 EventT0.");
      return;
    }
    B2DEBUG(20, "Current EventT0 is based on CDC, but it's not the chi2 value. Continue Search.");
  }

  // No CDC chi2 EventT0 present -> try to combine ECL and CDC hit based EventT0
  // First, clear the EventT0 so that EventT0::hasEventT0() will return false
  m_eventT0->clearEventT0();
  const auto& bestECLT0 = m_eventT0->getBestECLTemporaryEventT0();
  const auto& cdcT0Candidates = m_eventT0->getTemporaryEventT0s(Const::CDC);
  const auto& hitBasedCDCT0Candidate = std::find_if(cdcT0Candidates.begin(), cdcT0Candidates.end(), [](const auto & a) { return a.algorithm == "hit based";});

  // Strategy in case none of the SVD based or the CDC chi2 based EventT0 values is available:
  // 1) If we have both an EventT0 estimate from ECL and a CDC hit based value, combine the two
  // 2) If we only have one of the two, take that value
  // 3) If we don't have either, we have a problem -> issue a B2WARNING and clear the EventT0
  // If we arrive at 3), this means that we could only have TOP EventT0, or an EventT0 from a
  // CDC based algorithm other than "hit based" or "chi2", and so far we don't want to use these.
  if (bestECLT0 and hitBasedCDCT0Candidate != cdcT0Candidates.end()) {
    B2DEBUG(20, "Combining ECL EventT0 and CDC hit based EventT0.");
    const auto combined = computeCombination({ *bestECLT0, *hitBasedCDCT0Candidate });
    m_eventT0->setEventT0(combined);
    return;
  } else if (bestECLT0 and hitBasedCDCT0Candidate == cdcT0Candidates.end()) {
    B2DEBUG(20, "Using ECL EventT0, as CDC hit based EventT0 is not available.");
    m_eventT0->setEventT0(*bestECLT0);
    return;
  } else if (hitBasedCDCT0Candidate != cdcT0Candidates.end() and not bestECLT0) {
    B2DEBUG(20, "Using CDC hit based EventT0, as ECL EventT0 is not available.");
    m_eventT0->setEventT0(*hitBasedCDCT0Candidate);
    return;
  } else {
    B2DEBUG(20, "There is no EventT0 from neither \n" \
            " * the SVD based algorithm\n" \
            " * the CDC based chi^2 algorithm\n" \
            " * the CDC based hit-based algorithm\n" \
            " * the ECL algorithm.\n" \
            "Thus, no EventT0 value can be calculated.");
  }


}

EventT0::EventT0Component EventT0CombinerModule::computeCombination(std::vector<EventT0::EventT0Component> measurements) const
{
  if (measurements.size() == 0) {
    B2FATAL("Need at least one EvenT0 Measurement to do a sensible combination.");
  }

  double eventT0 = 0.0f;
  double preFactor = 0.0f;

  Const::DetectorSet usedDetectorSet;

  for (auto const& meas : measurements) {
    usedDetectorSet += meas.detectorSet;
    const double oneOverUncertaintySquared = 1.0f / std::pow(meas.eventT0Uncertainty, 2.0);
    eventT0 += meas.eventT0 * oneOverUncertaintySquared;
    preFactor += oneOverUncertaintySquared;
  }

  eventT0 /= preFactor;
  const auto eventT0unc = std::sqrt(1.0f / preFactor);

  return EventT0::EventT0Component(eventT0, eventT0unc, usedDetectorSet);
}


