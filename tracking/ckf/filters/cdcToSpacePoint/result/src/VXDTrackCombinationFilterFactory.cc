/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/result/VXDTrackCombinationFilterFactory.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/BaseVXDTrackCombinationFilter.h>
#include <tracking/ckf/filters/cdcToSpacePoint/result/VXDTrackCombinationVarSet.h>
#include <tracking/ckf/filters/pxdSpacePoint/PXDTrackCombinationVarSet.h>

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
  /// MC filter using the truth information
  using MCVXDTrackCombinationFilter = MCFilter<CKFResultTruthVarSet<RecoTrack, SpacePoint>>;

  /// MC filter using the teacher information
  using MCCTruthTeacherVXDTrackCombinationFilter = NamedChoosableVarSetFilter<CKFResultTruthVarSet<RecoTrack, SpacePoint>>;

  /// Basic recording filter for SVD - CDC relations.
  using RecordingCDCSVDTrackCombinationFilter =
    RecordingFilter<VariadicUnionVarSet<CKFResultTruthVarSet<RecoTrack, SpacePoint>, VXDTrackCombinationVarSet>>;

  /// Basic recording filter for PXD - track relations.
  using RecordingPXDTrackCombinationFilter =
    RecordingFilter<VariadicUnionVarSet<CKFResultTruthVarSet<RecoTrack, SpacePoint>, VXDTrackCombinationVarSet, PXDTrackCombinationVarSet>>;

  /// All filter for VXD - CDC relations.
  using AllVXDTrackCombinationFilter = AllFilter<BaseVXDTrackCombinationFilter>;

  /// Filter using a trained MVA method
  using MVACDCSVDTrackCombinationFilter = MVAFilter<VXDTrackCombinationVarSet>;

  using MVAPXDTrackCombinationFilter = MVAFilter<VariadicUnionVarSet<VXDTrackCombinationVarSet, PXDTrackCombinationVarSet>>;
}

VXDTrackCombinationFilterFactory::VXDTrackCombinationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string VXDTrackCombinationFilterFactory::getIdentifier() const
{
  return "VXDTrackCombination";
}

std::string VXDTrackCombinationFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points on very easy criteria";
}

std::map<std::string, std::string>
VXDTrackCombinationFilterFactory::getValidFilterNamesAndDescriptions() const
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

std::unique_ptr<BaseVXDTrackCombinationFilter>
VXDTrackCombinationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseVXDTrackCombinationFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllVXDTrackCombinationFilter >();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingCDCSVDTrackCombinationFilter>();
  } else if (filterName == "pxd_recording") {
    return std::make_unique<RecordingPXDTrackCombinationFilter>();
  } else if (filterName == "mva") {
    return std::make_unique<MVACDCSVDTrackCombinationFilter>("tracking/data/ckf_CDCSVDTrackCombination.xml");
  } else if (filterName == "truth") {
    return std::make_unique<MCVXDTrackCombinationFilter >();
  } else if (filterName == "truth_teacher") {
    return std::make_unique<MCCTruthTeacherVXDTrackCombinationFilter>("truth_teacher");
  } else {
    return Super::create(filterName);
  }
}
