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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


std::vector<std::string>
UnionRecordingSegmentPairFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"fitless", "skimmed_fitless", "fit"});
  return varSetNames;
}


std::unique_ptr<BaseVarSet<CDCSegmentPair> >
UnionRecordingSegmentPairFilter::createVarSet(const std::string& name) const
{
  if (name == "fitless") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new FitlessSegmentPairVarSet());
  } else if (name == "skimmed_fitless") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new SkimmedFitlessSegmentPairVarSet());
  } else if (name == "fit") {
    return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new FitSegmentPairVarSet());
  } else {
    return Super::createVarSet(name);
  }
}
