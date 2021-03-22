/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/PathFilterFactory.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/TwoHitVirtualIPFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/ThreeHitFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/FourHitFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/ShortPathFilter.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;


PathFilterFactory::PathFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

PathFilterFactory::~PathFilterFactory() = default;

std::string PathFilterFactory::getIdentifier() const
{
  return "SVD Hit Path Filter";
}

std::string PathFilterFactory::getFilterPurpose() const
{
  return "Rejects SVD hit combination of different length. ";
}

std::map<std::string, std::string> PathFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
    {"twoHitVirtualIP", "filter using two hits and a virtual IP at the origin"},
    {"threeHit", "filter three hits (path of length 2 plus next hit)"},
    {"fourHit", "filter four hits (path of length 3 plus next hit)"},
    {"shortPath", "filter a path longer than 4 hits"},
  };
}

std::unique_ptr<BasePathFilter>
PathFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BasePathFilter>>();
  }
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BasePathFilter>>();
  }
  if (filterName == "twoHitVirtualIP") {
    return std::make_unique<TwoHitVirtualIPFilter>();
  }
  if (filterName == "threeHit") {
    return std::make_unique<ThreeHitFilter>();
  }
  if (filterName == "fourHit") {
    return std::make_unique<FourHitFilter>();
  }
  if (filterName == "shortPath") {
    return std::make_unique<ShortPathFilter>();
  }

  return Super::create(filterName);
}
