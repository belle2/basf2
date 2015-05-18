/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCSegmentPairAutomatonDevModule.h>

#include <tracking/trackFindingCDC/filters/axial_stereo/AxialStereoSegmentPairFilters.h>
#include <tracking/trackFindingCDC/filters/axial_stereo_axial_stereo/AxialStereoSegmentPairNeighborChoosers.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCSegmentPairAutomatonDev);

TrackFinderCDCSegmentPairAutomatonDevModule::TrackFinderCDCSegmentPairAutomatonDevModule() :
  TrackFinderCDCSegmentPairAutomatonImplModule<>(c_None),
  m_param_segmentPairFilter("simple"),
  m_param_segmentPairNeighborChooser("simple")
{
  setDescription("Versatile module with adjustable filters for track generation.");

  addParam("SegmentPairFilter",
           m_param_segmentPairFilter,
           "Segment pair filter to be used during the construction of segment pairs. "
           "Valid values are: "
           "\"all\" (all segment pairs are valid), "
           "\"mc\" (monte carlo truth), "
           "\"mc_symmetric\" (monte carlo truth and their mirror image), "
           "\"simple\" (mc free with simple criteria), "
           "\"recording\" (record the encountered instances of segment pairs "
           "including truth information).",
           string(m_param_segmentPairFilter));

  addParam("SegmentPairFilterParameters",
           m_param_segmentPairFilterParameters,
           "Key - Value pairs depending on the segment pair filter",
           map<string, string>());

  addParam("SegmentPairNeighborChooser",
           m_param_segmentPairNeighborChooser,
           "Segment pair neighbor chooser to be used during the construction of the graph. "
           "Valid values are: "
           "\"none\" (no neighbor is correct, stops segment generation), "
           "\"all\" (all possible neighbors are valid), "
           "\"mc\" (monte carlo truth), "
           "\"mc_symmetric\" (monte carlo truth and the reversed version are excepted), "
           "\"simple\" (mc free with simple criteria).",
           string(m_param_segmentPairNeighborChooser));

  addParam("SegmentPairNeighborChooserParameters",
           m_param_segmentPairNeighborChooserParameters,
           "Key - Value pairs depending on the segment pair neighbor chooser",
           map<string, string>());
}

void TrackFinderCDCSegmentPairAutomatonDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseAxialStereoSegmentPairFilter>
  ptrAxialStereoSegmentPairFilter(new BaseAxialStereoSegmentPairFilter());

  if (m_param_segmentPairFilter == string("all")) {
    ptrAxialStereoSegmentPairFilter.reset(new AllAxialStereoSegmentPairFilter());
  } else if (m_param_segmentPairFilter == string("mc")) {
    ptrAxialStereoSegmentPairFilter.reset(new MCAxialStereoSegmentPairFilter(false));
  } else if (m_param_segmentPairFilter == string("mc_symmetric")) {
    ptrAxialStereoSegmentPairFilter.reset(new MCAxialStereoSegmentPairFilter(true));
  } else if (m_param_segmentPairFilter == string("simple")) {
    ptrAxialStereoSegmentPairFilter.reset(new SimpleAxialStereoSegmentPairFilter());
  } else if (m_param_segmentPairFilter == string("recording")) {
    ptrAxialStereoSegmentPairFilter.reset(new RecordingAxialStereoSegmentPairFilter());
  } else {
    B2ERROR("Unrecognised AxialStereoSegmentPairFilter option " <<
            m_param_segmentPairFilter <<
            ". Allowed values are \"all\", \"mc\", \"mc_symmetric\", \"simple\" and \"recording\".");
  }

  setAxialStereoSegmentPairFilter(std::move(ptrAxialStereoSegmentPairFilter));
  getAxialStereoSegmentPairFilter()->setParameters(m_param_segmentPairFilterParameters);

  std::unique_ptr<BaseAxialStereoSegmentPairNeighborChooser>
  ptrAxialStereoSegmentPairNeighborChooser(new BaseAxialStereoSegmentPairNeighborChooser());

  if (m_param_segmentPairNeighborChooser == string("none")) {
    ptrAxialStereoSegmentPairNeighborChooser.reset(new BaseAxialStereoSegmentPairNeighborChooser());
  } else if (m_param_segmentPairNeighborChooser == string("all")) {
    ptrAxialStereoSegmentPairNeighborChooser.reset(new AllAxialStereoSegmentPairNeighborChooser());
  } else if (m_param_segmentPairNeighborChooser == string("mc")) {
    ptrAxialStereoSegmentPairNeighborChooser.reset(new MCAxialStereoSegmentPairNeighborChooser(false));
  } else if (m_param_segmentPairNeighborChooser == string("mc_symmetric")) {
    ptrAxialStereoSegmentPairNeighborChooser.reset(new MCAxialStereoSegmentPairNeighborChooser(true));
  } else if (m_param_segmentPairNeighborChooser == string("simple")) {
    ptrAxialStereoSegmentPairNeighborChooser.reset(new SimpleAxialStereoSegmentPairNeighborChooser());
  } else {
    B2ERROR("Unrecognised AxialStereoSegmentPairNeighborChooser option " <<
            m_param_segmentPairNeighborChooser <<
            ". Allowed values are \"none\", \"all\", \"mc\", \"mc_symmetric\" and \"simple\".");
  }

  setAxialStereoSegmentPairNeighborChooser(std::move(ptrAxialStereoSegmentPairNeighborChooser));
  getAxialStereoSegmentPairNeighborChooser()->setParameters(m_param_segmentPairNeighborChooserParameters);

  TrackFinderCDCSegmentPairAutomatonImplModule<>::initialize();

  if (getAxialStereoSegmentPairFilter()->needsTruthInformation() or
      getAxialStereoSegmentPairNeighborChooser()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}


void TrackFinderCDCSegmentPairAutomatonDevModule::event()
{
  if (getAxialStereoSegmentPairFilter()->needsTruthInformation() or
      getAxialStereoSegmentPairNeighborChooser()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  TrackFinderCDCSegmentPairAutomatonImplModule<>::event();
}
