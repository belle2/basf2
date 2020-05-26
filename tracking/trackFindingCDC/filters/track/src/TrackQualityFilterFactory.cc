/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/TrackQualityFilterFactory.h>

#include <tracking/trackFindingCDC/filters/track/BestMatchedTruthVarSet.h>
#include <tracking/trackFindingCDC/filters/track/BasicTrackVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/RecordingFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/TruthVarFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterVarSet.icc.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

// these types are the only difference to TrackFilterFactory, since here BestMatchedTruthVarSet is
// used whereas the base class uses TruthTrackVarSet
namespace {
  using AllTrackFilter = AllFilter<BaseTrackFilter>;
  using NoneTrackFilter = NoneFilter<BaseTrackFilter>;
  using MCTrackFilter = TruthVarFilter<BestMatchedTruthVarSet>;
  using RecordingTrackFilter =
    RecordingFilter<VariadicUnionVarSet<BestMatchedTruthVarSet, BasicTrackVarSet>>;
  using RecordingDataTrackFilter =
    RecordingFilter<BasicTrackVarSet>;
  using MVATrackFilter = MVAFilter<BasicTrackVarSet>;
} // namespace

template class TrackFindingCDC::FilterFactory<BaseTrackFilter>;

std::string TrackQualityFilterFactory::getFilterPurpose() const
{
  return "Track filter for getting the quality indicator for CDC tracks. It should "
         "distinguish correct PR tracks from fake and if wanted also clone tracks. ";
}

std::unique_ptr<BaseTrackFilter> TrackQualityFilterFactory::create(
  const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneTrackFilter>();
  } else if (filterName == "all") {
    return std::make_unique<AllTrackFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCTrackFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingTrackFilter>("TrackQualityFilter.root");
  } else if (filterName == "recording_data") {
    return std::make_unique<RecordingDataTrackFilter>("TrackQualityFilter.root");
  } else if (filterName == "eval") {
    auto recordedVarSets = std::make_unique<UnionVarSet<CDCTrack>>();
    using TrackFilterVarSet = FilterVarSet<BaseTrackFilter>;
    recordedVarSets->push_back(std::make_unique<TrackFilterVarSet>("mva", create("mva")));
    recordedVarSets->push_back(std::make_unique<TrackFilterVarSet>("truth", create("truth")));
    return std::make_unique<Recording<BaseTrackFilter>>(std::move(recordedVarSets),
                                                        "TrackQualityFilter_eval.root");
  } else if (filterName == "mva") {
    return std::make_unique<MVATrackFilter>("trackfindingcdc_TrackQualityIndicator", 0.10);
  } else {
    return Super::create(filterName);
  }
}
