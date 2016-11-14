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
#include <tracking/trackFindingCDC/filters/backgroundSegment/BackgroundSegmentFilterFactory.h>
#include <tracking/trackFindingCDC/filters/newSegment/NewSegmentFilterFactory.h>
#include <tracking/trackFindingCDC/filters/track/TrackFilterFactory.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombiner)

SegmentTrackCombinerModule::SegmentTrackCombinerModule()
  : m_chooseableSegmentTrackFilterFirstStep(makeUnique<SegmentTrackFilterFirstStepFactory>("none"))
  , m_chooseableBackgroundSegmentFilter(makeUnique<BackgroundSegmentFilterFactory>("none"))
  , m_chooseableNewSegmentFilter(makeUnique<NewSegmentFilterFactory>("none"))
  , m_chooseableSegmentTrackFilterSecondStep(makeUnique<SegmentTrackFilterSecondStepFactory>("none"))
  , m_chooseableSegmentTrainFilter(makeUnique<SegmentTrainFilterFactory>("none"))
  , m_chooseableSegmentInformationListTrackFilter(makeUnique<SegmentInformationListTrackFilterFactory>("none"))
  , m_chooseableTrackFilter(makeUnique<TrackFilterFactory>("all"))
{
  this->setDescription("Versatile module with adjustable filters for segment track combination.");

  ModuleParamList mpl = this->getParamList();
  m_chooseableSegmentTrackFilterFirstStep.exposeParameters(&mpl, "SegmentTrackFilterFirstStep");
  m_chooseableBackgroundSegmentFilter.exposeParameters(&mpl, "BackgroundSegment");
  m_chooseableNewSegmentFilter.exposeParameters(&mpl, "NewSegment");
  m_chooseableSegmentTrackFilterSecondStep.exposeParameters(&mpl, "SegmentTrackFilterSecondStep");
  m_chooseableSegmentTrainFilter.exposeParameters(&mpl, "SegmentTrain");
  m_chooseableSegmentInformationListTrackFilter.exposeParameters(&mpl, "SegmentInformationListTrack");
  m_chooseableTrackFilter.exposeParameters(&mpl, "Track");
  this->setParamList(mpl);
}


void SegmentTrackCombinerModule::initialize()
{
  Super::initialize();
  m_chooseableSegmentTrackFilterFirstStep.initialize();
  m_chooseableBackgroundSegmentFilter.initialize();
  m_chooseableNewSegmentFilter.initialize();
  m_chooseableSegmentTrackFilterSecondStep.initialize();
  m_chooseableSegmentTrainFilter.initialize();
  m_chooseableSegmentInformationListTrackFilter.initialize();
  m_chooseableTrackFilter.initialize();

  // Require the Monte Carlo information, should be redundant:
  // The filters that require the truth information should have required
  // it in their initialize. Since this used to be here and it does not harm
  // we keep it for the moment
  if (m_chooseableSegmentTrackFilterFirstStep.needsTruthInformation() or
      m_chooseableBackgroundSegmentFilter.needsTruthInformation() or
      m_chooseableNewSegmentFilter.needsTruthInformation() or
      m_chooseableSegmentTrackFilterSecondStep.needsTruthInformation() or
      m_chooseableSegmentTrainFilter.needsTruthInformation() or
      m_chooseableSegmentInformationListTrackFilter.needsTruthInformation() or
      m_chooseableTrackFilter.needsTruthInformation()) {
    CDCMCManager::getInstance().requireTruthInformation();
  }
}

void SegmentTrackCombinerModule::beginRun()
{
  Super::beginRun();
  m_chooseableSegmentTrackFilterFirstStep.beginRun();
  m_chooseableBackgroundSegmentFilter.beginRun();
  m_chooseableNewSegmentFilter.beginRun();
  m_chooseableSegmentTrackFilterSecondStep.beginRun();
  m_chooseableSegmentTrainFilter.beginRun();
  m_chooseableSegmentInformationListTrackFilter.beginRun();
  m_chooseableTrackFilter.beginRun();
}

void SegmentTrackCombinerModule::event()
{
  // Handle beginEvent such that the filter can prepare
  m_chooseableSegmentTrackFilterFirstStep.beginEvent();
  m_chooseableBackgroundSegmentFilter.beginEvent();
  m_chooseableNewSegmentFilter.beginEvent();
  m_chooseableSegmentTrackFilterSecondStep.beginEvent();
  m_chooseableSegmentTrainFilter.beginEvent();
  m_chooseableSegmentInformationListTrackFilter.beginEvent();
  m_chooseableTrackFilter.beginEvent();

  // Fill the Monte Carlo maps, should be redundant:
  // The filters that require the truth information should have called fill
  // in their beginEvent(). Since this used to be here and it does not harm
  // we keep it for the moment.
  if (m_chooseableSegmentTrackFilterFirstStep.needsTruthInformation() or
      m_chooseableBackgroundSegmentFilter.needsTruthInformation() or
      m_chooseableNewSegmentFilter.needsTruthInformation() or
      m_chooseableSegmentTrackFilterSecondStep.needsTruthInformation() or
      m_chooseableSegmentTrainFilter.needsTruthInformation() or
      m_chooseableSegmentInformationListTrackFilter.needsTruthInformation() or
      m_chooseableTrackFilter.needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }
  Super::event();
}
void SegmentTrackCombinerModule::endRun()
{
  m_chooseableTrackFilter.endRun();
  m_chooseableSegmentInformationListTrackFilter.endRun();
  m_chooseableSegmentTrainFilter.endRun();
  m_chooseableSegmentTrackFilterSecondStep.endRun();
  m_chooseableNewSegmentFilter.endRun();
  m_chooseableBackgroundSegmentFilter.endRun();
  m_chooseableSegmentTrackFilterFirstStep.endRun();
  Super::endRun();
}

void SegmentTrackCombinerModule::terminate()
{
  m_chooseableTrackFilter.terminate();
  m_chooseableSegmentInformationListTrackFilter.terminate();
  m_chooseableSegmentTrainFilter.terminate();
  m_chooseableSegmentTrackFilterSecondStep.terminate();
  m_chooseableNewSegmentFilter.terminate();
  m_chooseableBackgroundSegmentFilter.terminate();
  m_chooseableSegmentTrackFilterFirstStep.terminate();
  Super::terminate();
}

// Do the combination work. See the SegmentTrackCombiner methods for full details.
void SegmentTrackCombinerModule::generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments,
                                          std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  // Resort hits for the arc length information
  for (CDCTrack& track : tracks) {
    TrackQualityTools::normalizeHitsAndResetTrajectory(track);
  }

  m_combiner.fillWith(tracks, segments);
  m_combiner.match(m_chooseableSegmentTrackFilterFirstStep);
  m_combiner.filterSegments(m_chooseableBackgroundSegmentFilter);
  m_combiner.filterOutNewSegment(m_chooseableNewSegmentFilter);
  m_combiner.combine(m_chooseableSegmentTrackFilterSecondStep,
                     m_chooseableSegmentTrainFilter,
                     m_chooseableSegmentInformationListTrackFilter);
  m_combiner.filterTracks(tracks, m_chooseableTrackFilter);
  m_combiner.clearAndRecover();

  // Resort hits for the arc length information
  for (CDCTrack& track : tracks) {
    TrackQualityTools::normalizeHitsAndResetTrajectory(track);
  }
}
