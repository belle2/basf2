/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SegmentPairRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/AllSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/MCSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SimpleSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/UnionRecordingSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/MVARealisticSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SegmentPairRelationFilterFactory::SegmentPairRelationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentPairRelationFilterFactory::getIdentifier() const
{
  return "SegmentPairRelation";
}

std::string SegmentPairRelationFilterFactory::getFilterPurpose() const
{
  return "Segment pair relation filter to construct of a segment pair network";
}

std::map<std::string, std::string>
SegmentPairRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment pair relation is valid, stop at segment pair creation"},
    {"all", "all segment pair relations are valid"},
    {"truth", "segment pair relations from monte carlo truth"},
    {"simple", "mc free with simple criteria"},
    {"unionrecording", "record multiple choosable variable set"},
    {"realistic", "mc free using a mva method"},
  };
}

std::unique_ptr<BaseSegmentPairRelationFilter>
SegmentPairRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<NoneFilter<BaseSegmentPairRelationFilter>>();
  } else if (filterName == "all") {
    return makeUnique<AllSegmentPairRelationFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentPairRelationFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleSegmentPairRelationFilter>();
  } else if (filterName == "unionrecording") {
    return makeUnique<UnionRecordingSegmentPairRelationFilter>();
  } else if (filterName == "realistic") {
    return makeUnique<MVARealisticSegmentPairRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
