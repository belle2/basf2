/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Malwin Weiler, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/vxdCDCTrackMerging/detectorTrackCombination/DetectorTrackCombinationFilterFactory.h>

#include <tracking/vxdCDCTrackMerging/detectorTrackCombination/DetectorTrackCombinationVarSet.h>
#include <tracking/vxdCDCTrackMerging/detectorTrackCombination/DetectorTrackCombinationTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/PassThroughFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/RandomFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for VXD - CDC relations.
  using MCDetectorTrackCombinationFilter =
    MCFilter<VariadicUnionVarSet<DetectorTrackCombinationTruthVarSet, DetectorTrackCombinationVarSet>>;

  /// Recording filter for VXD - CDC relations.
  using RecordingDetectorTrackCombinationFilter =
    RecordingFilter<VariadicUnionVarSet<DetectorTrackCombinationTruthVarSet, DetectorTrackCombinationVarSet>>;

  /// All filter for VXD - CDC relations.
  using AllDetectorTrackCombinationFilter = AllFilter<BaseDetectorTrackCombinationFilter>;

  /// MVA filter for VXD - CDC relations.
  using MVADetectorTrackCombinationFilter = MVAFilter<DetectorTrackCombinationVarSet>;

  // A pass through filter
  using PassThroughDetectorTrackCombinationWeightFilter = PassThroughFilter<BaseDetectorTrackCombinationFilter>;
}

DetectorTrackCombinationFilterFactory::DetectorTrackCombinationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string DetectorTrackCombinationFilterFactory::getIdentifier() const
{
  return "DetectorTrackCombination";
}

std::string DetectorTrackCombinationFilterFactory::getFilterPurpose() const
{
  return "Combine tracks from different detectors (e.g. VXD and CDC).";
}

std::map<std::string, std::string>
DetectorTrackCombinationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track combination is valid"},
    {"all", "set all track combinations as good"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"mva", "test with a mva method"},
    {"pass_through", "do not do anything to the weights"},
  };
}

std::unique_ptr<BaseDetectorTrackCombinationFilter>
DetectorTrackCombinationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseDetectorTrackCombinationFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllDetectorTrackCombinationFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCDetectorTrackCombinationFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingDetectorTrackCombinationFilter>("DetectorTrackCombinationFilter.root");
  } else if (filterName == "mva") {
    return makeUnique<MVADetectorTrackCombinationFilter>("tracking/data/vxdcdc_DetectorTrackCombinationFilter.xml");
  } else if (filterName == "pass_through") {
    return makeUnique<PassThroughDetectorTrackCombinationWeightFilter>();
  } else {
    return Super::create(filterName);
  }
}
