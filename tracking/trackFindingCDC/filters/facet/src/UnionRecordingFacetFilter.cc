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

#include <tracking/trackFindingCDC/filters/facet/FacetVarSets.h>

using namespace Belle2;
using namespace TrackFindingCDC;


std::vector<std::string> UnionRecordingFacetFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "fitless", "fit", "bend", "tmva", "truth"});
  return varSetNames;
}

std::unique_ptr<BaseVarSet<CDCFacet> >
UnionRecordingFacetFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new BasicFacetVarSet());
  } else if (name == "fitless") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new FitlessFacetVarSet());
  } else if (name == "bend") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new BendFacetVarSet());
  } else if (name == "fit") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new FitFacetVarSet());
  } else if (name == "tmva") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new TMVAFacetVarSet());
  } else if (name == "truth") {
    return std::unique_ptr<BaseVarSet<CDCFacet> >(new TruthFacetVarSet());
  } else {
    return Super::createVarSet(name);
  }
}
