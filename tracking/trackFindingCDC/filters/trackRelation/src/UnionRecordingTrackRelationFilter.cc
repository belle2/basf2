/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/UnionRecordingTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/trackRelation/TrackRelationVarSets.h>

#include <tracking/trackFindingCDC/filters/trackRelation/MVAFeasibleTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/MVARealisticTrackRelationFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::vector<std::string>
UnionRecordingTrackRelationFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "hit_gap", "feasible", "fit", "realistic"});
  return varSetNames;
}

std::unique_ptr<BaseVarSet<Relation<const CDCTrack> > >
UnionRecordingTrackRelationFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return makeUnique<BasicTrackRelationVarSet>();
  } else if (name == "hit_gap") {
    return makeUnique<HitGapTrackRelationVarSet>();
  } else if (name == "feasible") {
    MVAFeasibleTrackRelationFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "fit") {
    return makeUnique<FitTrackRelationVarSet>();
  } else if (name == "realistic") {
    MVARealisticTrackRelationFilter filter;
    return std::move(filter).releaseVarSet();
  } else {
    return Super::createVarSet(name);
  }
}
