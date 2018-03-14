/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SegmentTripleRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segmentTripleRelation/BaseSegmentTripleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/AllSegmentTripleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/MCSegmentTripleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SimpleSegmentTripleRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseSegmentTripleRelationFilter>;

SegmentTripleRelationFilterFactory::SegmentTripleRelationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentTripleRelationFilterFactory::getIdentifier() const
{
  return "SegmentTripleRelation";
}

std::string SegmentTripleRelationFilterFactory::getFilterPurpose() const
{
  return "Segment triple relation filter to construct of a segment triple network";
}

std::map<std::string, std::string>
SegmentTripleRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment triple relation is valid, stop at segment triple creation"},
    {"all", "all segment triple relations are valid"},
    {"truth", "segment triple relations from monte carlo truth"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<BaseSegmentTripleRelationFilter>
SegmentTripleRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseSegmentTripleRelationFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllSegmentTripleRelationFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCSegmentTripleRelationFilter>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleSegmentTripleRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
