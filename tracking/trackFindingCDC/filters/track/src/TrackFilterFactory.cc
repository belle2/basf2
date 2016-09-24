/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/TrackFilterFactory.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
TrackFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"truth", "monte carlo truth"},
    {"all", "set all segments as good"},
    {"none", "no segment track combination is valid"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
  });
  return filterNames;
}

std::unique_ptr<BaseTrackFilter>
TrackFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::unique_ptr<BaseTrackFilter>(new BaseTrackFilter());
  } else if (filterName == "truth") {
    return std::unique_ptr<BaseTrackFilter>(new MCTrackFilter());
  } else if (filterName == "all") {
    return std::unique_ptr<BaseTrackFilter>(new AllTrackFilter());
  } else if (filterName == "tmva") {
    return std::unique_ptr<BaseTrackFilter>(new TMVATrackFilter("TrackFilter"));
  } else if (filterName == "recording") {
    return std::unique_ptr<BaseTrackFilter>(new RecordingTrackFilter("TrackFilter.root"));
  } else {
    return Super::create(filterName);
  }
}
