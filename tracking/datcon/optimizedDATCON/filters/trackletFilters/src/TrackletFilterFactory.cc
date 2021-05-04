/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/trackletFilters/TrackletFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackletFilterFactory::TrackletFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

TrackletFilterFactory::~TrackletFilterFactory() = default;

std::string TrackletFilterFactory::getIdentifier() const
{
  return "SVD Hit Pair";
}

std::string TrackletFilterFactory::getFilterPurpose() const
{
  return "Rejects SVD hit pairs. ";
}

std::map<std::string, std::string> TrackletFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
  };
}

std::unique_ptr<BaseTrackletFilter>
TrackletFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseTrackletFilter>>();
  }

  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseTrackletFilter>>();
  }

  return Super::create(filterName);
}
