/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/UnionRecordingSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BasicSegmentPairRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/FitSegmentPairRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/TruthSegmentPairRelationVarSet.h>

#include <tracking/trackFindingCDC/filters/segmentPairRelation/MVARealisticSegmentPairRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::UnionRecordingFilter<SegmentPairRelationFilterFactory>;

std::vector<std::string>
UnionRecordingSegmentPairRelationFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "fit", "realistic", "truth"});
  return varSetNames;
}

std::unique_ptr<BaseVarSet<Relation<const CDCSegmentPair> > >
UnionRecordingSegmentPairRelationFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return std::make_unique<BasicSegmentPairRelationVarSet>();
  } else if (name == "fit") {
    return std::make_unique<FitSegmentPairRelationVarSet>();
  } else if (name == "realistic") {
    MVARealisticSegmentPairRelationFilter filter;
    return std::move(filter).releaseVarSet();
  } else if (name == "truth") {
    return std::make_unique<TruthSegmentPairRelationVarSet>();
  }
  return Super::createVarSet(name);
}
