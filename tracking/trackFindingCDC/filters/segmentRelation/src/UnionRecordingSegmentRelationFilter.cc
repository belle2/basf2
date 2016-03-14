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

#include <tracking/trackFindingCDC/filters/segmentRelation/CDCRecoSegment2DRelationVarSets.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


std::vector<std::string>
UnionRecordingSegmentRelationFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {});
  return varSetNames;
}


std::unique_ptr<BaseVarSet<Relation<CDCRecoSegment2D> > >
UnionRecordingSegmentRelationFilter::createVarSet(const std::string& name) const
{
  // if (name == "basic") {
  //   return std::unique_ptr<BaseVarSet<Relation<CDCRecoSegment2D> > >(new CDCSegmentRelationBasicVarSet());
  // } else if (name == "truth") {
  //   return std::unique_ptr<BaseVarSet<Relation<CDCRecoSegment2D> > >(new CDCSegmentRelationTruthVarSet());
  // } else {
  return Super::createVarSet(name);
  // }

}
