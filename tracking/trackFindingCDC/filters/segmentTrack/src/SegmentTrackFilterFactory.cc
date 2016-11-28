/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segmentTrack/SimpleSegmentTrackFilter.h>

#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackVarSet.h>

#include <tracking/trackFindingCDC/filters/base/AllFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllSegmentTrackFilter = AllFilter<BaseSegmentTrackFilter>;
  using MCSegmentTrackFilter = MCFilter<VariadicUnionVarSet<SegmentTrackTruthVarSet, SegmentTrackVarSet>>;
  using RecordingSegmentTrackFilter = RecordingFilter<VariadicUnionVarSet<SegmentTrackTruthVarSet, SegmentTrackVarSet>>;
  using MVASegmentTrackFilter = MVAFilter<SegmentTrackVarSet>;
}

SegmentTrackFilterFactory::SegmentTrackFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::map<std::string, std::string>
SegmentTrackFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no segment track combination is valid"},
    {"truth", "monte carlo truth"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseSegmentTrackFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentTrackFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleSegmentTrackFilter>();
  } else {
    return Super::create(filterName);
  }
}

// ------ First step ------
SegmentTrackFilterFirstStepFactory::SegmentTrackFilterFirstStepFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentTrackFilterFirstStepFactory::getIdentifier() const
{
  return "SegmentTrackFilterFirstStep";
}

std::string SegmentTrackFilterFirstStepFactory::getFilterPurpose() const
{
  return "Segment track chooser to be used during the combination of segment track pairs";
}

std::map<std::string, std::string>
SegmentTrackFilterFirstStepFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string> filterNames = Super::getValidFilterNamesAndDescriptions();
  filterNames.insert({
    {"mva", "test with a mva method"},
    {"recording", "record variables to a TTree"},
  });
  return filterNames;
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterFirstStepFactory::create(const std::string& filterName) const
{
  if (filterName == "mva") {
    return makeUnique<MVASegmentTrackFilter>("tracking/data/trackfindingcdc_SegmentTrackFilterFirstStep.xml");
  } else if (filterName == "recording") {
    return makeUnique<RecordingSegmentTrackFilter>("SegmentTrackFilterFirstStep.root");
  } else {
    return Super::create(filterName);
  }
}

// ------ Second step ------
SegmentTrackFilterSecondStepFactory::SegmentTrackFilterSecondStepFactory(
  const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string SegmentTrackFilterSecondStepFactory::getIdentifier() const
{
  return "SegmentTrackFilterSecondStep";
}

std::string SegmentTrackFilterSecondStepFactory::getFilterPurpose() const
{
  return "Segment track chooser to be used during the matching of segment track pairs.";
}

std::map<std::string, std::string>
SegmentTrackFilterSecondStepFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string> filterNames = Super::getValidFilterNamesAndDescriptions();
  filterNames.insert({
    {"mva", "test with a mva method"},
    {"recording", "record variables to a TTree"},
  });
  return filterNames;
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterSecondStepFactory::create(const std::string& filterName) const
{
  if (filterName == "mva") {
    return makeUnique<MVASegmentTrackFilter>("tracking/data/trackfindingcdc_SegmentTrackFilterSecondStep.xml");
  } else if (filterName ==  "recording") {
    return makeUnique<RecordingSegmentTrackFilter>("SegmentTrackFilterSecondStep.root");
  } else {
    return Super::create(filterName);
  }
}
