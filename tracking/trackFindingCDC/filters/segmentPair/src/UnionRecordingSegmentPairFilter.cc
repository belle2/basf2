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

#include <tracking/trackFindingCDC/filters/segmentPair/SegmentPairVarSets.h>

#include <tracking/trackFindingCDC/filters/segmentPair/MVAFeasibleSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/MVARealisticSegmentPairFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::vector<std::string>
UnionRecordingSegmentPairFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {
    "basic",
    "hit_gap", "skimmed_hit_gap",
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
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new BasicSegmentPairVarSet());
  } else if (name == "hit_gap") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new HitGapSegmentPairVarSet());
  } else if (name == "skimmed_hit_gap") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new SkimmedHitGapSegmentPairVarSet());
  } else if (name == "fitless") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new FitlessSegmentPairVarSet());
  } else if (name == "feasible") {
    MVAFeasibleSegmentPairFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "pre_fit") {
    bool preliminaryFit = true;
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new FitSegmentPairVarSet(preliminaryFit));
  } else if (name == "fit") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new FitSegmentPairVarSet(false));
  } else if (name == "realistic") {
    MVARealisticSegmentPairFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "truth") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new TruthSegmentPairVarSet());
  } else if (name == "trail") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new TrailSegmentPairVarSet());
  } else {
    return Super::createVarSet(name);
  }
}
