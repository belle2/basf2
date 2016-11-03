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

using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
SegmentTrackFilterFactory::getValidFilterNamesAndDescriptions() const
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
SegmentTrackFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseSegmentTrackFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentTrackFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleSegmentTrackFilter>();
  } else {
    return Super::create(filterName);
  }
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterFirstStepFactory::create(const std::string& filterName) const
{
  if (filterName == "tmva") {
    return makeUnique<TMVASegmentTrackFilter>("SegmentTrackFilterFirstStep");
  } else if (filterName == "recording") {
    return makeUnique<RecordingSegmentTrackFilter>("SegmentTrackFilterFirstStep.root");
  } else {
    return Super::create(filterName);
  }
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterSecondStepFactory::create(const std::string& filterName) const
{
  if (filterName == "tmva") {
    return makeUnique<TMVASegmentTrackFilter>("SegmentTrackFilterSecondStep");
  } else if (filterName ==  "recording") {
    return makeUnique<RecordingSegmentTrackFilter>("SegmentTrackFilterSecondStep.root");
  } else {
    return Super::create(filterName);
  }
}
