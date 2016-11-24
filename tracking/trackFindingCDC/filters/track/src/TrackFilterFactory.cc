/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/TrackFilterFactory.h>

#include <tracking/trackFindingCDC/filters/track/TruthTrackVarSet.h>
#include <tracking/trackFindingCDC/filters/track/BasicTrackVarSet.h>

#include <tracking/trackFindingCDC/filters/base/MVAFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCFilter.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.h>

#include <tracking/trackFindingCDC/varsets/VariadicUnionVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using AllTrackFilter = AllFilter<BaseTrackFilter>;
  using MCTrackFilter = MCFilter<VariadicUnionVarSet<TruthTrackVarSet, BasicTrackVarSet>>;
  using RecordingTrackFilter = RecordingFilter<VariadicUnionVarSet<TruthTrackVarSet, BasicTrackVarSet>>;
  using MVATrackFilter = MVAFilter<BasicTrackVarSet>;
}

TrackFilterFactory::TrackFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string TrackFilterFactory::getIdentifier() const
{
  return "Track";
}

std::string TrackFilterFactory::getFilterPurpose() const
{
  return "Track filter to reject fakes";
}

std::map<std::string, std::string>
TrackFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no track is valid"},
    {"all", "set all tracks as good"},
    {"truth", "monte carlo truth"},
    {"recording", "record variables to a TTree"},
    {"mva", "test with a mva method"}
  };
}

std::unique_ptr<BaseTrackFilter>
TrackFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseTrackFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllTrackFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCTrackFilter>();
  } else if (filterName == "recording") {
    return makeUnique<RecordingTrackFilter>("TrackFilter.root");
  } else if (filterName == "mva") {
    return makeUnique<MVATrackFilter>("tracking/data/trackfindingcdc_TrackFilter.xml");
  } else {
    return Super::create(filterName);
  }
}
