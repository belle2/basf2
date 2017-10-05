/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateFilterFactory.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/BaseCKFCDCToSpacePointStateFilter.h>
#include <tracking/ckf/filters/base/CKFStateTruthVarSet.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateVarSet.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateBasicVarSet.h>

#include <tracking/ckf/filters/cdcToSVDSpacePoint/SimpleCKFCDCToSVDStateFilter.h>
#include <tracking/ckf/filters/pxdSpacePoint/SimpleCKFPXDStateFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/SloppyMCFilter.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCCKFCDCToSpacePointStateFilter = MCFilter<CKFStateTruthVarSet<RecoTrack, SpacePoint>>;

  /// MC-ordering filter for VXD - CDC relations.
  using MCOrderingCKFCDCToSpacePointStateFilter = NamedChoosableVarSetFilter<CKFStateTruthVarSet<RecoTrack, SpacePoint>>;

  /// Sloppy MC filter for VXD - CDC relations.
  using SloppyMCCKFCDCToSpacePointStateFilter = SloppyMCFilter<CKFStateTruthVarSet<RecoTrack, SpacePoint>>;

  /// Prescaled recording filter for VXD - CDC relations.
  class SloppyRecordingCKFCDCToSpacePointStateFilter : public
    RecordingFilter<VariadicUnionVarSet<CKFStateTruthVarSet<RecoTrack, SpacePoint>,
    CKFCDCToSpacePointStateBasicVarSet, CKFCDCToSpacePointStateVarSet>> {
    using Super = RecordingFilter<VariadicUnionVarSet<CKFStateTruthVarSet<RecoTrack, SpacePoint>,
          CKFCDCToSpacePointStateBasicVarSet, CKFCDCToSpacePointStateVarSet>>;
  public:
    SloppyRecordingCKFCDCToSpacePointStateFilter(const std::string& defaultRootFileName) : Super(defaultRootFileName)
    {
      setSkimFilter(std::make_unique<SloppyMCCKFCDCToSpacePointStateFilter>());
    }
  };

  /// Recording filter for VXD - CDC relations.
  using RecordingCKFCDCToSpacePointStateFilter = RecordingFilter<VariadicUnionVarSet<CKFStateTruthVarSet<RecoTrack, SpacePoint>,
        CKFCDCToSpacePointStateBasicVarSet, CKFCDCToSpacePointStateVarSet>>;

  /// MVA filter for VXD - CDC relations.
  class MVACKFCDCToSpacePointStateFilter : public
    MVAFilter<CKFCDCToSpacePointStateBasicVarSet> {
    using Super = MVAFilter<CKFCDCToSpacePointStateBasicVarSet>;
  public:
    using Super::Super;

    /// Function to object for its signalness
    Weight operator()(const CKFCDCToSpacePointStateBasicVarSet::Object& obj) override
    {
      if (obj.getHit()) {
        return -Super::operator()(obj);
      } else {
        return 0;
      }
    }
  };

  /// All filter for VXD - CDC relations.
  using AllCKFCDCToSpacePointStateFilter = AllFilter<BaseCKFCDCToSpacePointStateFilter>;
}

CKFCDCToSpacePointStateFilterFactory::CKFCDCToSpacePointStateFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CKFCDCToSpacePointStateFilterFactory::getIdentifier() const
{
  return "CKFCDCToSpacePointState";
}

std::string CKFCDCToSpacePointStateFilterFactory::getFilterPurpose() const
{
  return "Filter out relations between CDC Reco Tracks and Space Points on very easy criteria";
}

std::map<std::string, std::string>
CKFCDCToSpacePointStateFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"truth", "monte carlo truth"},
    {"ordering_truth", "monte carlo truth ordering"},
    {"sloppy_truth", "sloppy monte carlo truth"},
    {"pxd_simple", "simple filter to be used in pxd"},
    {"svd_simple", "simple filter to be used in svd"},
    {"recording", "record variables to a TTree"},
    {"mva", "MVA filter"},
    {"sloppy_recording", "record variables to a TTree"},
  };
}

std::unique_ptr<BaseCKFCDCToSpacePointStateFilter>
CKFCDCToSpacePointStateFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseCKFCDCToSpacePointStateFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllCKFCDCToSpacePointStateFilter>();
  } else if (filterName == "svd_simple") {
    return std::make_unique<SimpleCKFCDCToSVDStateFilter>();
  } else if (filterName == "pxd_simple") {
    return std::make_unique<SimpleCKFPXDStateFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCCKFCDCToSpacePointStateFilter>();
  } else if (filterName == "ordering_truth") {
    return std::make_unique<MCOrderingCKFCDCToSpacePointStateFilter>("truth_inverted");
  } else if (filterName == "sloppy_truth") {
    return std::make_unique<SloppyMCCKFCDCToSpacePointStateFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingCKFCDCToSpacePointStateFilter>("CKFCDCToSpacePointStateFilter.root");
  } else if (filterName == "mva") {
    return std::make_unique<MVACKFCDCToSpacePointStateFilter>("tracking/data/ckf_CDCSVDStateFilter_1.xml");
  } else if (filterName == "sloppy_recording") {
    return std::make_unique<SloppyRecordingCKFCDCToSpacePointStateFilter>("CKFCDCToSpacePointStateFilter.root");
  } else {
    return Super::create(filterName);
  }
}
