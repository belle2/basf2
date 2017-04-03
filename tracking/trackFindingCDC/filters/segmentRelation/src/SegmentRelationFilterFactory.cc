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

#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/MCSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/UnionRecordingSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/MVAFeasibleSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/MVARealisticSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>

#include <tracking/trackFindingCDC/filters/base/AndFilter.h>
#include <tracking/trackFindingCDC/filters/base/NotFilter.h>

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
    {"false_positive", "accepts the instances that are really false but are accepted by the default filter"},
    {"false_negative", "accepts the instances that are really true but are rejected by the default filter"},
  };
}

std::unique_ptr<BaseSegmentRelationFilter >
SegmentRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<NoneFilter<BaseSegmentRelationFilter>>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentRelationFilter>();
  } else if (filterName == "unionrecording") {
    return makeUnique<UnionRecordingSegmentRelationFilter>();
  } else if (filterName == "feasible") {
    return makeUnique<MVAFeasibleSegmentRelationFilter>();
  } else if (filterName == "realistic") {
    return makeUnique<MVARealisticSegmentRelationFilter>();
  } else if (filterName == "false_positive") {
    std::string defaultFilterName = this->getDefaultFilterName();
    auto defaultFilter = this->create(defaultFilterName);
    std::string truthFilterName = "truth";
    auto truthFilter = this->create(truthFilterName);
    auto notTruthFilter = makeUnique<NotFilter<BaseSegmentRelationFilter>>(std::move(truthFilter));
    return makeUnique<AndFilter<BaseSegmentRelationFilter>>(std::move(notTruthFilter),
                                                            std::move(defaultFilter));
  } else if (filterName == "false_negative") {
    std::string defaultFilterName = this->getDefaultFilterName();
    auto defaultFilter = this->create(defaultFilterName);
    std::string truthFilterName = "truth";
    auto truthFilter = this->create(truthFilterName);
    auto notDefaultFilter = makeUnique<NotFilter<BaseSegmentRelationFilter>>(std::move(defaultFilter));
    return makeUnique<AndFilter<BaseSegmentRelationFilter>>(std::move(notDefaultFilter),
                                                            std::move(truthFilter));
  } else {
    return Super::create(filterName);
  }
}
