/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
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
  addProcessingSignalListener(&m_recoTrackLoader);
  addProcessingSignalListener(&m_fullGridExtraction);
  addProcessingSignalListener(&m_trackTimeExtraction);

  ModuleParamList moduleParamList;
  const std::string prefix = "";
  this->exposeParameters(&moduleParamList, prefix);
  moduleParamList.getParameter<unsigned int>("maximalIterations").setDefaultValue(1);
  moduleParamList.getParameter<unsigned int >("minimalIterations").setDefaultValue(1);
}

void CombinedTrackTimeExtraction::initialize()
{
  Super::initialize();
}

void CombinedTrackTimeExtraction::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_fullGridExtraction.exposeParameters(moduleParamList, prefix);
  m_trackTimeExtraction.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "useFullGridExtraction"), m_param_useFullGridExtraction,
                                "use full grid t0 extraction in case the fast methods don't work",
                                m_param_useFullGridExtraction);

}



void CombinedTrackTimeExtraction::apply()
{
  m_recoTrackLoader.apply(m_recoTracks);

  // make sure the EventT0 object is available and created (even if we don't fill it later)
  if (!m_eventT0.isValid()) {
    m_eventT0.create();
  }

  // is there already an output from the Fast CDC hit finder ?
  bool doFullGridExtraction = true;
  if (m_eventT0->hasEventT0(Belle2::Const::CDC)) {

    const double fastExtractT0 = m_eventT0->getEventT0(Belle2::Const::CDC);
    const double fastExtractT0Uncertainty = m_eventT0->getEventT0Uncertainty(Belle2::Const::CDC);

    B2DEBUG(50, "Will use initial estimate of CDC hit based t0 of t=" << fastExtractT0 << " += " << fastExtractT0Uncertainty);

    // use fast hit-based as starting point for the TrackTimeExtraction
    m_trackTimeExtraction.apply(m_recoTracks);

    // check if t0 extraction was successful, if not the CDC number will be NAN
    double timeExtractT0 = m_eventT0->getEventT0(Belle2::Const::CDC);

    if (std::isnan(timeExtractT0)) {
      B2DEBUG(50, "CDC t0 fit failed");
      // set the old (best) t0 value and run full grid extraction
      m_eventT0->addEventT0(fastExtractT0, fastExtractT0Uncertainty, Belle2::Const::CDC);
    } else {
      B2DEBUG(50, "CDC t0 fit successfull, will not do full extraction");
      doFullGridExtraction = false;
    }
  }

  // sufficiently precise ?
  if (doFullGridExtraction) {
    B2DEBUG(50, "Running full grid search for CDC t0 fit extraction");
    m_fullGridExtraction.apply(m_recoTracks);
  }

  B2DEBUG(50, "CDC t0 result with combined method: " << m_eventT0->getEventT0(Belle2::Const::CDC) <<
          " +- " << m_eventT0->getEventT0Uncertainty(Belle2::Const::CDC));
}
