/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/TrackRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>

#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/AllTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/MCTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/UnionRecordingTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/MVAFeasibleTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/MVARealisticTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/PhiTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseTrackRelationFilter>;

TrackRelationFilterFactory::TrackRelationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string TrackRelationFilterFactory::getIdentifier() const
{
  return "TrackRelation";
}

std::string TrackRelationFilterFactory::getFilterPurpose() const
{
  return "Track relation filter to construct a track network for merging";
}

std::map<std::string, std::string>
TrackRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "accepts nothing"},
    {"all", "accepts everything"},
    {"truth", "accepts based on monte carlo information"},
    {"unionrecording", "record multiple choosable variable set"},
    {"phi", "filter just based on the phi distance"},
    {"feasible", "a rough efficient compatibility check"},
    {"realistic", "an expensive pure compatibility check"},
  };
}

std::unique_ptr<BaseTrackRelationFilter>
TrackRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseTrackRelationFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllTrackRelationFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCTrackRelationFilter>();
  } else if (filterName == "unionrecording") {
    return std::make_unique< UnionRecordingTrackRelationFilter>();
  } else if (filterName == "feasible") {
    return std::make_unique<MVAFeasibleTrackRelationFilter>();
  } else if (filterName == "realistic") {
    return std::make_unique<MVARealisticTrackRelationFilter>();
  } else if (filterName == "phi") {
    return std::make_unique<PhiTrackRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
