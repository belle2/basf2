/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/SegmentRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/MCSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/UnionRecordingSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/MVAFeasibleSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/MVARealisticSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/AndFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NotFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseSegmentRelationFilter>;

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
    return std::make_unique<NoneFilter<BaseSegmentRelationFilter>>();
  } else if (filterName == "truth") {
    return std::make_unique<MCSegmentRelationFilter>();
  } else if (filterName == "unionrecording") {
    return std::make_unique<UnionRecordingSegmentRelationFilter>();
  } else if (filterName == "feasible") {
    return std::make_unique<MVAFeasibleSegmentRelationFilter>();
  } else if (filterName == "realistic") {
    return std::make_unique<MVARealisticSegmentRelationFilter>();
  } else if (filterName == "false_positive") {
    std::string defaultFilterName = this->getDefaultFilterName();
    auto defaultFilter = this->create(defaultFilterName);
    std::string truthFilterName = "truth";
    auto truthFilter = this->create(truthFilterName);
    auto notTruthFilter = std::make_unique<NotFilter<BaseSegmentRelationFilter>>(std::move(truthFilter));
    return std::make_unique<AndFilter<BaseSegmentRelationFilter>>(std::move(notTruthFilter),
           std::move(defaultFilter));
  } else if (filterName == "false_negative") {
    std::string defaultFilterName = this->getDefaultFilterName();
    auto defaultFilter = this->create(defaultFilterName);
    std::string truthFilterName = "truth";
    auto truthFilter = this->create(truthFilterName);
    auto notDefaultFilter = std::make_unique<NotFilter<BaseSegmentRelationFilter>>(std::move(defaultFilter));
    return std::make_unique<AndFilter<BaseSegmentRelationFilter>>(std::move(notDefaultFilter),
           std::move(truthFilter));
  } else {
    return Super::create(filterName);
  }
}
