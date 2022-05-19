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

  addParam("combinationLogic", m_paramCombinationMode, "Method of how the final T0 is selected.\n"
           "Currently '" + m_combinationModePreferSVD + ", " + m_combinationModePreferCDC + "' and '" + m_combinationModeCombineCDCandECLandSVD
           + "' is available\n" +
           m_combinationModePreferSVD + ": the SVD t0 value (if available) will be set as the final T0 value."
           "Only if no SVD value could be found "
           "(which is very rare for BBBar events, and around less than 5% of low multiplicity events), the best CDC value will be set"
           "If also no CDC value could be found "
           "(which is also very rare for BBBar events, and around 5% of low multiplicity events), the best ECL value will be set\n" +
           m_combinationModeCombineCDCandECLandSVD + ": In this mode, the SVD and CDC t0 values (if available) will be used to "
           "select the ECL t0 information which is closest in time "
           "to the best SVD and/or CDC values. If both CDC and SVD are availbale and ECL is compatible with both,"
           "SVD, CDC and ECL values will be combined to one final value. If only one between CDC and SVD is available and ECL is compatible,"
           "SVD or CDC and ECL values will be combined to one final value.",
           m_paramCombinationMode);

  setPropertyFlags(c_ParallelProcessingCertified);
}

void EventT0CombinerModule::event()
{
  if (!m_eventT0.isValid()) {
    B2DEBUG(20, "EventT0 object not created, cannot do EventT0 combination");
    return;
  }

  // check if a SVD hypothesis exists
  auto svdHypos = m_eventT0->getTemporaryEventT0s(Const::EDetector::SVD);

  // check if a CDC hypothesis exists
  auto cdcHypos = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);

  if (svdHypos.size() == 0 && cdcHypos.size() == 0) {
    B2DEBUG(20, "No SVD and CDC time hypotheses available, stopping");
    // if no CDC value was found, the best t0 has already been set by the ECL t0 module.
    return;
  }

  // get the latest SVD hypothesis information, that is also the only one available
  const auto svdBestT0 = svdHypos.back();
  B2DEBUG(20, "Best SVD time hypothesis t0 = " << svdBestT0.eventT0 << " +- " << svdBestT0.eventT0Uncertainty);

  // get the latest CDC hypothesis information, this is also the most accurate t0 value the CDC can provide
  const auto cdcBestT0 = cdcHypos.back();
  B2DEBUG(20, "Best CDC time hypothesis t0 = " << cdcBestT0.eventT0 << " +- " << cdcBestT0.eventT0Uncertainty);

  if (m_paramCombinationMode == m_combinationModePreferSVD) {
    // we have a SVD value, so set this as new best global value
    B2DEBUG(20, "Setting SVD time hypothesis t0 = " << svdBestT0.eventT0 << " +- " << svdBestT0.eventT0Uncertainty <<
            " as new final value.");
    //set SVD value, if available
    m_eventT0->setEventT0(svdBestT0);
  } else if (m_paramCombinationMode == m_combinationModePreferCDC) {
    // we have a CDC value, so set this as new best global value
    B2DEBUG(20, "Setting CDC time hypothesis t0 = " << cdcBestT0.eventT0 << " +- " << cdcBestT0.eventT0Uncertainty <<
            " as new final value.");
    //set CDC value, if available
    m_eventT0->setEventT0(cdcBestT0);
  } else if (m_paramCombinationMode == m_combinationModeCombineCDCandECLandSVD) {
    // start comparing with all available ECL hypothesis
    auto eclHypos = m_eventT0->getTemporaryEventT0s(Const::EDetector::ECL);

    if (eclHypos.size() == 0) {
      B2DEBUG(20, "No ECL t0 hypothesis available, exiting");
      return;
    }

    EventT0::EventT0Component eclBestMatch;
    double bestDistanceSVD = std::numeric_limits<double>::max();
    double bestDistanceCDC = std::numeric_limits<double>::max();
    for (auto const& eclHypo : eclHypos) {
      // compute distance
      double distSVD = std::abs(eclHypo.eventT0 - svdBestT0.eventT0);
      double distCDC = std::abs(eclHypo.eventT0 - cdcBestT0.eventT0);
      B2DEBUG(20, "Checking compatibility of ECL t0 = " << eclHypo.eventT0 << " +- " << eclHypo.eventT0Uncertainty <<
              " distance w.r.t SVD = " <<
              distSVD << ", distance w.r.t CDC = " << distCDC);

      // combine and update final value
      if (distSVD < bestDistanceSVD && distCDC < bestDistanceCDC) {
        eclBestMatch = eclHypo;
        bestDistanceSVD = distSVD;
        bestDistanceCDC = distCDC;

        B2DEBUG(20, "Best Matching ECL timing is t0 = " << eclBestMatch.eventT0 << " +- " << eclBestMatch.eventT0Uncertainty);

        const auto combined = computeCombination({ eclBestMatch, cdcBestT0, svdBestT0 });
        m_eventT0->setEventT0(combined);
        B2DEBUG(20, "Combined T0 from SVD, CDC and ECL is t0 = " << combined.eventT0 << " +- " << combined.eventT0Uncertainty);
      } else if (distSVD < bestDistanceSVD) {
        eclBestMatch = eclHypo;
        bestDistanceSVD = distSVD;

        B2DEBUG(20, "Best Matching ECL timing is t0 = " << eclBestMatch.eventT0 << " +- " << eclBestMatch.eventT0Uncertainty);

        const auto combined = computeCombination({ eclBestMatch, svdBestT0 });
        m_eventT0->setEventT0(combined);
        B2DEBUG(20, "Combined T0 from SVD and ECL is t0 = " << combined.eventT0 << " +- " << combined.eventT0Uncertainty);
      } else if (distCDC < bestDistanceCDC) {
        eclBestMatch = eclHypo;
        bestDistanceSVD = distCDC;

        B2DEBUG(20, "Best Matching ECL timing is t0 = " << eclBestMatch.eventT0 << " +- " << eclBestMatch.eventT0Uncertainty);

        const auto combined = computeCombination({ eclBestMatch, cdcBestT0 });
        m_eventT0->setEventT0(combined);
        B2DEBUG(20, "Combined T0 from CDC and ECL is t0 = " << combined.eventT0 << " +- " << combined.eventT0Uncertainty);
      } else {

        //set SVD value, if available (SVD is the one with highest efficiency)
        m_eventT0->setEventT0(svdBestT0);

        B2DEBUG(20, "No sufficient match found between SVD and ECL timing, setting best SVD t0 = " << svdBestT0.eventT0 << " +- " <<
                svdBestT0.eventT0Uncertainty);
      }
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


