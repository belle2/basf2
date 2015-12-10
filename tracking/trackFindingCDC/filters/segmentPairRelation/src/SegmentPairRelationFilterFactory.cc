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
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SegmentPairRelationFilters.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


SegmentPairRelationFilterFactory::SegmentPairRelationFilterFactory(const std::string& defaultFilterName) :
  FilterFactory<BaseSegmentPairRelationFilter>(defaultFilterName)
{
}

std::string SegmentPairRelationFilterFactory::getFilterPurpose() const
{
  return "Segment pair relation filter to be used during the construction of a segment pair network.";
}

std::string SegmentPairRelationFilterFactory::getModuleParamPrefix() const
{
  return "SegmentPairRelation";
}

std::map<std::string, std::string>
SegmentPairRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all segment pair relations are valid"},
    {"truth", "segment pair relations from monte carlo truth"},
    {"none", "no segment pair relation is valid, stop at segment pair creation"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<BaseSegmentPairRelationFilter>
SegmentPairRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseSegmentPairRelationFilter>(new BaseSegmentPairRelationFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseSegmentPairRelationFilter>(new AllSegmentPairRelationFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseSegmentPairRelationFilter>(new MCSegmentPairRelationFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseSegmentPairRelationFilter>(new SimpleSegmentPairRelationFilter());
    // } else if (filterName == string("recording")) {
    //   return std::unique_ptr<BaseSegmentPairRelationFilter>(new RecordingSegmentPairRelationFilter());
  } else {
    return std::unique_ptr<BaseSegmentPairRelationFilter>(nullptr);
  }
}
