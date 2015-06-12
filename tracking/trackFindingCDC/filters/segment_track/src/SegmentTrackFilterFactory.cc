/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segment_track/SimpleSegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/MCSegmentTrackFilter.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


SegmentTrackFilterFactory::SegmentTrackFilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string SegmentTrackFilterFactory::getFilterPurpose() const
{
  return "Segment pair filter to be used during the construction of segment pairs.";
}

std::string SegmentTrackFilterFactory::getModuleParamPrefix() const
{
  return "SegmentTrack";
}

std::map<std::string, std::string>
SegmentTrackFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"mc", "depricated alias for 'truth'"},
    {"truth", "monte carlo truth"},
    {"none", "no segment track combination is valid"},
    {"simple", "mc free with simple criteria"}
  });
  return filterNames;
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new BaseSegmentTrackFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new MCSegmentTrackFilter());
  } else if (filterName == string("mc")) {
    B2WARNING("Filter name 'mc' is depricated in favour of 'truth'");
    return std::unique_ptr<BaseSegmentTrackFilter>(new MCSegmentTrackFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseSegmentTrackFilter>(new SimpleSegmentTrackFilter());
  } else {
    return Super::create(filterName);
  }
}
