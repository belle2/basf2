/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/EventT0Combiner/EventT0Combiner.h>

#include <numeric>
#include <cmath>

using namespace Belle2;

REG_MODULE(EventT0Combiner)

EventT0CombinerModule::EventT0CombinerModule() : Module()
{
  setDescription("Module to combine the EventT0 values from multiple sub-detectors");

  addParam("combinationLogic", m_paramCombinationMode, "Method of how the final T0 is selected.\n"
           "Currently '" + m_combinationModePreferCDC + "' and '" + m_combinationModeCombineCDCandECL + "' is available\n" +
           m_combinationModePreferCDC + ": the CDC t0 value (if available) will be set as the final T0 value."
           "Only if no CDC value could be found "
           "(which is very rare for BBBar events, and around 5% of low multiplicity events), the best ECL value will be set\n" +
           m_combinationModeCombineCDCandECL + ": In this mode, the CDC t0 value (if available) will be used to "
           "select the ECL t0 information which is closest in time "
           "to the best CDC value and this two values will be combined to one final value.",
           m_combinationModePreferCDC);

  setPropertyFlags(c_ParallelProcessingCertified);
}

void EventT0CombinerModule::event()
{
  if (!m_eventT0.isValid()) {
    B2DEBUG(20, "EventT0 object not created, cannot do EventT0 combination");
    return;
  }

  // check if a CDC hypothesis exists
  auto cdcHypos = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);

  if (cdcHypos.size() == 0) {
    B2DEBUG(20, "No CDC time hypothesis available, stopping");
    // if no CDC value was found, the best t0 has already been set by the ECL t0 module.
    return;
  }
  // get the latest CDC hypothesis information, this is also the most accurate t0 value the CDC can provide
  const auto cdcBestT0 = cdcHypos.back();

  B2DEBUG(20, "Best CDC time hypothesis t0 = " << cdcBestT0.eventT0 << " +- " << cdcBestT0.eventT0Uncertainty);

  if (m_paramCombinationMode == m_combinationModePreferCDC) {
    // we have a CDC value, so set this as new best global value
    B2DEBUG(20, "Setting CDC time hypothesis t0 = " << cdcBestT0.eventT0 << " +- " << cdcBestT0.eventT0Uncertainty <<
            " as new final value.");
    //set CDC value, if available
    m_eventT0->setEventT0(cdcBestT0.eventT0, cdcBestT0.eventT0Uncertainty, cdcBestT0.detectorSet);
  } else if (m_paramCombinationMode == m_combinationModeCombineCDCandECL) {
    // start comparing with all available ECL hypothesis
    auto eclHypos = m_eventT0->getTemporaryEventT0s(Const::EDetector::ECL);

    if (eclHypos.size() == 0) {
      B2DEBUG(20, "No ECL t0 hypothesis available, exiting");
      return;
    }

    EventT0::EventT0Component eclBestMatch;
    double bestDistance = std::numeric_limits<double>::max();
    bool foundMatch = false;
    for (auto const& eclHypo : eclHypos) {
      // compute distance
      double dist = std::abs(eclHypo.eventT0 - cdcBestT0.eventT0);
      B2DEBUG(20, "Checking compatibility of ECL  t0 = " << eclHypo.eventT0 << " +- " << eclHypo.eventT0Uncertainty << " distance = " <<
              dist);
      if (dist < bestDistance) {
        eclBestMatch = eclHypo;
        bestDistance = dist;
        foundMatch = true;
      }
    }

    B2DEBUG(20, "Best Matching ECL timing is t0 = " << eclBestMatch.eventT0 << " +- " << eclBestMatch.eventT0Uncertainty);

    // combine and update final value
    if (foundMatch) {
      const auto combined = computeCombination({ eclBestMatch, cdcBestT0 });
      m_eventT0->setEventT0(combined.eventT0, combined.eventT0Uncertainty, combined.detectorSet);
      B2DEBUG(20, "Combined T0 from CDC and ECL is t0 = " << combined.eventT0 << " +- " << combined.eventT0Uncertainty);
    } else {

      //set CDC value, if available
      m_eventT0->setEventT0(cdcBestT0.eventT0, cdcBestT0.eventT0Uncertainty, cdcBestT0.detectorSet);

      B2DEBUG(20, "No sufficient match found between CDC and ECL timing, setting best CDC t0 = " << cdcBestT0.eventT0 << " +- " <<
              cdcBestT0.eventT0Uncertainty);
    }
  } else {
    B2FATAL("Event t0 combination mode " << m_paramCombinationMode << " not supported.");
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


