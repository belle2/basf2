/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackTimeExtraction/CombinedTrackTimeExtraction.h>
#include <tracking/timeExtraction/TimeExtractionUtils.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/ckf/general/utilities/ClassMnemomics.h>

#include <numeric>
#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;


CombinedTrackTimeExtraction::CombinedTrackTimeExtraction()
{
  addProcessingSignalListener(&m_fullGridExtraction);
  addProcessingSignalListener(&m_trackTimeExtraction);

  ModuleParamList moduleParamList;
  const std::string prefix = "";
  this->exposeParameters(&moduleParamList, prefix);
}

void CombinedTrackTimeExtraction::initialize()
{
  Super::initialize();

  m_eventT0.registerInDataStore();
}

void CombinedTrackTimeExtraction::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_fullGridExtraction.exposeParameters(moduleParamList, prefix + "fullGridExtraction");
  m_trackTimeExtraction.exposeParameters(moduleParamList, prefix + "trackTimeExtraction");

  moduleParamList->addParameter(prefixed(prefix, "useFullGridExtraction"), m_param_useFullGridExtraction,
                                "use full grid t0 extraction in case the fast methods don't work",
                                m_param_useFullGridExtraction);
  moduleParamList->addParameter(prefixed(prefix, "setAsFinalEventT0"), m_param_setAsFinalEventT0,
                                "Set the event T0 (if one was found) as the final entry in the EventT0 object.",
                                m_param_setAsFinalEventT0);

}

void CombinedTrackTimeExtraction::apply(std::vector<RecoTrack*>& recoTracks)
{
  B2DEBUG(50, "Got " << recoTracks.size() << " RecoTracks for time extraction (before selection of tracks used for t0 fit)");

  // check if there are any reco tracks at all available
  if (recoTracks.size() == 0) {
    B2DEBUG(50, "No tracks for time extraction available, skipping time extraction for this event ");
    return;
  }

  // make sure the EventT0 object is available and created (even if we don't fill it later)
  if (!m_eventT0.isValid()) {
    m_eventT0.create();
  }

  // is there already an output from the Fast CDC hit finder ?
  bool doFullGridExtraction = true;
  bool extractionSuccesful = false;

  std::tuple <bool, EventT0::EventT0Component> initialT0 = {false, EventT0::EventT0Component()};
  auto currentEvent0 = m_eventT0->getEventT0Component();
  // check and unpack boost::optional
  if (currentEvent0) {
    initialT0 = {true, *currentEvent0 };
  }

  B2DEBUG(50, "m_eventT0->hasEventT0(Belle2::Const::CDC) = :" << m_eventT0->hasTemporaryEventT0(Belle2::Const::CDC));
  if (m_eventT0->hasTemporaryEventT0(Belle2::Const::CDC)) {
    // get the last (newest) entry. At this stage, there should be onyl one entry from the CDC, as only the CDC hit time extraction was run
    // so far
    auto newestEventT0 = m_eventT0->getTemporaryEventT0s(Belle2::Const::CDC).back();
    const double fastExtractT0 = newestEventT0.eventT0;
    const double fastExtractT0Uncertainty = newestEventT0.eventT0Uncertainty;

    B2DEBUG(50, "Will use initial estimate of CDC hit based t0 of t=" << fastExtractT0 << " += " << fastExtractT0Uncertainty);

    m_eventT0->setEventT0(fastExtractT0, fastExtractT0Uncertainty, Const::EDetector::CDC);

    // use fast hit-based as starting point for the TrackTimeExtraction
    m_trackTimeExtraction.apply(recoTracks);

    if (!m_trackTimeExtraction.wasSucessful()) {
      B2DEBUG(50, "CDC t0 fit failed");
    } else {
      B2DEBUG(50, "CDC t0 fit successful, will not do full extraction");
      doFullGridExtraction = false;
      extractionSuccesful = true;
    }
  }

  // sufficiently precise ?
  if (doFullGridExtraction && m_param_useFullGridExtraction) {
    B2DEBUG(50, "Running full grid search for CDC t0 fit extraction");
    m_fullGridExtraction.apply(recoTracks);
    extractionSuccesful = m_fullGridExtraction.wasSuccessful();
  }

  if (extractionSuccesful) {
    // The value of either the fullgridextraction or of the TrackTimeExtraction will be the last one added
    // if successulf
    auto newestEventT0 = m_eventT0->getTemporaryEventT0s(Belle2::Const::CDC).back();
    B2DEBUG(50, "CDC t0 result with combined method: " << newestEventT0.eventT0 <<
            " +- " << newestEventT0.eventT0Uncertainty);

    if (m_param_setAsFinalEventT0) {
      B2DEBUG(50, "Setting this CDC Event T0 as final value for now");
      initialT0 = {true, newestEventT0};
    }
  } else {
    B2DEBUG(50, "CDC t0 extraction not successful, keeping the previous CDC EventT0 if has been set");
  }

  // set back the previously determined value, if one existed
  // this was modified by the iterations done during the finding procedure
  if (std::get<0>(initialT0)) {
    const auto t0Comp = std::get<1>(initialT0);
    m_eventT0->setEventT0(t0Comp.eventT0, t0Comp.eventT0Uncertainty, t0Comp.detectorSet);
  } else {
    // otherwise, be sure to remove any values which might have been set during the finding
    // procedure
    m_eventT0->clearEventT0();
  }
}
