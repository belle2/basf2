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
           "\"simple\" (mc free with simple criteria).",
           string("simple"));

  addParam("SegmentPairNeighborChooser",
           m_param_segmentPairNeighborChooser,
           "Segment pair neighbor chooser to be used during the construction of the graph. "
           "Valid values are: "
           "\"none\" (no neighbor is correct, stops segment generation), "
           "\"all\" (all possible neighbors are valid), "
           "\"mc\" (monte carlo truth), "
           "\"mc_symmetric\" (monte carlo truth and the reversed version are excepted), "
           "\"simple\" (mc free with simple criteria).",
           string("simple"));
}

void TrackFinderCDCSegmentPairAutomatonDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  BaseAxialStereoSegmentPairFilter* ptrAxialStereoSegmentPairFilter = nullptr;
  if (m_param_segmentPairFilter == string("all")) {
    ptrAxialStereoSegmentPairFilter = new AllAxialStereoSegmentPairFilter();
  } else if (m_param_segmentPairFilter == string("mc")) {
    ptrAxialStereoSegmentPairFilter = new MCAxialStereoSegmentPairFilter(false);
  } else if (m_param_segmentPairFilter == string("mc_symmetric")) {
    ptrAxialStereoSegmentPairFilter = new MCAxialStereoSegmentPairFilter(true);
  } else if (m_param_segmentPairFilter == string("simple")) {
    ptrAxialStereoSegmentPairFilter = new SimpleAxialStereoSegmentPairFilter();
  } else {
    B2ERROR("Unrecognised AxialStereoSegmentPairFilter option " << m_param_segmentPairFilter << ". Allowed values are \"all\", \"mc\", \"mc_symmetric\" and \"simple\".");
  }

  if (ptrAxialStereoSegmentPairFilter) {
    // Takes ownership
    setAxialStereoSegmentPairFilter(ptrAxialStereoSegmentPairFilter);
  }

  BaseAxialStereoSegmentPairNeighborChooser* ptrAxialStereoSegmentPairNeighborChooser = nullptr;
  if (m_param_segmentPairNeighborChooser == string("none")) {
    ptrAxialStereoSegmentPairNeighborChooser = new BaseAxialStereoSegmentPairNeighborChooser();
  } else if (m_param_segmentPairNeighborChooser == string("all")) {
    ptrAxialStereoSegmentPairNeighborChooser = new AllAxialStereoSegmentPairNeighborChooser();
  } else if (m_param_segmentPairNeighborChooser == string("mc")) {
    ptrAxialStereoSegmentPairNeighborChooser = new MCAxialStereoSegmentPairNeighborChooser(false);
  } else if (m_param_segmentPairNeighborChooser == string("mc_symmetric")) {
    ptrAxialStereoSegmentPairNeighborChooser = new MCAxialStereoSegmentPairNeighborChooser(true);
  } else if (m_param_segmentPairNeighborChooser == string("simple")) {
    ptrAxialStereoSegmentPairNeighborChooser = new SimpleAxialStereoSegmentPairNeighborChooser();
  } else {
    B2ERROR("Unrecognised AxialStereoSegmentPairNeighborChooser option " << m_param_segmentPairNeighborChooser << ". Allowed values are \"none\", \"all\", \"mc\", \"mc_symmetric\" and \"simple\".");
  }
  if (ptrAxialStereoSegmentPairNeighborChooser) {
    // Takes ownership
    setAxialStereoSegmentPairNeighborChooser(ptrAxialStereoSegmentPairNeighborChooser);
  }

  TrackFinderCDCSegmentPairAutomatonImplModule<>::initialize();

  if ((m_param_segmentPairFilter == string("mc")) or
      (m_param_segmentPairFilter == string("mc_symmetric")) or
      (m_param_segmentPairNeighborChooser == string("mc")) or
      (m_param_segmentPairNeighborChooser == string("mc_symmetric"))) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }

}


void TrackFinderCDCSegmentPairAutomatonDevModule::event()
{
  if ((m_param_segmentPairFilter == string("mc")) or
      (m_param_segmentPairFilter == string("mc_symmetric")) or
      (m_param_segmentPairNeighborChooser == string("mc")) or
      (m_param_segmentPairNeighborChooser == string("mc_symmetric"))) {

    CDCMCManager::getInstance().fill();
  }

  TrackFinderCDCSegmentPairAutomatonImplModule<>::event();
}
