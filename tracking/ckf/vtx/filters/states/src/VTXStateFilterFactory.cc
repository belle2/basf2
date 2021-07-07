/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/states/VTXStateFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AndFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NegativeFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/SloppyFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/general/filters/AdvanceFilter.h>
#include <tracking/ckf/general/filters/KalmanFilter.h>
#include <tracking/ckf/vtx/utilities/VTXAdvancer.h>
#include <tracking/ckf/vtx/utilities/VTXKalmanStepper.h>

#include <tracking/ckf/vtx/filters/states/VTXStateVarSet.h>
#include <tracking/ckf/vtx/filters/states/VTXStateBasicVarSet.h>
#include <tracking/ckf/vtx/filters/states/VTXStateTruthVarSet.h>
#include <tracking/ckf/vtx/filters/states/SimpleVTXStateFilter.h>
#include <tracking/ckf/vtx/filters/states/ResidualVTXStateFilter.h>
#include <tracking/ckf/vtx/filters/states/AllVTXStateFilter.h>
#include <tracking/ckf/vtx/filters/states/NonIPCrossingVTXStateFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VTX - CDC relations.
  using MCVTXStateFilter = TruthVarFilter<VTXStateTruthVarSet>;

  /// MC-ordering filter for VTX - CDC relations.
  using MCOrderingVTXStateFilter = ChoosableFromVarSetFilter<VTXStateTruthVarSet>;

  /// Sloppy MC filter for VTX - CDC relations.
  using SloppyMCVTXStateFilter = Sloppy<TruthVarFilter<VTXStateTruthVarSet>>;

  /// Recording filter for VTX - CDC relations.
  using RecordingVTXStateFilter = RecordingFilter<VariadicUnionVarSet<VTXStateTruthVarSet, VTXStateBasicVarSet, VTXStateVarSet>>;

  /// MVA filter for vtx states
//   using MVAVTXStateFilter = NegativeFilter<MVAFilter<VTXStateBasicVarSet>>;
  using MVAVTXStateFilter = NegativeFilter<MVAFilter<VariadicUnionVarSet<VTXStateBasicVarSet, VTXStateVarSet>>>;

  /// And filter for vtx states
  using AndVTXStateFilter = AndFilter<BaseVTXStateFilter>;

  /// Prescaled recording filter for VTX - CDC relations.
  class SloppyRecordingVTXStateFilter : public RecordingVTXStateFilter {
  public:
    explicit SloppyRecordingVTXStateFilter(const std::string& defaultRootFileName) : RecordingVTXStateFilter(defaultRootFileName)
    {
      setSkimFilter(std::make_unique<SloppyMCVTXStateFilter>());
    }
  };
}


VTXStateFilterFactory::VTXStateFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

VTXStateFilterFactory::~VTXStateFilterFactory() = default;

std::string VTXStateFilterFactory::getIdentifier() const
{
  return "VTXState";
}

std::string VTXStateFilterFactory::getFilterPurpose() const
{
  return "Reject VTX states. ";
}

std::map<std::string, std::string> VTXStateFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"non_ip_crossing", "all combinations are fine, except for crossing IPs"},
    {"advance", "extrapolate the states"},
    {"fit", "update the mSoP using a Kalman Filter"},
    {"truth", "monte carlo truth"},
    {"ordering_truth", "monte carlo truth ordering"},
    {"sloppy_truth", "sloppy monte carlo truth"},
    {"simple", "simple filter to be used in vtx"},
    {"residual", "residual filter to be used in vtx"},
    {"recording", "record variables to a TTree"},
    {"recording_with_direction_check", "record variables to a TTree with direction check"},
    {"recording_and_truth", "record variables to a TTree and store truth information"},
    {"mva_with_direction_check", "MVA filter with direction check"},
    {"mva", "MVA filter"},
    {"sloppy_recording", "record variables to a TTree"},
  };
}

std::unique_ptr<BaseVTXStateFilter>
VTXStateFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseVTXStateFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllVTXStateFilter>();
  } else if (filterName == "non_ip_crossing") {
    return std::make_unique<NonIPCrossingVTXStateFilter>();
  } else if (filterName == "advance") {
    return std::make_unique<AdvanceFilter<CKFToVTXState, VTXAdvancer>>();
  } else if (filterName == "fit") {
    return std::make_unique<KalmanFilter<CKFToVTXState, VTXKalmanStepper>>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleVTXStateFilter>();
  } else if (filterName == "residual") {
    return std::make_unique<ResidualVTXStateFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCVTXStateFilter>();
  } else if (filterName == "ordering_truth") {
    return std::make_unique<MCOrderingVTXStateFilter>("truth_inverted");
  } else if (filterName == "sloppy_truth") {
    return std::make_unique<SloppyMCVTXStateFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingVTXStateFilter>("VTXStateFilterRecords.root");
  } else if (filterName == "recording_with_direction_check") {
    return std::make_unique<AndVTXStateFilter>(
             std::make_unique<NonIPCrossingVTXStateFilter>(),
             std::make_unique<RecordingVTXStateFilter>("VTXStateFilterRecords.root"));
  } else if (filterName == "recording_and_truth") {
    return std::make_unique<AndVTXStateFilter>(
             std::make_unique<RecordingVTXStateFilter>("VTXStateFilterRecords.root"),
             std::make_unique<MCVTXStateFilter>());
  } else if (filterName == "mva_with_direction_check") {
    return std::make_unique<AndVTXStateFilter>(
             std::make_unique<NonIPCrossingVTXStateFilter>(),
             std::make_unique<MVAVTXStateFilter>("ckf_CDCVTXStateFilter_1"));
  } else if (filterName == "mva") {
    return std::make_unique<AndVTXStateFilter>(
             std::make_unique<MVAVTXStateFilter>("ckf_CDCVTXStateFilter_1"),
             std::make_unique<NonIPCrossingVTXStateFilter>());
  } else if (filterName == "sloppy_recording") {
    return std::make_unique<SloppyRecordingVTXStateFilter>("VTXStateFilterRecords.root");
  } else {
    return Super::create(filterName);
  }
}
