/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombinerDevModule.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <framework/datastore/StoreArray.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombinerDev);

SegmentTrackCombinerDevModule::SegmentTrackCombinerDevModule() :
  Super(),
  m_segmentTrackFilterFirstStepFactory("none"),
  m_backgroundSegmentsFilterFactory("none"),
  m_newSegmentsFilterFactory("none"),
  m_segmentTrackFilterSecondStepFactory("none"),
  m_segmentTrainFilterFactory("none"),
  m_segmentInformationListTrackFilterFactory("none"),
  m_trackFilterFactory("all")
{
  setDescription("Versatile module with adjustable filters for segment track combination.");

  m_segmentTrackFilterFirstStepFactory.exposeParameters(this);
  m_backgroundSegmentsFilterFactory.exposeParameters(this);
  m_newSegmentsFilterFactory.exposeParameters(this);
  m_segmentTrackFilterSecondStepFactory.exposeParameters(this);
  m_segmentTrainFilterFactory.exposeParameters(this);
  m_segmentInformationListTrackFilterFactory.exposeParameters(this);
  m_trackFilterFactory.exposeParameters(this);
}

void SegmentTrackCombinerDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentTrackFilter> ptrSegmentTrackFilterFirstStep = m_segmentTrackFilterFirstStepFactory.create();
  setSegmentTrackFilterFirstStep(std::move(ptrSegmentTrackFilterFirstStep));

  std::unique_ptr<BaseBackgroundSegmentsFilter> ptrBackgroundSegmentsFilter = m_backgroundSegmentsFilterFactory.create();
  setBackgroundSegmentFilter(std::move(ptrBackgroundSegmentsFilter));

  std::unique_ptr<BaseNewSegmentsFilter> ptrNewSegmentsFilter = m_newSegmentsFilterFactory.create();
  setNewSegmentFilter(std::move(ptrNewSegmentsFilter));

  std::unique_ptr<BaseSegmentTrackFilter> ptrSegmentTrackFilterSecondStep = m_segmentTrackFilterSecondStepFactory.create();
  setSegmentTrackFilterSecondStep(std::move(ptrSegmentTrackFilterSecondStep));

  std::unique_ptr<BaseSegmentTrainFilter> ptrSegmentTrainFilter = m_segmentTrainFilterFactory.create();
  setSegmentTrainFilter(std::move(ptrSegmentTrainFilter));

  std::unique_ptr<BaseSegmentInformationListTrackFilter> ptrSegmentInformationListTrackFilter =
    m_segmentInformationListTrackFilterFactory.create();
  setSegmentInformationListTrackFilter(std::move(ptrSegmentInformationListTrackFilter));

  std::unique_ptr<BaseTrackFilter> ptrTrackFilter = m_trackFilterFactory.create();
  setTrackFilter(std::move(ptrTrackFilter));

  Super::initialize();

  if (getSegmentTrackFilterFirstStep()->needsTruthInformation() or
      getBackgroundSegmentFilter()->needsTruthInformation() or
      getNewSegmentFilter()->needsTruthInformation() or
      getSegmentTrackFilterSecondStep()->needsTruthInformation() or
      getSegmentTrainFilter()->needsTruthInformation() or
      getSegmentInformationListTrackFilter()->needsTruthInformation() or
      getTrackFilter()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}


void SegmentTrackCombinerDevModule::event()
{
  if (getSegmentTrackFilterFirstStep()->needsTruthInformation() or
      getBackgroundSegmentFilter()->needsTruthInformation() or
      getNewSegmentFilter()->needsTruthInformation() or
      getSegmentTrackFilterSecondStep()->needsTruthInformation() or
      getSegmentTrainFilter()->needsTruthInformation() or
      getSegmentInformationListTrackFilter()->needsTruthInformation() or
      getTrackFilter()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  Super::event();
}
