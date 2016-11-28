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

#include <tracking/trackFindingCDC/filters/facetRelation/MVAFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facetRelation/FitFacetRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/facetRelation/BendFacetRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/facetRelation/BasicFacetRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facet/FitlessFacetVarSet.h>

#include <tracking/trackFindingCDC/varsets/RelationVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::vector<std::string>
UnionRecordingFacetRelationFilter::getValidVarSetNames() const
{
  std::vector<std::string> varSetNames = Super::getValidVarSetNames();
  varSetNames.insert(varSetNames.end(), {"basic", "relation", " bend", "fit", "mva"});
  return varSetNames;
}

std::unique_ptr<BaseVarSet<Relation<const CDCFacet> > >
UnionRecordingFacetRelationFilter::createVarSet(const std::string& name) const
{
  if (name == "basic") {
    return makeUnique<BasicFacetRelationVarSet>();
  } else if (name == "relation") {
    return  makeUnique<RelationVarSet<FitlessFacetVarSet> >();
  } else if (name == "bend") {
    return makeUnique<BendFacetRelationVarSet>();
  } else if (name == "fit") {
    return makeUnique<FitFacetRelationVarSet>();
  } else if (name == "mva") {
    MVAFacetRelationFilter mvaFacetRelationFilter;
    return std::move(mvaFacetRelationFilter).releaseVarSet();
  } else {
    return Super::createVarSet(name);
  }
}
