/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/SegmentTrackCombinerModule.h>

#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>

#include <tracking/trackFindingCDC/filters/segmentInformationListTrack/SegmentInformationListTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentTrain/SegmentTrainFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segment/SegmentFilterFactory.h>
#include <tracking/trackFindingCDC/filters/newSegment/NewSegmentFilterFactory.h>
#include <tracking/trackFindingCDC/filters/track/TrackFilterFactory.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombiner)

SegmentTrackCombinerModule::SegmentTrackCombinerModule()
  : m_chooseableSegmentTrackFilterFirstStep(makeUnique<SegmentTrackFilterFirstStepFactory>("none"))
  , m_trackRejecter("all")
{
  this->setDescription("Versatile module with adjustable filters for segment track combination.");

  ModuleParamList mpl = this->getParamList();
  m_chooseableSegmentTrackFilterFirstStep.exposeParameters(&mpl, "SegmentTrackFilterFirstStep");
  m_trackRejecter.exposeParameters(&mpl, "Track");
  this->setParamList(mpl);
}


void SegmentTrackCombinerModule::initialize()
{
  Super::initialize();
  m_chooseableSegmentTrackFilterFirstStep.initialize();
  m_trackRejecter.initialize();

  // Require the Monte Carlo information, should be redundant:
  // The filters that require the truth information should have required
  // it in their initialize. Since this used to be here and it does not harm
  // we keep it for the moment
  if (m_chooseableSegmentTrackFilterFirstStep.needsTruthInformation())
    CDCMCManager::getInstance().requireTruthInformation();
}
}

void SegmentTrackCombinerModule::beginRun()
{
  Super::beginRun();
  m_chooseableSegmentTrackFilterFirstStep.beginRun();
  m_trackRejecter.beginRun();
}

void SegmentTrackCombinerModule::event()
{
  // Handle beginEvent such that the filter can prepare
  m_chooseableSegmentTrackFilterFirstStep.beginEvent();
  m_trackRejecter.beginEvent();

  // Fill the Monte Carlo maps, should be redundant:
  // The filters that require the truth information should have called fill
  // in their beginEvent(). Since this used to be here and it does not harm
  // we keep it for the moment.
  if (m_chooseableSegmentTrackFilterFirstStep.needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }
  Super::event();
}
void SegmentTrackCombinerModule::endRun()
{
  m_chooseableSegmentTrackFilterFirstStep.endRun();
  m_trackRejecter.endRun();
  Super::endRun();
}

void SegmentTrackCombinerModule::terminate()
{
  m_chooseableSegmentTrackFilterFirstStep.terminate();
  m_trackRejecter.terminate();
  Super::terminate();
}

// Do the combination work. See the SegmentTrackCombiner methods for full details.
void SegmentTrackCombinerModule::generate(std::vector<TrackFindingCDC::CDCSegment2D>& segments,
                                          std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  // Resort hits for the arc length information
  for (CDCTrack& track : tracks) {
    TrackQualityTools::normalizeHitsAndResetTrajectory(track);
  }

  m_combiner.fillWith(tracks, segments);
  m_combiner.match(m_chooseableSegmentTrackFilterFirstStep);
  m_trackRejecter.apply(tracks);
  m_combiner.clearAndRecover();

  // Resort hits for the arc length information
  for (CDCTrack& track : tracks) {
    TrackQualityTools::normalizeHitsAndResetTrajectory(track);
  }
}
