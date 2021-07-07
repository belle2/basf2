/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/paths/CDCPathFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>

#include <tracking/ckf/cdc/filters/paths/SizeCDCPathFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AndFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

#include <tracking/ckf/cdc/filters/paths/CDCPathBasicVarSet.h>
#include <tracking/ckf/cdc/filters/paths/CDCPathTruthVarSet.h>
#include <tracking/ckf/cdc/filters/paths/CDCfromEclPathTruthVarSet.h>

#include <tracking/ckf/cdc/filters/paths/SeedChargeCDCPathFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Recording filter
  using RecordingCDCPathFilter = RecordingFilter<VariadicUnionVarSet<CDCPathBasicVarSet, CDCPathTruthVarSet>>;
  /// Recording filter
  using RecordingCDCfromEclPathFilter = RecordingFilter<VariadicUnionVarSet<CDCPathBasicVarSet, CDCfromEclPathTruthVarSet>>;
  /// And filter for cdc paths
  using AndCDCPathFilter = AndFilter<BaseCDCPathFilter>;
}

CDCPathFilterFactory::CDCPathFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

CDCPathFilterFactory::~CDCPathFilterFactory() = default;

std::string CDCPathFilterFactory::getIdentifier() const
{
  return "CDCCKFPath";
}

std::string CDCPathFilterFactory::getFilterPurpose() const
{
  return "Reject CDC CKF paths. ";
}

std::map<std::string, std::string> CDCPathFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"size", "very rough filtering"},
    {"recording", "record variables to a TTree"},
    {"size_and_recording", "record variables to a TTree"},
    {"recording_fromEcl", "record variables to a TTree"},
    {"size_and_recording_fromEcl", "record variables to a TTree"},
    {"seedCharge", "charge of path corresponds to charge of seed"},
    {"seedCharge_and_recording_fromEcl", "record variables to a TTree"}
    //{"mc_truth", "Extrapolation and update"},
  };
}

std::unique_ptr<BaseCDCPathFilter>
CDCPathFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseCDCPathFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseCDCPathFilter>>();
  } else if (filterName == "size") {
    return std::make_unique<SizeCDCPathFilter>();
    //} else if (filterName == "mc_truth") {
    //  return std::make_unique<MCTruthCDCPathFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingCDCPathFilter>("CDCPathFilter.root");
  } else if (filterName == "size_and_recording") {
    return std::make_unique<AndCDCPathFilter>(
             std::make_unique<RecordingCDCPathFilter>("CDCPathFilter.root"),
             std::make_unique<SizeCDCPathFilter>()
           );
  } else if (filterName == "recording_fromEcl") {
    return std::make_unique<RecordingCDCfromEclPathFilter>("CDCfromEclPathFilter.root");
  } else if (filterName == "size_and_recording_fromEcl") {
    return std::make_unique<AndCDCPathFilter>(
             std::make_unique<RecordingCDCfromEclPathFilter>("CDCfromEclPathFilter.root"),
             std::make_unique<SizeCDCPathFilter>()
           );
  } else if (filterName == "seedCharge") {
    return std::make_unique<SeedChargeCDCPathFilter>();
  } else if (filterName == "seedCharge_and_recording_fromEcl") {
    return std::make_unique<AndCDCPathFilter>(
             std::make_unique<RecordingCDCfromEclPathFilter>("CDCfromEclPathFilter.root"),
             std::make_unique<SeedChargeCDCPathFilter>()
           );

  } else {
    return Super::create(filterName);
  }
}
