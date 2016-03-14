/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/cluster/UnionRecordingClusterFilter.h>

#include <tracking/trackFindingCDC/filters/cluster/CDCWireHitClusterVarSets.h>


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

UnionRecordingClusterFilter::UnionRecordingClusterFilter() : Super("BackgroundHitFinder.root")
{;}

std::vector<std::string> UnionRecordingClusterFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "truth"});
  return varSetNames;
}

std::unique_ptr<BaseVarSet<CDCWireHitCluster> >
UnionRecordingClusterFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return std::unique_ptr<BaseVarSet<CDCWireHitCluster> >(new CDCWireHitClusterBasicVarSet());
  } else if (name == "truth") {
    return std::unique_ptr<BaseVarSet<CDCWireHitCluster> >(new CDCWireHitClusterBkgTruthVarSet());
  } else {
    return Super::createVarSet(name);
  }
}
