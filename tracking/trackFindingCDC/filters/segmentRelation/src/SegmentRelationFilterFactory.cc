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

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentRelationFilterFactory::SegmentRelationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentRelationFilterFactory::getIdentifier() const
{
  return "SegmentRelation";
}

std::string SegmentRelationFilterFactory::getFilterPurpose() const
{
  return "Segment relation filter to construct a segment network for in super cluster merging";
}

std::map<std::string, std::string>
SegmentRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment relation is valid, stop at segment creation."},
    {"truth", "segment relations from monte carlo truth"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"feasible", "check if the segment relation is feasible"},
    {"realistic", "check if the segment relation is a good combination"},
  };
}

std::unique_ptr<BaseSegmentRelationFilter >
SegmentRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseSegmentRelationFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentRelationFilter>();
  } else if (filterName == "unionrecording") {
    return makeUnique<UnionRecordingSegmentRelationFilter>();
  } else if (filterName == "feasible") {
    return makeUnique<MVAFeasibleSegmentRelationFilter>();
  } else if (filterName == "realistic") {
    return makeUnique<MVARealisticSegmentRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
