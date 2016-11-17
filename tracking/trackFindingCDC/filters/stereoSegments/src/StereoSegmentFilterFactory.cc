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
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using MCStereoSegmentFilter = MCFilter<StereoSegmentTruthVarSet>;
  using RecordingStereoSegmentFilter =
    RecordingFilter<VariadicUnionVarSet<StereoSegmentTruthVarSet, StereoSegmentVarSet>>;
  using AllStereoSegmentFilter = AllFilter<BaseStereoSegmentFilter>;
  using RandomStereoSegmentFilter = RandomFilter<BaseStereoSegmentFilter>;
  using MVAStereoSegmentFilter = MVAFilter<StereoSegmentVarSet>;
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
    {"mva", "use the trained BDT."},
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
    return makeUnique<RecordingStereoSegmentFilter>("StereoSegmentFilter.root");
  } else if (filterName == "mva") {
    return makeUnique<MVAStereoSegmentFilter>("tracking/data/trackfindingcdc_StereoSegmentFilter.xml");
  } else {
    return Super::create(filterName);
  }
}
