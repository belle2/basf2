/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/SegmentRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/SegmentRelationFilters.h>

using namespace Belle2;
using namespace TrackFindingCDC;


SegmentRelationFilterFactory::SegmentRelationFilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string SegmentRelationFilterFactory::getFilterPurpose() const
{
  return "Segment relation filter to be used during the construction of the segment network for in super layer merginig.";
}

std::string SegmentRelationFilterFactory::getModuleParamPrefix() const
{
  return "SegmentRelation";
}

std::map<std::string, std::string>
SegmentRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"none", "no segment relation is valid, stop at segment creation."},
    {"truth", "segment relations from monte carlo truth"},
    {"unionrecording", "record many multiple choosable variable set"},
  });

  return filterNames;
}

std::unique_ptr<BaseSegmentRelationFilter >
SegmentRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::unique_ptr<BaseSegmentRelationFilter >(new BaseSegmentRelationFilter());
  } else if (filterName == "truth") {
    return std::unique_ptr<BaseSegmentRelationFilter >(new MCSegmentRelationFilter());
  } else if (filterName == "unionrecording") {
    return std::unique_ptr<BaseSegmentRelationFilter >(new UnionRecordingSegmentRelationFilter());
  } else {
    return Super::create(filterName);
  }
}
