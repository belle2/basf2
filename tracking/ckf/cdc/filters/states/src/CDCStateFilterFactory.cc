/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/CDCStateFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AndFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NegativeFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/cdc/filters/states/CDCStateTruthVarSet.h>

#include <tracking/ckf/cdc/filters/states/MCTruthCDCStateFilter.h>
#include <tracking/ckf/cdc/filters/states/RoughCDCStateFilter.h>
#include <tracking/ckf/cdc/filters/states/RoughCDCfromEclStateFilter.h>
#include <tracking/ckf/cdc/filters/states/ExtrapolateAndUpdateCDCStateFilter.h>
#include <tracking/ckf/cdc/filters/states/DistanceCDCStateFilter.h>
#include <tracking/ckf/cdc/filters/states/MCTruthEclSeedFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Recording filter for ECL seeding.
  using RecordingCDCStateFilter = RecordingFilter<VariadicUnionVarSet<CDCStateTruthVarSet>>;
  /// And filter for cdc states
  using AndCDCStateFilter = AndFilter<BaseCDCStateFilter>;
}


CDCStateFilterFactory::CDCStateFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

CDCStateFilterFactory::~CDCStateFilterFactory() = default;

std::string CDCStateFilterFactory::getIdentifier() const
{
  return "CDCState";
}

std::string CDCStateFilterFactory::getFilterPurpose() const
{
  return "Reject cdc states";
}

std::map<std::string, std::string> CDCStateFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"mc_truth", "filtering based on the mc truth information"},
    {"mc_truth_eclSeed", "filtering based on the mc truth information"},
    {"rough", "very rough filtering"},
    {"roughEclSeed", "very rough filtering, seed created from ECL shower"},
    {"extrapolate_and_update", "Extrapolation and update"},
    {"distance", "Give a weight based on the distance"},
    {"recording_eclSeed", "record variables to a TTree"},
    {"roughEclSeed_and_record", "very rough filtering, seed created from ECL shower"},
  };
}

std::unique_ptr<BaseCDCStateFilter>
CDCStateFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseCDCStateFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseCDCStateFilter>>();
  } else if (filterName == "rough") {
    return std::make_unique<RoughCDCStateFilter>();
  } else if (filterName == "roughEclSeed") {
    return std::make_unique<RoughCDCfromEclStateFilter>();
  } else if (filterName == "mc_truth") {
    return std::make_unique<MCTruthCDCStateFilter>();
  } else if (filterName == "mc_truth_eclSeed") {
    return std::make_unique<MCTruthEclSeedFilter>();
  } else if (filterName == "extrapolate_and_update") {
    return std::make_unique<ExtrapolateAndUpdateCDCStateFilter>();
  } else if (filterName == "distance") {
    return std::make_unique<DistanceCDCStateFilter>();
  } else if (filterName == "recording_eclSeed") {
    return std::make_unique<RecordingCDCStateFilter>("CDCStateFilter.root");
  } else if (filterName == "roughEclSeed_and_record") {
    return std::make_unique<AndCDCStateFilter>(
             std::make_unique<RoughCDCfromEclStateFilter>(),
             std::make_unique<RecordingCDCStateFilter>("CDCStateFilter.root")
           );
  } else {
    return Super::create(filterName);
  }
}
