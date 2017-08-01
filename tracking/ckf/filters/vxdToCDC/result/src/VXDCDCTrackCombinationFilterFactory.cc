/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/vxdToCDC/result/VXDCDCTrackCombinationFilterFactory.h>
#include <tracking/ckf/filters/vxdToCDC/result/BaseVXDCDCTrackCombinationFilter.h>

#include <tracking/ckf/filters/base/CKFResultTruthVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCVXDCDCTrackCombinationFilter = MCFilter<CKFResultTruthVarSet<RecoTrack, CDCRLWireHit>>;

  /// MC filter for VXD - CDC relations.
  using MCCTruthTeacherVXDCDCTrackCombinationFilter = NamedChoosableVarSetFilter<CKFResultTruthVarSet<RecoTrack, CDCRLWireHit>>;

  /// All filter for VXD - CDC relations.
  using AllVXDCDCTrackCombinationFilter = AllFilter<BaseVXDCDCTrackCombinationFilter>;
}

VXDCDCTrackCombinationFilterFactory::VXDCDCTrackCombinationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string VXDCDCTrackCombinationFilterFactory::getIdentifier() const
{
  return "VXDCDCTrackCombination";
}

std::string VXDCDCTrackCombinationFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points on very easy criteria";
}

std::map<std::string, std::string>
VXDCDCTrackCombinationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"truth", "monte carlo truth"},
    {"truth_teacher", "monte carlo truth returning the result of the teacher"},
  };
}

std::unique_ptr<BaseVXDCDCTrackCombinationFilter>
VXDCDCTrackCombinationFilterFactory::create(const std::string& filterName) const
{
  // TODO: Implement a simple, recording and mva filter here (with a var set)
  if (filterName == "none") {
    return makeUnique<NoneFilter<BaseVXDCDCTrackCombinationFilter>>();
  } else if (filterName == "all") {
    return makeUnique<AllVXDCDCTrackCombinationFilter >();
  } else if (filterName == "truth") {
    return makeUnique<MCVXDCDCTrackCombinationFilter >();
  } else if (filterName == "truth_teacher") {
    return makeUnique<MCCTruthTeacherVXDCDCTrackCombinationFilter>("truth_teacher");
  } else {
    return Super::create(filterName);
  }
}
