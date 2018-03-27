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
}

void CombinedTrackTimeExtraction::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_fullGridExtraction.exposeParameters(moduleParamList, prefix + "fullGridExtraction");
  m_trackTimeExtraction.exposeParameters(moduleParamList, prefix + "trackTimeExtraction");

  moduleParamList->addParameter(prefixed(prefix, "useFullGridExtraction"), m_param_useFullGridExtraction,
                                "use full grid t0 extraction in case the fast methods don't work",
                                m_param_useFullGridExtraction);

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

  B2DEBUG(50, "m_eventT0->hasEventT0(Belle2::Const::CDC) = :" << m_eventT0->hasTemporaryEventT0(Belle2::Const::CDC));
  if (m_eventT0->hasTemporaryEventT0(Belle2::Const::CDC)) {

    const double fastExtractT0 = 0.0;
    // TODO
    // m_eventT0->getEventT0(Belle2::Const::CDC);
    const double fastExtractT0Uncertainty = 0.0;
    // TODO
    //m_eventT0->getEventT0Uncertainty(Belle2::Const::CDC);

    B2DEBUG(50, "Will use initial estimate of CDC hit based t0 of t=" << fastExtractT0 << " += " << fastExtractT0Uncertainty);

    // use fast hit-based as starting point for the TrackTimeExtraction
    m_trackTimeExtraction.apply(recoTracks);

    // check if t0 extraction was successful, if not the CDC number will be NAN
    //double timeExtractT0 = m_eventT0->getEventT0(Belle2::Const::CDC);

    // todo: this is not nan any more in case the fit failed
    if (!m_trackTimeExtraction.wasSucessful()) {
      B2DEBUG(50, "CDC t0 fit failed");
      // set the old (best) t0 value and run full grid extraction
      // TODO, renable
      //m_eventT0->addEventT0(fastExtractT0, fastExtractT0Uncertainty, Belle2::Const::CDC);
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
    // todo
    /*B2DEBUG(50, "CDC t0 result with combined method: " << m_eventT0->getEventT0(Belle2::Const::CDC) <<
            " +- " << m_eventT0->getEventT0Uncertainty(Belle2::Const::CDC));*/
  } else {
    B2DEBUG(50, "CDC t0 extraction not successful, keeping the previous CDC EventT0 if has been set");
  }
}
