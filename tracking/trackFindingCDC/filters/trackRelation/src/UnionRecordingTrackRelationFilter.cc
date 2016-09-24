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

#include <tracking/trackFindingCDC/filters/trackRelation/CDCTrackRelationVarSets.h>

using namespace Belle2;
using namespace TrackFindingCDC;


std::vector<std::string>
UnionRecordingTrackRelationFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {});
  return varSetNames;
}


std::unique_ptr<BaseVarSet<Relation<const CDCTrack> > >
UnionRecordingTrackRelationFilter::createVarSet(const std::string& name) const
{
  // if (name == "basic") {
  //   return std::unique_ptr<BaseVarSet<Relation<const CDCTrack> > >(new CDCTrackRelationBasicVarSet());
  // } else if (name == "truth") {
  //   return std::unique_ptr<BaseVarSet<Relation<const CDCTrack> > >(new CDCTrackRelationTruthVarSet());
  // } else {
  return Super::createVarSet(name);
  // }

}
