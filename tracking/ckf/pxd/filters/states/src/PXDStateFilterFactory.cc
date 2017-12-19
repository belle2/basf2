/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/states/PXDStateFilterFactory.h>

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
#include <tracking/ckf/pxd/utilities/PXDAdvancer.h>
#include <tracking/ckf/pxd/utilities/PXDKalmanStepper.h>

#include <tracking/ckf/pxd/filters/states/PXDStateBasicVarSet.h>
#include <tracking/ckf/pxd/filters/states/PXDStateTruthVarSet.h>
#include <tracking/ckf/pxd/filters/states/SimplePXDStateFilter.h>
#include <tracking/ckf/pxd/filters/states/AllPXDStateFilter.h>
#include <tracking/ckf/pxd/filters/states/NonIPCrossingPXDStateFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCPXDStateFilter = TruthVarFilter<PXDStateTruthVarSet>;

  /// MC-ordering filter for VXD - CDC relations.
  using MCOrderingPXDStateFilter = ChoosableFromVarSetFilter<PXDStateTruthVarSet>;

  /// Sloppy MC filter for VXD - CDC relations.
  using SloppyMCPXDStateFilter = Sloppy<TruthVarFilter<PXDStateTruthVarSet>>;

  /// Recording filter for VXD - CDC relations.
  using RecordingPXDStateFilter = RecordingFilter<VariadicUnionVarSet<PXDStateTruthVarSet, PXDStateBasicVarSet>>;

  /// MVA filter for pxd states
  using MVAPXDStateFilter = NegativeFilter<MVAFilter<PXDStateBasicVarSet>>;

  /// And filter for pxd states
  using AndPXDStateFilter = AndFilter<BasePXDStateFilter>;

  /// Prescaled recording filter for VXD - CDC relations.
  class SloppyRecordingPXDStateFilter : public RecordingPXDStateFilter {
  public:
    SloppyRecordingPXDStateFilter(const std::string& defaultRootFileName) : RecordingPXDStateFilter(defaultRootFileName)
    {
      setSkimFilter(std::make_unique<SloppyMCPXDStateFilter>());
    }
  };
}


PXDStateFilterFactory::PXDStateFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

PXDStateFilterFactory::~PXDStateFilterFactory() = default;

std::string PXDStateFilterFactory::getIdentifier() const
{
  return "PXDState";
}

std::string PXDStateFilterFactory::getFilterPurpose() const
{
  return "Reject pxd states";
}

std::map<std::string, std::string> PXDStateFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"advance", "extrapolate the states"},
    {"fit", "update the mSoP using a Kalman Filter"},
    {"truth", "monte carlo truth"},
    {"ordering_truth", "monte carlo truth ordering"},
    {"sloppy_truth", "sloppy monte carlo truth"},
    {"simple", "simple filter to be used in pxd"},
    {"recording", "record variables to a TTree"},
    {"mva_with_direction_check", "MVA filter with direction check"},
    {"mva", "MVA filter"},
    {"sloppy_recording", "record variables to a TTree"},
  };
}

std::unique_ptr<BasePXDStateFilter>
PXDStateFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BasePXDStateFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllPXDStateFilter>();
  } else if (filterName == "advance") {
    return std::make_unique<AdvanceFilter<CKFToPXDState, PXDAdvancer>>();
  } else if (filterName == "fit") {
    return std::make_unique<KalmanFilter<CKFToPXDState, PXDKalmanStepper>>();
  } else if (filterName == "simple") {
    return std::make_unique<SimplePXDStateFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCPXDStateFilter>();
  } else if (filterName == "ordering_truth") {
    return std::make_unique<MCOrderingPXDStateFilter>("truth_inverted");
  } else if (filterName == "sloppy_truth") {
    return std::make_unique<SloppyMCPXDStateFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingPXDStateFilter>("PXDStateFilter.root");
  } else if (filterName == "mva_with_direction_check") {
    return std::make_unique<AndPXDStateFilter>(
             std::make_unique<MVAPXDStateFilter>("tracking/data/ckf_CDCPXDStateFilter_1.xml"),
             std::make_unique<NonIPCrossingPXDStateFilter>());
  } else if (filterName == "mva") {
    return std::make_unique<MVAPXDStateFilter>("tracking/data/ckf_CDCPXDStateFilter_1.xml");
  } else if (filterName == "sloppy_recording") {
    return std::make_unique<SloppyRecordingPXDStateFilter>("PXDStateFilter.root");
  } else {
    return Super::create(filterName);
  }
}
