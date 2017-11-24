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

using namespace Belle2;


CombinedTrackTimeExtraction::CombinedTrackTimeExtraction()
{
  addProcessingSignalListener(&m_recoTrackLoader);
  addProcessingSignalListener(&m_fullGridExtraction);
  addProcessingSignalListener(&m_trackTimeExtraction);
}

void CombinedTrackTimeExtraction::initialize()
{
  Super::initialize();
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
    // use fast hit-based as starting point for the TrackTimeExtraction
    m_trackTimeExtraction.apply(m_recoTracks);

    // sufficiently precise ?

  }

}
