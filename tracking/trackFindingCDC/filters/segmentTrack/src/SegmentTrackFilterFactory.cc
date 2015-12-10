/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentTrack/SimpleSegmentTrackFilter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
FilterFactory<BaseSegmentTrackFilter>::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"truth", "monte carlo truth"},
    {"none", "no segment track combination is valid"},
    {"simple", "mc free with simple criteria"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
  });
  return filterNames;
}

std::unique_ptr<BaseSegmentTrackFilter>
FilterFactory<BaseSegmentTrackFilter>::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new BaseSegmentTrackFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new MCSegmentTrackFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new SimpleSegmentTrackFilter());
  } else {
    return Super::create(filterName);
  }
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterFirstStepFactory::create(const std::string& filterName) const
{
  if (filterName == string("tmva")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new TMVASegmentTrackFilter("SegmentTrackFilterFirstStep"));
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new RecordingSegmentTrackFilter("SegmentTrackFilterFirstStep.root"));
  } else {
    return Super::create(filterName);
  }
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterSecondStepFactory::create(const std::string& filterName) const
{
  if (filterName == string("tmva")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new TMVASegmentTrackFilter("SegmentTrackFilterSecondStep"));
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new RecordingSegmentTrackFilter("SegmentTrackFilterSecondStep.root"));
  } else {
    return Super::create(filterName);
  }
}
