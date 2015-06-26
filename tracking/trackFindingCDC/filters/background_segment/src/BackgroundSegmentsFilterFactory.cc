/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/background_segment/BackgroundSegmentsFilterFactory.h>
#include <tracking/trackFindingCDC/filters/background_segment/SimpleBackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/background_segment/MCBackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/background_segment/AllBackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/background_segment/TMVABackgroundSegmentsChooser.h>
#include <tracking/trackFindingCDC/filters/background_segment/RecordingBackgroundSegmentsFilter.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

std::map<std::string, std::string>
FilterFactory<BaseBackgroundSegmentsFilter>::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"mc", "depricated alias for 'truth'"},
    {"truth", "monte carlo truth"},
    {"all", "set all segments as good"},
    {"none", "no segment track combination is valid"},
    {"simple", "mc free with simple criteria"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
  });
  return filterNames;
}

std::unique_ptr<BaseBackgroundSegmentsFilter>
FilterFactory<BaseBackgroundSegmentsFilter>::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new BaseBackgroundSegmentsFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new MCBackgroundSegmentsFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new AllBackgroundSegmentsFilter());
  } else if (filterName == string("mc")) {
    B2WARNING("Filter name 'mc' is depricated in favour of 'truth'");
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new MCBackgroundSegmentsFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new SimpleBackgroundSegmentsFilter());
  } else if (filterName == string("tmva")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new TMVABackgroundSegmentsFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseBackgroundSegmentsFilter>(new RecordingBackgroundSegmentsFilter());
  } else {
    return Super::create(filterName);
  }
}

std::string FilterFactory<BaseBackgroundSegmentsFilter>::getFilterPurpose() const
{
  return "Segment background finder.";
}

std::string FilterFactory<BaseBackgroundSegmentsFilter>::getModuleParamPrefix() const
{
  return "BackgroundSegments";
}
