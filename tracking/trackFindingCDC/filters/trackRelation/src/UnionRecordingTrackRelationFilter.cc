/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/UnionRecordingTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/trackRelation/MVAFeasibleTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/MVARealisticTrackRelationFilter.h>

#include <tracking/trackFindingCDC/filters/trackRelation/BasicTrackRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/trackRelation/HitGapTrackRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/trackRelation/FitTrackRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::UnionRecordingFilter<TrackRelationFilterFactory>;

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
    return std::make_unique<BasicTrackRelationVarSet>();
  } else if (name == "hit_gap") {
    return std::make_unique<HitGapTrackRelationVarSet>();
  } else if (name == "feasible") {
    MVAFeasibleTrackRelationFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "fit") {
    return std::make_unique<FitTrackRelationVarSet>();
  } else if (name == "realistic") {
    MVARealisticTrackRelationFilter filter;
    return std::move(filter).releaseVarSet();
  } else {
    return Super::createVarSet(name);
  }
}
