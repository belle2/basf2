/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/pathFilters/PathFilterFactory.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/TwoHitVirtualIPFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/TwoHitVirtualIPQIFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/ThreeHitFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/FourHitFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/FiveHitFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/TwoHitVirtualIPQIFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/QualityIndicatorFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

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
  return "Rejects track candidates based on the number of hits. ";
}

std::map<std::string, std::string> PathFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
    {"twoHitVirtualIP", "filter using two hits and a virtual IP at the origin"},
    {"twoHitVirtualIPQI", "filter using two hits and a virtual IP at the origin based on a fit quality indicator"},
    {"threeHit", "filter three hits (path of length 2 plus next hit)"},
    {"fourHit", "filter four hits (path of length 3 plus next hit)"},
    {"fiveHit", "filter five hits (path of length 4 plus next hit)"},
    {"qualityIndicator", "filter a path of hits based on a fit quality indicator"},
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
  if (filterName == "twoHitVirtualIPQI") {
    return std::make_unique<TwoHitVirtualIPQIFilter>();
  }
  if (filterName == "threeHit") {
    return std::make_unique<ThreeHitFilter>();
  }
  if (filterName == "fourHit") {
    return std::make_unique<FourHitFilter>();
  }
  if (filterName == "fiveHit") {
    return std::make_unique<FiveHitFilter>();
  }
  if (filterName == "qualityIndicator") {
    return std::make_unique<QualityIndicatorFilter>();
  }

  return Super::create(filterName);
}
