/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentFilterFactory.h>
#include <tracking/trackFindingCDC/filters/stereoSegments/SimpleStereoSegmentFilter.h>

#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentVarSet.h>

#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/RandomFilter.h>
#include <tracking/trackFindingCDC/filters/base/NamedChoosableVarSetFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// MC filter for stereo segment - track relations.
  using MCStereoSegmentFilter = MCFilter<StereoSegmentTruthVarSet>;

  /// Recording filter for stereo segment - track relations.
  using RecordingStereoSegmentFilter =
    RecordingFilter<VariadicUnionVarSet<StereoSegmentTruthVarSet, StereoSegmentVarSet>>;

  /// All filter for stereo segment - track relations.
  using AllStereoSegmentFilter = AllFilter<BaseStereoSegmentFilter>;

  /// Random filter for stereo segment - track relations.
  using RandomStereoSegmentFilter = RandomFilter<BaseStereoSegmentFilter>;

  /// TMVA filter for stereo segment - track relations.
  using TMVAStereoSegmentFilter = TMVAFilter<StereoSegmentVarSet>;
}

StereoSegmentFilterFactory::StereoSegmentFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string StereoSegmentFilterFactory::getIdentifier() const
{
  return "StereoSegment";
}

std::string StereoSegmentFilterFactory::getFilterPurpose() const
{
  return "Stereo segment to track combination filter for adding the segment.";
}

std::map<std::string, std::string>
StereoSegmentFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid."},
    {"all", "set all segments as good."},
    {"random", "returns a random weight (for testing)."},
    {"truth", "monte carlo truth."},
    {"simple", "give back a weight based on very simple variables you can give as a parameter."},
    {"recording", "record variables to a TTree."},
    {"tmva", "use the trained BDT."},
  };
}

std::unique_ptr<BaseStereoSegmentFilter>
StereoSegmentFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseStereoSegmentFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllStereoSegmentFilter>();
  } else if (filterName == "random") {
    return makeUnique<RandomStereoSegmentFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCStereoSegmentFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleStereoSegmentFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingStereoSegmentFilter>("StereoSegment.root");
  } else if (filterName == "tmva") {
    return makeUnique<TMVAStereoSegmentFilter>("StereoSegment");
  } else {
    return Super::create(filterName);
  }
}
