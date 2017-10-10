/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/states/SVDStateFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

#include <tracking/ckf/general/findlets/OnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LimitedOnStateApplier.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllSVDOnStateApplier = LimitedOnStateApplier<CKFToSVDState, TrackFindingCDC::AllFilter<BaseSVDStateFilter>>;
}


SVDStateFilterFactory::SVDStateFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

SVDStateFilterFactory::~SVDStateFilterFactory() = default;

std::string SVDStateFilterFactory::getIdentifier() const
{
  return "SVDState";
}

std::string SVDStateFilterFactory::getFilterPurpose() const
{
  return "Reject svd states";
}

std::map<std::string, std::string> SVDStateFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no combination is valid"},
    {"all", "all combination are valid"},
  };
}

std::unique_ptr<BaseSVDOnStateApplier>
SVDStateFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<BaseSVDOnStateApplier>();
  } else if (filterName == "all") {
    return std::make_unique<AllSVDOnStateApplier>();
  } else {
    return Super::create(filterName);
  }
}
