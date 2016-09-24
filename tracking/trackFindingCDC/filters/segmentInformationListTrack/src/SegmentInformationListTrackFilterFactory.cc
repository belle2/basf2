/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentInformationListTrack/SegmentInformationListTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentInformationListTrack/SimpleSegmentInformationListTrackFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
SegmentInformationListTrackFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"truth", "monte carlo truth"},
    {"none", "no segment track combination is valid"},
    {"simple", "mc free with simple criteria"},
    {"tmva", "test using tmva methods"},
    {"recording", "Record to a ttree"}

  });
  return filterNames;
}

std::unique_ptr<BaseSegmentInformationListTrackFilter>
SegmentInformationListTrackFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::unique_ptr<BaseSegmentInformationListTrackFilter>(new BaseSegmentInformationListTrackFilter());
  } else if (filterName == "truth") {
    return std::unique_ptr<BaseSegmentInformationListTrackFilter>(new MCSegmentInformationListTrackFilter());
  } else if (filterName == "simple") {
    return std::unique_ptr<BaseSegmentInformationListTrackFilter>(new SimpleSegmentInformationListTrackFilter());
  } else if (filterName == "tmva") {
    return std::unique_ptr<BaseSegmentInformationListTrackFilter>(new TMVASegmentInformationListTrackFilter());
  } else if (filterName == "recording") {
    return std::unique_ptr<BaseSegmentInformationListTrackFilter>(new RecordingSegmentInformationListTrackFilter());
  } else {
    return Super::create(filterName);
  }
}
