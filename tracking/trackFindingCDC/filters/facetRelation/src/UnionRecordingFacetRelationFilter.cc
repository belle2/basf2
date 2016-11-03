/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/UnionRecordingFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facetRelation/FacetRelationVarSets.h>

using namespace Belle2;
using namespace TrackFindingCDC;


std::vector<std::string>
UnionRecordingFacetRelationFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "truth"});
  return varSetNames;
}


std::unique_ptr<BaseVarSet<Relation<const CDCFacet> > >
UnionRecordingFacetRelationFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return makeUnique<BasicFacetRelationVarSet>();
  } else if (name == "bend") {
    return makeUnique<BendFacetRelationVarSet>();
  } else if (name == "fit") {
    return makeUnique<FitFacetRelationVarSet>();
  } else if (name == "tmva") {
    return makeUnique<TMVAFacetRelationVarSet>();
  } else if (name == "truth") {
    return makeUnique<TruthFacetRelationVarSet>();
  } else {
    return Super::createVarSet(name);
  }
}
