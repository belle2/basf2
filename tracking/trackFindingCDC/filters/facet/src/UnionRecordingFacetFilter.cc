/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/UnionRecordingFacetFilter.h>

#include <tracking/trackFindingCDC/filters/facet/CDCFacetVarSets.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


std::vector<std::string> UnionRecordingFacetFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"fitless", "fit", "truth"});
  return varSetNames;
}


std::unique_ptr<BaseVarSet<CDCFacet> >
UnionRecordingFacetFilter::createVarSet(const std::string& name) const
{
  if (name == "fitless") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new CDCFacetFitlessVarSet());
  } else if (name == "fit") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new CDCFacetFitVarSet());
  } else if (name == "truth") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new CDCFacetTruthVarSet());
  } else {
    return Super::createVarSet(name);
  }
}
