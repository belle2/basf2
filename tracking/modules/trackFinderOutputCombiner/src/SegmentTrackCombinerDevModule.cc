/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombinerDevModule.h>
#include <tracking/trackFindingCDC/filters/segment_track/RecordingSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/TMVASegmentTrackChooser.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombinerDev);

SegmentTrackCombinerDevModule::SegmentTrackCombinerDevModule() :
  SegmentTrackCombinerImplModule<>(),
  m_param_segmentTrackChooser("simple"),
  m_param_segmentTrainFilter("simple"),
  m_param_segmentTrackFilter("simple")
{
  setDescription("Versatile module with adjustable filters for segment track combination.");

  addParam("SegmentTrackChooser",
           m_param_segmentTrackChooser, "", string("simple"));

  addParam("SegmentTrackChooserParameters",
           m_param_segmentTrackChooserParameters,
           "Key - Value pairs depending on the filter",
           map<string, string>());

  addParam("SegmentTrainFilter",
           m_param_segmentTrainFilter, "", string("simple"));

  addParam("SegmentTrackFilter",
           m_param_segmentTrackFilter, "", string("simple"));
}

void SegmentTrackCombinerDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentTrackChooser> ptrSegmentTrackChooser(new BaseSegmentTrackChooser());

  if (m_param_segmentTrackChooser == string("simple")) {
    ptrSegmentTrackChooser.reset(new SimpleSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("recording")) {
    ptrSegmentTrackChooser.reset(new RecordingSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("tmva")) {
    ptrSegmentTrackChooser.reset(new TMVASegmentTrackChooser());
  } else {
    B2ERROR("Unrecognised SegmentTrackChooser option " << m_param_segmentTrackChooser);
  }

  // Takes ownership
  setSegmentTrackChooser(std::move(ptrSegmentTrackChooser));
  getSegmentTrackChooser()->setParameters(m_param_segmentTrackChooserParameters);

  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentTrainFilter> ptrSegmentTrainFilter(new BaseSegmentTrainFilter());;

  if (m_param_segmentTrainFilter == string("simple")) {
    // TODO
    // ptrSegmentTrainFilter.reset(new SimpleSegmentTrainFilter());
  } else {
    B2ERROR("Unrecognised SegmentTrainFilter option " << m_param_segmentTrainFilter);
  }

  // Takes ownership
  setSegmentTrainFilter(std::move(ptrSegmentTrainFilter));

  std::unique_ptr<BaseSegmentTrackFilter> ptrSegmentTrackFilter(new BaseSegmentTrackFilter());
  if (m_param_segmentTrackFilter == string("simple")) {
    // TODO
    // ptrSegmentTrackFilter.reset(new SimpleSegmentTrackFilter());
  } else {
    B2ERROR("Unrecognised SegmentTrackFilter option " << m_param_segmentTrackFilter <<
            ". Allowed values are \"none\", \"all\", \"mc\", and \"simple\".");
  }

  // Takes ownership
  setSegmentTrackFilter(std::move(ptrSegmentTrackFilter));

  SegmentTrackCombinerImplModule<>::initialize();

  if (getSegmentTrackChooser()->needsTruthInformation() or
      getSegmentTrainFilter()->needsTruthInformation() or
      getSegmentTrackFilter()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}


void SegmentTrackCombinerDevModule::event()
{
  if (getSegmentTrackChooser()->needsTruthInformation() or
      getSegmentTrainFilter()->needsTruthInformation() or
      getSegmentTrackFilter()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  SegmentTrackCombinerImplModule<>::event();
}
