/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_track_chooser/SegmentTrackChooserFactory.h>
#include <tracking/trackFindingCDC/filters/segment_track_chooser/SimpleSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track_chooser/MCSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track_chooser/TMVASegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track_chooser/RecordingSegmentTrackChooser.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
FilterFactory<BaseSegmentTrackChooser>::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"mc", "depricated alias for 'truth'"},
    {"truth", "monte carlo truth"},
    {"none", "no segment track combination is valid"},
    {"simple", "mc free with simple criteria"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
  });
  return filterNames;
}

std::unique_ptr<BaseSegmentTrackChooser>
FilterFactory<BaseSegmentTrackChooser>::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseSegmentTrackChooser>(new BaseSegmentTrackChooser());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseSegmentTrackChooser>(new MCSegmentTrackChooser());
  } else if (filterName == string("mc")) {
    B2WARNING("Filter name 'mc' is depricated in favour of 'truth'");
    return std::unique_ptr<BaseSegmentTrackChooser>(new MCSegmentTrackChooser());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseSegmentTrackChooser>(new SimpleSegmentTrackChooser());
  } else {
    return Super::create(filterName);
  }
}

std::string SegmentTrackChooserFirstStepFactory::getFilterPurpose() const
{
  return "Segment track chooser to be used during the matching of segment track pairs.";
}

std::string SegmentTrackChooserFirstStepFactory::getModuleParamPrefix() const
{
  return "SegmentTrackChooserFirstStep";
}

std::unique_ptr<BaseSegmentTrackChooser>
SegmentTrackChooserFirstStepFactory::create(const std::string& filterName) const
{
  if (filterName == string("tmva")) {
    return std::unique_ptr<BaseSegmentTrackChooser>(new TMVASegmentTrackChooser("SegmentTrackChooserFirstStep"));
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseSegmentTrackChooser>(new RecordingSegmentTrackChooser("SegmentTrackChooserFirstStep"));
  } else {
    return Super::create(filterName);
  }
}

std::string SegmentTrackChooserSecondStepFactory::getFilterPurpose() const
{
  return "Segment track chooser to be used during the combination of segment track pairs.";
}

std::string SegmentTrackChooserSecondStepFactory::getModuleParamPrefix() const
{
  return "SegmentTrackChooserSecondStep";
}

std::unique_ptr<BaseSegmentTrackChooser>
SegmentTrackChooserSecondStepFactory::create(const std::string& filterName) const
{
  if (filterName == string("tmva")) {
    return std::unique_ptr<BaseSegmentTrackChooser>(new TMVASegmentTrackChooser("SegmentTrackChooserSecondStep"));
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseSegmentTrackChooser>(new RecordingSegmentTrackChooser("SegmentTrackChooserSecondStep"));
  } else {
    return Super::create(filterName);
  }
}
