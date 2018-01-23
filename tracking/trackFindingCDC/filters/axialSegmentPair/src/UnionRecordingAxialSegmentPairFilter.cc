/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/UnionRecordingAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BasicAxialSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/HitGapAxialSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/FitlessAxialSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/FitAxialSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/MVAFeasibleAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/MVARealisticAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::UnionRecordingFilter<AxialSegmentPairFilterFactory>;

std::vector<std::string>
UnionRecordingAxialSegmentPairFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "hit_gap", "feasible", "fitless", "fit", "realistic"});
  return varSetNames;
}

std::unique_ptr<BaseVarSet<CDCAxialSegmentPair> >
UnionRecordingAxialSegmentPairFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return std::make_unique<BasicAxialSegmentPairVarSet>();
  } else if (name == "hit_gap") {
    return std::make_unique<HitGapAxialSegmentPairVarSet>();
  } else if (name == "feasible") {
    MVAFeasibleAxialSegmentPairFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "fitless") {
    return std::make_unique<FitlessAxialSegmentPairVarSet>();
  } else if (name == "fit") {
    return std::make_unique<FitAxialSegmentPairVarSet>();
  } else if (name == "realistic") {
    MVARealisticAxialSegmentPairFilter filter;
    return std::move(filter).releaseVarSet();
  } else {
    return Super::createVarSet(name);
  }
}
