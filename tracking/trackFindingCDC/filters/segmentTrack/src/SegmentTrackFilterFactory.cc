/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackFilterFactory.h>

#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentTrack/SegmentTrackVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterVarSet.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllSegmentTrackFilter = AllFilter<BaseSegmentTrackFilter>;
  using NoneSegmentTrackFilter = NoneFilter<BaseSegmentTrackFilter>;
  using MCSegmentTrackFilter = TruthVarFilter<SegmentTrackTruthVarSet>;
  using RecordingSegmentTrackFilter = RecordingFilter<VariadicUnionVarSet<SegmentTrackTruthVarSet, SegmentTrackVarSet>>;
  using MVASegmentTrackFilter = MVAFilter<SegmentTrackVarSet>;
}

template class TrackFindingCDC::FilterFactory<BaseSegmentTrackFilter>;

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
    return std::make_unique<NoneSegmentTrackFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCSegmentTrackFilter>();
  } else if (filterName == "mva") {
    return std::make_unique<MVASegmentTrackFilter>("trackfindingcdc_SegmentTrackFilter", 0.74);
  } else if (filterName == "eval") {
    auto recordedVarSets = std::make_unique<UnionVarSet<BaseSegmentTrackFilter::Object>>();
    using TrackFilterVarSet = FilterVarSet<BaseSegmentTrackFilter>;
    recordedVarSets->push_back(std::make_unique<TrackFilterVarSet>("mva", create("mva")));
    recordedVarSets->push_back(std::make_unique<TrackFilterVarSet>("truth", create("truth")));
    return std::make_unique<Recording<BaseSegmentTrackFilter>>(std::move(recordedVarSets), "SegmentTrackFilter_eval.root");
  } else if (filterName == "recording") {
    return std::make_unique<RecordingSegmentTrackFilter>("SegmentTrackFilter.root");
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
