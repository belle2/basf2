/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/StereoSegmentTrackMatcherDevModule.h>

#include <tracking/trackFindingCDC/filters/segment_track/RecordingSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/TMVASegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/MCSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/AllSegmentTrackChooser.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(StereoSegmentTrackMatcherDev);

StereoSegmentTrackMatcherDevModule::StereoSegmentTrackMatcherDevModule() :
  StereoSegmentTrackMatcherModuleImpl<>(),
  m_param_segmentTrackChooser("simple")
{
  setDescription("Versatile module with adjustable filters for stereo segment track matching.");

  addParam("SegmentTrackChooser",
           m_param_segmentTrackChooser, "", string("simple"));

  addParam("SegmentTrackChooserParameters",
           m_param_segmentTrackChooserParameters,
           "Key - Value pairs depending on the filter",
           map<string, string>());
}

void StereoSegmentTrackMatcherDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentTrackChooser> ptrSegmentTrackChooser(new BaseSegmentTrackChooser());

  if (m_param_segmentTrackChooser == string("none")) {
    ptrSegmentTrackChooser.reset(new BaseSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("simple")) {
    ptrSegmentTrackChooser.reset(new SimpleSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("mc")) {
    ptrSegmentTrackChooser.reset(new MCSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("recording")) {
    ptrSegmentTrackChooser.reset(new RecordingSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("tmva")) {
    ptrSegmentTrackChooser.reset(new TMVASegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("all")) {
    ptrSegmentTrackChooser.reset(new AllSegmentTrackChooser());
  } else {
    B2ERROR("Unrecognised SegmentTrackChooser option " << m_param_segmentTrackChooser);
  }

  // Takes ownership
  setSegmentTrackChooser(std::move(ptrSegmentTrackChooser));
  getSegmentTrackChooser()->setParameters(m_param_segmentTrackChooserParameters);

  StereoSegmentTrackMatcherModuleImpl<>::initialize();

  if (getSegmentTrackChooser()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}


void StereoSegmentTrackMatcherDevModule::event()
{
  if (getSegmentTrackChooser()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  StereoSegmentTrackMatcherModuleImpl<>::event();
}
