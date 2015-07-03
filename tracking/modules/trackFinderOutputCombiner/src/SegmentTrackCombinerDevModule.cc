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
  SegmentTrackCombinerImplModule<>(),
  m_segmentTrackChooserFirstStepFactory("tmva"),
  m_backgroundSegmentsFilterFactory("tmva"),
  m_newSegmentsFilterFactory("none"),
  m_segmentTrackChooserSecondStepFactory("none"),
  m_segmentTrainFilterFactory("none"),
  m_segmentTrackFilterFactory("none")
{
  setDescription("Versatile module with adjustable filters for segment track combination.");

  m_segmentTrackChooserFirstStepFactory.exposeParameters(this);
  m_backgroundSegmentsFilterFactory.exposeParameters(this);
  m_newSegmentsFilterFactory.exposeParameters(this);
  m_segmentTrackChooserSecondStepFactory.exposeParameters(this);
  m_segmentTrainFilterFactory.exposeParameters(this);
  m_segmentTrackFilterFactory.exposeParameters(this);
}

void SegmentTrackCombinerDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentTrackFilter> ptrSegmentTrackChooserFirstStep = m_segmentTrackChooserFirstStepFactory.create();
  setSegmentTrackChooserFirstStep(std::move(ptrSegmentTrackChooserFirstStep));

  std::unique_ptr<BaseBackgroundSegmentsFilter> ptrBackgroundSegmentsFilter = m_backgroundSegmentsFilterFactory.create();
  setBackgroundSegmentFilter(std::move(ptrBackgroundSegmentsFilter));

  std::unique_ptr<BaseNewSegmentsFilter> ptrNewSegmentsFilter = m_newSegmentsFilterFactory.create();
  setNewSegmentFilter(std::move(ptrNewSegmentsFilter));

  std::unique_ptr<BaseSegmentTrackFilter> ptrSegmentTrackChooserSecondStep = m_segmentTrackChooserSecondStepFactory.create();
  setSegmentTrackChooserSecondStep(std::move(ptrSegmentTrackChooserSecondStep));

  std::unique_ptr<BaseSegmentTrainFilter> ptrSegmentTrainFilter = m_segmentTrainFilterFactory.create();
  setSegmentTrainFilter(std::move(ptrSegmentTrainFilter));

  std::unique_ptr<BaseSegmentInformationListTrackFilter> ptrSegmentTrackFilter = m_segmentTrackFilterFactory.create();
  setSegmentTrackFilter(std::move(ptrSegmentTrackFilter));

  SegmentTrackCombinerImplModule<>::initialize();

  if (getSegmentTrackChooserFirstStep()->needsTruthInformation() or
      getBackgroundSegmentFilter()->needsTruthInformation() or
      getNewSegmentFilter()->needsTruthInformation() or
      getSegmentTrackChooserSecondStep()->needsTruthInformation() or
      getSegmentTrainFilter()->needsTruthInformation() or
      getSegmentTrackFilter()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}


void SegmentTrackCombinerDevModule::event()
{
  if (getSegmentTrackChooserFirstStep()->needsTruthInformation() or
      getBackgroundSegmentFilter()->needsTruthInformation() or
      getNewSegmentFilter()->needsTruthInformation() or
      getSegmentTrackChooserSecondStep()->needsTruthInformation() or
      getSegmentTrainFilter()->needsTruthInformation() or
      getSegmentTrackFilter()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  SegmentTrackCombinerImplModule<>::event();
}
