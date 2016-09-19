/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/SegmentPairFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentPair/SegmentPairFilters.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentPairFilterFactory::SegmentPairFilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string SegmentPairFilterFactory::getFilterPurpose() const
{
  return "Segment pair filter to be used during the construction of segment pairs.";
}

std::string SegmentPairFilterFactory::getModuleParamPrefix() const
{
  return "SegmentPair";
}

std::map<std::string, std::string>
SegmentPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"all", "all segment pairs are valid"},
    {"truth", "monte carlo truth"},
    {"none", "no segment pair is valid"},
    {"feasible", "multivariat method based on variables of the first and last hit in each segment meant as precut"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"simple", "mc free with simple criteria"},
    {"fitless", "mc free with simple criteria without the common fit"},
    {"realistic", "realistic filter using a common fit and combination of all information with an mva"},
  });
  return filterNames;
}

std::unique_ptr<Filter<CDCSegmentPair> >
SegmentPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new BaseSegmentPairFilter());
  } else if (filterName == "all") {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new AllSegmentPairFilter());
  } else if (filterName == "truth") {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new MCSegmentPairFilter());
  } else if (filterName == "feasible") {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new MVAFeasibleSegmentPairFilter());
  } else if (filterName == "simple") {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new SimpleSegmentPairFilter());
  } else if (filterName == "fitless") {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new FitlessSegmentPairFilter());
  } else if (filterName == "realistic") {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new MVARealisticSegmentPairFilter());
  } else if (filterName == "unionrecording") {
    return std::unique_ptr<Filter<CDCSegmentPair> >(new UnionRecordingSegmentPairFilter());
  } else {
    return Super::create(filterName);
  }
}
