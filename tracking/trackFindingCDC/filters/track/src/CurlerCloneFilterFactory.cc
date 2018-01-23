/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun, Michael Eliachevitch           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/CurlerCloneFilterFactory.h>

#include <tracking/trackFindingCDC/filters/track/TruthTrackVarSet.h>
#include <tracking/trackFindingCDC/filters/track/CurlerCloneTruthVarSet.h>
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

namespace {
  using AllTrackFilter = AllFilter<BaseTrackFilter>;
  using NoneTrackFilter = NoneFilter<BaseTrackFilter>;
  using MCTrackFilter = TruthVarFilter<CurlerCloneTruthVarSet>;
  using RecordingTrackFilter = RecordingFilter<VariadicUnionVarSet<CurlerCloneTruthVarSet, BasicTrackVarSet>>;
  using MVATrackFilter = MVAFilter<BasicTrackVarSet>;
}

template class TrackFindingCDC::FilterFactory<BaseTrackFilter>;

CurlerCloneFilterFactory::CurlerCloneFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string CurlerCloneFilterFactory::getIdentifier() const
{
  return "Track";
}

std::string CurlerCloneFilterFactory::getFilterPurpose() const
{
  return "Track filter to reject fakes";
}

std::map<std::string, std::string>
CurlerCloneFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track is valid"},
    {"all", "set all tracks as good"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"eval", "record truth and the mva response for insitu comparision"},
    {"mva", "test with a mva method"}
  };
}

std::unique_ptr<BaseTrackFilter>
CurlerCloneFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneTrackFilter>();
  } else if (filterName == "all") {
    return std::make_unique<AllTrackFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCTrackFilter>();
  } else if (filterName == "recording") {
    return std::make_unique<RecordingTrackFilter>("CurlerCloneFilter.root");
  } else if (filterName == "eval") {
    auto recordedVarSets = std::make_unique<UnionVarSet<CDCTrack>>();
    using TrackFilterVarSet = FilterVarSet<BaseTrackFilter>;
    recordedVarSets->push_back(std::make_unique<TrackFilterVarSet>("mva", create("mva")));
    recordedVarSets->push_back(std::make_unique<TrackFilterVarSet>("truth", create("truth")));
    return std::make_unique<Recording<BaseTrackFilter>>(std::move(recordedVarSets), "CurlerClone_eval.root");
  } else if (filterName == "mva") {
    return std::make_unique<MVATrackFilter>("tracking/data/trackfindingcdc_CurlerClone.xml", 0.10);
  } else {
    return Super::create(filterName);
  }
}
