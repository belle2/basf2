/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/threeHitFilters/ThreeHitFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;


ThreeHitFilterFactory::ThreeHitFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

ThreeHitFilterFactory::~ThreeHitFilterFactory() = default;

std::string ThreeHitFilterFactory::getIdentifier() const
{
  return "SVD Hit Pair";
}

std::string ThreeHitFilterFactory::getFilterPurpose() const
{
  return "Rejects SVD hit pairs. ";
}

std::map<std::string, std::string> ThreeHitFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
  };
}

std::unique_ptr<BaseThreeHitFilter>
ThreeHitFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseThreeHitFilter>>();
  }

  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseThreeHitFilter>>();
  }

  return Super::create(filterName);
}
