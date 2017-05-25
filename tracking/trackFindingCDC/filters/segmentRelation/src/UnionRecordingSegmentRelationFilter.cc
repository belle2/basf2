/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/UnionRecordingSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/segmentRelation/BasicSegmentRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/HitGapSegmentRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/FitlessSegmentRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/FitSegmentRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/segmentRelation/MVAFeasibleSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/MVARealisticSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::vector<std::string>
UnionRecordingSegmentRelationFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "hit_gap", "feasible", "fitless", "fit", "realistic"});
  return varSetNames;
}

std::unique_ptr<BaseVarSet<Relation<const CDCSegment2D> > >
UnionRecordingSegmentRelationFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return makeUnique<BasicSegmentRelationVarSet>();
  } else if (name == "hit_gap") {
    return makeUnique<HitGapSegmentRelationVarSet>();
  } else if (name == "feasible") {
    MVAFeasibleSegmentRelationFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "fitless") {
    return makeUnique<FitlessSegmentRelationVarSet>();
  } else if (name == "fit") {
    return makeUnique<FitSegmentRelationVarSet>();
  } else if (name == "realistic") {
    MVARealisticSegmentRelationFilter filter;
    return std::move(filter).releaseVarSet();
  } else {
    return Super::createVarSet(name);
  }
}
