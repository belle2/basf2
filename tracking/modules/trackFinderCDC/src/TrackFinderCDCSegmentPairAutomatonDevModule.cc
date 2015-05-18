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

#include <tracking/trackFindingCDC/filters/axial_stereo/SegmentPairFilters.h>
#include <tracking/trackFindingCDC/filters/axial_stereo_axial_stereo/SegmentPairNeighborChoosers.h>

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
  std::unique_ptr<BaseSegmentPairFilter>
  ptrSegmentPairFilter(new BaseSegmentPairFilter());

  if (m_param_segmentPairFilter == string("all")) {
    ptrSegmentPairFilter.reset(new AllSegmentPairFilter());
  } else if (m_param_segmentPairFilter == string("mc")) {
    ptrSegmentPairFilter.reset(new MCSegmentPairFilter(false));
  } else if (m_param_segmentPairFilter == string("mc_symmetric")) {
    ptrSegmentPairFilter.reset(new MCSegmentPairFilter(true));
  } else if (m_param_segmentPairFilter == string("simple")) {
    ptrSegmentPairFilter.reset(new SimpleSegmentPairFilter());
  } else if (m_param_segmentPairFilter == string("recording")) {
    ptrSegmentPairFilter.reset(new RecordingSegmentPairFilter());
  } else {
    B2ERROR("Unrecognised SegmentPairFilter option " <<
            m_param_segmentPairFilter <<
            ". Allowed values are \"all\", \"mc\", \"mc_symmetric\", \"simple\" and \"recording\".");
  }

  setSegmentPairFilter(std::move(ptrSegmentPairFilter));
  getSegmentPairFilter()->setParameters(m_param_segmentPairFilterParameters);

  std::unique_ptr<BaseSegmentPairNeighborChooser>
  ptrSegmentPairNeighborChooser(new BaseSegmentPairNeighborChooser());

  if (m_param_segmentPairNeighborChooser == string("none")) {
    ptrSegmentPairNeighborChooser.reset(new BaseSegmentPairNeighborChooser());
  } else if (m_param_segmentPairNeighborChooser == string("all")) {
    ptrSegmentPairNeighborChooser.reset(new AllSegmentPairNeighborChooser());
  } else if (m_param_segmentPairNeighborChooser == string("mc")) {
    ptrSegmentPairNeighborChooser.reset(new MCSegmentPairNeighborChooser(false));
  } else if (m_param_segmentPairNeighborChooser == string("mc_symmetric")) {
    ptrSegmentPairNeighborChooser.reset(new MCSegmentPairNeighborChooser(true));
  } else if (m_param_segmentPairNeighborChooser == string("simple")) {
    ptrSegmentPairNeighborChooser.reset(new SimpleSegmentPairNeighborChooser());
  } else {
    B2ERROR("Unrecognised SegmentPairNeighborChooser option " <<
            m_param_segmentPairNeighborChooser <<
            ". Allowed values are \"none\", \"all\", \"mc\", \"mc_symmetric\" and \"simple\".");
  }

  setSegmentPairNeighborChooser(std::move(ptrSegmentPairNeighborChooser));
  getSegmentPairNeighborChooser()->setParameters(m_param_segmentPairNeighborChooserParameters);

  TrackFinderCDCSegmentPairAutomatonImplModule<>::initialize();

  if (getSegmentPairFilter()->needsTruthInformation() or
      getSegmentPairNeighborChooser()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}


void TrackFinderCDCSegmentPairAutomatonDevModule::event()
{
  if (getSegmentPairFilter()->needsTruthInformation() or
      getSegmentPairNeighborChooser()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  TrackFinderCDCSegmentPairAutomatonImplModule<>::event();
}
