/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/filters/states/SVDStateFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NegativeFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AndFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/SloppyFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/general/filters/AdvanceFilter.h>
#include <tracking/ckf/general/filters/KalmanFilter.h>
#include <tracking/ckf/svd/utilities/SVDAdvancer.h>
#include <tracking/ckf/svd/utilities/SVDKalmanStepper.h>

#include <tracking/ckf/svd/filters/states/SVDStateVarSet.h>
#include <tracking/ckf/svd/filters/states/SVDStateBasicVarSet.h>
#include <tracking/ckf/svd/filters/states/SVDStateTruthVarSet.h>
#include <tracking/ckf/svd/filters/states/SimpleSVDStateFilter.h>
#include <tracking/ckf/svd/filters/states/ResidualSVDStateFilter.h>
#include <tracking/ckf/svd/filters/states/AllSVDStateFilter.h>
#include <tracking/ckf/svd/filters/states/NonIPCrossingSVDStateFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCSVDStateFilter = TruthVarFilter<SVDStateTruthVarSet>;

  /// MC-ordering filter for VXD - CDC relations.
  using MCOrderingSVDStateFilter = ChoosableFromVarSetFilter<SVDStateTruthVarSet>;

  /// Sloppy MC filter for VXD - CDC relations.
  using SloppyMCSVDStateFilter = Sloppy<TruthVarFilter<SVDStateTruthVarSet>>;

  /// Recording filter for VXD - CDC relations.
  using RecordingSVDStateFilter = RecordingFilter<VariadicUnionVarSet<SVDStateTruthVarSet, SVDStateBasicVarSet, SVDStateVarSet>>;

  /// MVA filter for svd states
  using MVASVDStateFilter = NegativeFilter<MVAFilter<VariadicUnionVarSet<SVDStateBasicVarSet>>>;

  /// And filter for svd states
  using AndSVDStateFilter = AndFilter<BaseSVDStateFilter>;

  /// Prescaled recording filter for VXD - CDC relations.
  class SloppyRecordingSVDStateFilter : public RecordingSVDStateFilter {
  public:
    explicit SloppyRecordingSVDStateFilter(const std::string& defaultRootFileName) : RecordingSVDStateFilter(defaultRootFileName)
    {
      setSkimFilter(std::make_unique<SloppyMCSVDStateFilter>());
    }
  };
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
  return "Reject SVD CKF states. ";
}

std::map<std::string, std::string> SVDStateFilterFactory::getValidFilterNamesAndDescriptions() const
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
    {"simple", "simple filter to be used in svd"},
    {"residual", "residual filter to be used in svd"},
    {"recording", "record variables to a TTree"},
    {"recording_and_truth", "record variables to a TTree and store truth information"},
    {"recording_with_direction_check", "record variables to a TTree with direction check"},
    {"mva", "MVA filter"},
    {"mva_with_direction_check", "MVA filter with direction check"},
    {"sloppy_recording", "record variables to a TTree"},
  };
}

std::unique_ptr<BaseSVDStateFilter>
SVDStateFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseSVDStateFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllSVDStateFilter>();
  } else if (filterName == "non_ip_crossing") {
    return std::make_unique<NonIPCrossingSVDStateFilter>();
  } else if (filterName == "advance") {
    return std::make_unique<AdvanceFilter<CKFToSVDState, SVDAdvancer>>();
  } else if (filterName == "fit") {
    return std::make_unique<KalmanFilter<CKFToSVDState, SVDKalmanStepper>>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleSVDStateFilter>();
  } else if (filterName == "residual") {
    return std::make_unique<ResidualSVDStateFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCSVDStateFilter>();
  } else if (filterName == "ordering_truth") {
    return std::make_unique<MCOrderingSVDStateFilter>("truth_inverted");
  } else if (filterName == "sloppy_truth") {
    return std::make_unique<SloppyMCSVDStateFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingSVDStateFilter>("SVDStateFilter.root");
  } else if (filterName == "recording_and_truth") {
    return std::make_unique<AndSVDStateFilter>(
             std::make_unique<RecordingSVDStateFilter>("SVDStateFilter.root"),
             std::make_unique<MCSVDStateFilter>());
  } else if (filterName == "recording_with_direction_check") {
    return std::make_unique<AndSVDStateFilter>(
             std::make_unique<NonIPCrossingSVDStateFilter>(),
             std::make_unique<RecordingSVDStateFilter>("SVDStateFilter.root"));
  } else if (filterName == "mva") {
    return std::make_unique<MVASVDStateFilter>("ckf_CDCSVDStateFilter_1");
  } else if (filterName == "mva_with_direction_check") {
    return std::make_unique<AndSVDStateFilter>(
             std::make_unique<NonIPCrossingSVDStateFilter>(),
             std::make_unique<MVASVDStateFilter>("ckf_CDCSVDStateFilter_1"));
  } else if (filterName == "sloppy_recording") {
    return std::make_unique<SloppyRecordingSVDStateFilter>("SVDStateFilter.root");
  } else {
    return Super::create(filterName);
  }
}
