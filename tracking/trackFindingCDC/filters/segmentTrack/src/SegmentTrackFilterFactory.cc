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
    {"mva", "test with a mva method"},
    {"recording", "record variables to a TTree"},
  };
}

std::unique_ptr<BaseSegmentTrackFilter>
SegmentTrackFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseSegmentTrackFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCSegmentTrackFilter>();
  } else if (filterName == "mva") {
    return makeUnique<MVASegmentTrackFilter>("tracking/data/trackfindingcdc_SegmentTrackFilter.xml");
  } else if (filterName == "recording") {
    return makeUnique<RecordingSegmentTrackFilter>("SegmentTrackFilter.root");
  } else {
    return Super::create(filterName);
  }
}

std::string SegmentTrackFilterFactory::getFilterPurpose() const
{
  return "Segment track chooser to be used during the combination of segment track pairs";
}

std::string SegmentTrackFilterFactory::getIdentifier() const
{
  return "SegmentTrackFilter";
}
