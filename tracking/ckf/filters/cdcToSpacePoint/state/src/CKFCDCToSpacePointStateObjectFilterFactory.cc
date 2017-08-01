/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectFilterFactory.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/BaseCKFCDCToSpacePointStateObjectFilter.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectTruthVarSet.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectVarSet.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectBasicVarSet.h>

#include <tracking/ckf/filters/cdcToSVDSpacePoint/SimpleCKFCDCToSVDStateFilter.h>
#include <tracking/ckf/filters/pxdSpacePoint/SimpleCKFPXDStateFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCCKFCDCToSpacePointStateObjectFilter = MCFilter<CKFCDCToSpacePointStateObjectTruthVarSet>;

  /// Recording filter for VXD - CDC relations.
  using RecordingCKFCDCToSpacePointStateObjectFilter =
    RecordingFilter<VariadicUnionVarSet<CKFCDCToSpacePointStateObjectTruthVarSet,
    CKFCDCToSpacePointStateObjectBasicVarSet, CKFCDCToSpacePointStateObjectVarSet>>;

  /// Basic recording filter for VXD - CDC relations.
  using BasicRecordingCKFCDCToSpacePointStateObjectFilter =
    RecordingFilter<VariadicUnionVarSet<CKFCDCToSpacePointStateObjectTruthVarSet, CKFCDCToSpacePointStateObjectBasicVarSet>>;

  /// All filter for VXD - CDC relations.
  using AllCKFCDCToSpacePointStateObjectFilter = AllFilter<BaseCKFCDCToSpacePointStateObjectFilter>;
}

CKFCDCToSpacePointStateObjectFilterFactory::CKFCDCToSpacePointStateObjectFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CKFCDCToSpacePointStateObjectFilterFactory::getIdentifier() const
{
  return "CKFCDCToSpacePointStateObject";
}

std::string CKFCDCToSpacePointStateObjectFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points on very easy criteria";
}

std::map<std::string, std::string>
CKFCDCToSpacePointStateObjectFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"basic_recording", "record variables to a TTree"},
  };
}

std::unique_ptr<BaseCKFCDCToSpacePointStateObjectFilter>
CKFCDCToSpacePointStateObjectFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<NoneFilter<BaseCKFCDCToSpacePointStateObjectFilter>>();
  } else if (filterName == "all") {
    return makeUnique<AllCKFCDCToSpacePointStateObjectFilter>();
  } else if (filterName == "svd_simple") {
    return makeUnique<SimpleCKFCDCToSVDStateFilter>();
  } else if (filterName == "pxd_simple") {
    return makeUnique<SimpleCKFPXDStateFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCCKFCDCToSpacePointStateObjectFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingCKFCDCToSpacePointStateObjectFilter>("CKFCDCToSpacePointStateObjectFilter.root");
  } else if (filterName == "basic_recording") {
    return makeUnique<BasicRecordingCKFCDCToSpacePointStateObjectFilter>("CKFCDCToSpacePointStateObjectFilter.root");
  } else {
    return Super::create(filterName);
  }
}
