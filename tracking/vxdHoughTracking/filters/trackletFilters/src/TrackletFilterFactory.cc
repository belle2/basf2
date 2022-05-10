/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/trackletFilters/TrackletFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

TrackletFilterFactory::TrackletFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

TrackletFilterFactory::~TrackletFilterFactory() = default;

std::string TrackletFilterFactory::getIdentifier() const
{
  return "Factory for Filters for SpacePointTrackCands";
}

std::string TrackletFilterFactory::getFilterPurpose() const
{
  return "Rejects SpacePointTrackCands. ";
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

  // cppcheck-suppress knownConditionTrueFalse
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseTrackletFilter>>();
  }

  return Super::create(filterName);
}
