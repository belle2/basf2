/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_pair/SegmentPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segment_pair/SegmentPairFilters.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FilterFactory<Filter<CDCSegmentPair> >::FilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string FilterFactory<Filter<CDCSegmentPair> >::getFilterPurpose() const
{
  return "Segment pair filter to be used during the construction of segment pairs.";
}

std::string FilterFactory<Filter<CDCSegmentPair> >::getModuleParamPrefix() const
{
  return "SegmentPair";
}

std::map<std::string, std::string>
FilterFactory<Filter<CDCSegmentPair> >::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"all", "all segment pairs are valid"},
    {"mc", "depricated alias for 'truth'"},
    {"truth", "monte carlo truth"},
    {"none", "no segment pair is valid"},
    {"recording", "record the encountered instances of segment pairs"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"simple", "mc free with simple criteria"},
  });
  return filterNames;
}

std::unique_ptr<Filter<CDCSegmentPair>>
                                     FilterFactory<Filter<CDCSegmentPair> >::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new BaseSegmentPairFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new AllSegmentPairFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new MCSegmentPairFilter());
  } else if (filterName == string("mc")) {
    B2WARNING("Filter name 'mc' is depricated in favour of 'truth'");
    return std::unique_ptr<Filter<CDCSegmentPair> >(new MCSegmentPairFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new SimpleSegmentPairFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new RecordingSegmentPairFilter());
  } else if (filterName == string("unionrecording")) {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new UnionRecordingSegmentPairFilter());
  } else {
    return Super::create(filterName);
  }
}
