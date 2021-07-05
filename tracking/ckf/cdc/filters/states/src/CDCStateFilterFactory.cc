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
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/cdc/filters/states/CDCStateBasicVarSet.h>
#include <tracking/ckf/cdc/filters/states/CDCfromEclStateTruthVarSet.h>

#include <tracking/ckf/cdc/filters/states/MCTruthCDCStateFilter.h>
#include <tracking/ckf/cdc/filters/states/RoughCDCStateFilter.h>
#include <tracking/ckf/cdc/filters/states/RoughCDCfromEclStateFilter.h>
#include <tracking/ckf/cdc/filters/states/ExtrapolateAndUpdateCDCStateFilter.h>
#include <tracking/ckf/cdc/filters/states/DistanceCDCStateFilter.h>
#include <tracking/ckf/cdc/filters/states/MCTruthEclSeedFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Recording filter for SVD seeding.
  using RecordingCDCStateFilter = RecordingFilter<VariadicUnionVarSet<CDCStateBasicVarSet>>;
  //using RecordingCDCStateFilter = RecordingFilter<VariadicUnionVarSet<CDCStateBasicVarSet, CDCStateTruthVarSet>>;
  /// Recording filter for ECL seeding.
  using RecordingCDCfromEclStateFilter = RecordingFilter<VariadicUnionVarSet<CDCStateBasicVarSet, CDCfromEclStateTruthVarSet>>;
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
  return "Reject CDC CKF states. ";
}

std::map<std::string, std::string> CDCStateFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"mc_truth", "filtering based on the mc truth information"},
    {"mc_truth_eclSeed", "filtering based on the mc truth information"},
    {"rough", "very rough filtering"},
    {"rough_eclSeed", "very rough filtering, seed created from ECL shower"},
    {"extrapolate_and_update", "Extrapolation and update"},
    {"distance", "Give a weight based on the distance"},
    {"recording", "record variables to a TTree"},
    {"recording_eclSeed", "record variables to a TTree"},
    {"rough_and_recording", "very rough filtering, seed created from SVD track"},
    {"rough_and_recording_eclSeed", "very rough filtering, seed created from ECL shower"},
    {"distance_and_recording_eclSeed", "Give a weight based on the distance"},

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
  } else if (filterName == "rough_eclSeed") {
    return std::make_unique<RoughCDCfromEclStateFilter>();
  } else if (filterName == "mc_truth") {
    return std::make_unique<MCTruthCDCStateFilter>();
  } else if (filterName == "mc_truth_eclSeed") {
    return std::make_unique<MCTruthEclSeedFilter>();
  } else if (filterName == "extrapolate_and_update") {
    return std::make_unique<ExtrapolateAndUpdateCDCStateFilter>();
  } else if (filterName == "distance") {
    return std::make_unique<DistanceCDCStateFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingCDCStateFilter>("CDCStateFilter.root");
  } else if (filterName == "rough_and_recording") {
    return std::make_unique<AndCDCStateFilter>(
             std::make_unique<RecordingCDCStateFilter>("CDCStateFilter.root"),
             std::make_unique<RoughCDCStateFilter>()
           );
  } else if (filterName == "recording_eclSeed") {
    return std::make_unique<RecordingCDCfromEclStateFilter>("CDCfromECLStateFilter.root");
  } else if (filterName == "rough_and_recording_eclSeed") {
    return std::make_unique<AndCDCStateFilter>(
             std::make_unique<RecordingCDCfromEclStateFilter>("CDCfromECLStateFilter.root"),
             std::make_unique<RoughCDCfromEclStateFilter>()
           );
  } else if (filterName == "distance_and_recording_eclSeed") {
    return std::make_unique<AndCDCStateFilter>(
             std::make_unique<RecordingCDCfromEclStateFilter>("CDCfromECLStateFilter.root"),
             std::make_unique<DistanceCDCStateFilter>()
           );
  } else {
    return Super::create(filterName);
  }
}
