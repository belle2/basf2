/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/backgroundSegment/BackgroundSegmentsFilterFactory.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
BackgroundSegmentsFilterFactory::getValidFilterNamesAndDescriptions() const
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

std::unique_ptr<BaseBackgroundSegmentsFilter>
BackgroundSegmentsFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new BaseBackgroundSegmentsFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new MCBackgroundSegmentsFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new AllBackgroundSegmentsFilter());
  } else if (filterName == string("tmva")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new TMVABackgroundSegmentsFilter("BackgroundSegmentsFilter"));
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new RecordingBackgroundSegmentsFilter("BackgroundSegmentsFilter.root"));
  } else {
    return Super::create(filterName);
  }
}

