/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/newSegment/NewSegmentsFilterFactory.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
NewSegmentsFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"truth", "monte carlo truth"},
    {"none", "no segment track combination is valid"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
  });
  return filterNames;
}

std::unique_ptr<BaseNewSegmentsFilter>
NewSegmentsFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::unique_ptr<BaseNewSegmentsFilter>(new BaseNewSegmentsFilter());
  } else if (filterName == "truth") {
    return std::unique_ptr<BaseNewSegmentsFilter>(new MCNewSegmentsFilter());
  } else if (filterName == "tmva") {
    return std::unique_ptr<BaseNewSegmentsFilter>(new TMVANewSegmentsFilter("NewSegmentsFilter"));
  } else if (filterName == "recording") {
    return std::unique_ptr<BaseNewSegmentsFilter>(new RecordingNewSegmentsFilter("NewSegmentsFilter.root"));
  } else {
    return Super::create(filterName);
  }
}

std::string NewSegmentsFilterFactory::getFilterPurpose() const
{
  return "Segment background finder.";
}

std::string NewSegmentsFilterFactory::getModuleParamPrefix() const
{
  return "NewSegments";
}
