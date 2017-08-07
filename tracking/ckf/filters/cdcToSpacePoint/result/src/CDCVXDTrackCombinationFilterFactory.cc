/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/result/CDCVXDTrackCombinationFilterFactory.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/BaseCDCVXDTrackCombinationFilter.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/CDCVXDTrackCombinationVarSet.h>

#include <tracking/ckf/filters/base/CKFResultTruthVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCCDCVXDTrackCombinationFilter = MCFilter<CKFResultTruthVarSet<RecoTrack, SpacePoint>>;

  /// MC filter for VXD - CDC relations.
  using MCCTruthTeacherCDCVXDTrackCombinationFilter = NamedChoosableVarSetFilter<CKFResultTruthVarSet<RecoTrack, SpacePoint>>;

  /// Basic recording filter for VXD - CDC relations.
  using RecordingCDCVXDTrackCombinationFilter =
    RecordingFilter<VariadicUnionVarSet<CKFResultTruthVarSet<RecoTrack, SpacePoint>, CDCVXDTrackCombinationVarSet>>;

  /// All filter for VXD - CDC relations.
  using AllCDCVXDTrackCombinationFilter = AllFilter<BaseCDCVXDTrackCombinationFilter>;

  /// Filter using a trained MVA method
  using MVACDCVXDTrackCombinationFilter = MVAFilter<CDCVXDTrackCombinationVarSet>;
}

CDCVXDTrackCombinationFilterFactory::CDCVXDTrackCombinationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CDCVXDTrackCombinationFilterFactory::getIdentifier() const
{
  return "CDCVXDTrackCombination";
}

std::string CDCVXDTrackCombinationFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points on very easy criteria";
}

std::map<std::string, std::string>
CDCVXDTrackCombinationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"recording", "record variables to a TTree"},
    {"mva", "filter based on the trained MVA method"},
    {"truth", "monte carlo truth"},
    {"truth_teacher", "monte carlo truth returning the result of the teacher"},
  };
}

std::unique_ptr<BaseCDCVXDTrackCombinationFilter>
CDCVXDTrackCombinationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseCDCVXDTrackCombinationFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllCDCVXDTrackCombinationFilter >();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingCDCVXDTrackCombinationFilter>();
  } else if (filterName == "mva") {
    return std::make_unique<MVACDCVXDTrackCombinationFilter>("tracking/data/ckf_CDCVXDTrackCombination.xml");
  } else if (filterName == "truth") {
    return std::make_unique<MCCDCVXDTrackCombinationFilter >();
  } else if (filterName == "truth_teacher") {
    return std::make_unique<MCCTruthTeacherCDCVXDTrackCombinationFilter>("truth_teacher");
  } else {
    return Super::create(filterName);
  }
}
