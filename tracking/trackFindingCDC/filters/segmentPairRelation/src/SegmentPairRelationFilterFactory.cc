/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SegmentPairRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/AllSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/MCSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SimpleSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/UnionRecordingSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/MVARealisticSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseSegmentPairRelationFilter>;

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
    return std::make_unique<NoneFilter<BaseSegmentPairRelationFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllSegmentPairRelationFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCSegmentPairRelationFilter>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleSegmentPairRelationFilter>();
  } else if (filterName == "unionrecording") {
    return std::make_unique<UnionRecordingSegmentPairRelationFilter>();
  } else if (filterName == "realistic") {
    return std::make_unique<MVARealisticSegmentPairRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
