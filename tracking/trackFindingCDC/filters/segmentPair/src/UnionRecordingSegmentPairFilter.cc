/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/UnionRecordingSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/segmentPair/BasicSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/filters/segmentPair/HitGapSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/SkimmedHitGapSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitlessSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/FitSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/TruthSegmentPairVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/TrailSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/filters/segmentPair/MVAFeasibleSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/MVARealisticSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::UnionRecordingFilter<SegmentPairFilterFactory>;

std::vector<std::string>
UnionRecordingSegmentPairFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {
    "basic",
    "hit_gap",
    "skimmed_hit_gap",
    "fitless",
    "feasible",
    "pre_fit",
    "fit",
    "releastic",
    "truth",
    "trail"
  });

  return varSetNames;
}

std::unique_ptr<BaseVarSet<CDCSegmentPair> >
UnionRecordingSegmentPairFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return std::make_unique<BasicSegmentPairVarSet>();
  } else if (name == "hit_gap") {
    return std::make_unique<HitGapSegmentPairVarSet>();
  } else if (name == "skimmed_hit_gap") {
    return std::make_unique<SkimmedHitGapSegmentPairVarSet>();
  } else if (name == "fitless") {
    return std::make_unique<FitlessSegmentPairVarSet>();
  } else if (name == "feasible") {
    MVAFeasibleSegmentPairFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "pre_fit") {
    const bool preliminaryFit = true;
    return std::make_unique<FitSegmentPairVarSet>(preliminaryFit);
  } else if (name == "fit") {
    return std::make_unique<FitSegmentPairVarSet>(false);
  } else if (name == "realistic") {
    MVARealisticSegmentPairFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "truth") {
    return std::make_unique<TruthSegmentPairVarSet>();
  } else if (name == "trail") {
    return std::make_unique<TrailSegmentPairVarSet>();
  } else {
    return Super::createVarSet(name);
  }
}
