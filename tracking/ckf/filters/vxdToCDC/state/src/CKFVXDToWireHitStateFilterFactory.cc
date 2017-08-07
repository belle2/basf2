/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/vxdToCDC/state/CKFVXDToWireHitStateFilterFactory.h>
#include <tracking/ckf/filters/vxdToCDC/state/BaseCKFCKFVXDToWireHitStateFilter.h>
#include <tracking/ckf/filters/vxdToCDC/state/CKFVXDToWireHitStateTruthVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCCKFVXDToWireHitStateFilter = MCFilter<CKFVXDToWireHitStateTruthVarSet>;

  /// All filter for VXD - CDC relations.
  using AllCKFVXDToWireHitStateFilter = AllFilter<BaseCKFCKFVXDToWireHitStateFilter>;
}

CKFVXDToWireHitStateFilterFactory::CKFVXDToWireHitStateFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CKFVXDToWireHitStateFilterFactory::getIdentifier() const
{
  return "CKFVXDToWireHitState";
}

std::string CKFVXDToWireHitStateFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points on very easy criteria";
}

std::map<std::string, std::string>
CKFVXDToWireHitStateFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"truth", "monte carlo truth"},
  };
}

std::unique_ptr<BaseCKFCKFVXDToWireHitStateFilter>
CKFVXDToWireHitStateFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseCKFCKFVXDToWireHitStateFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllCKFVXDToWireHitStateFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCCKFVXDToWireHitStateFilter>();
  } else {
    return Super::create(filterName);
  }
}
