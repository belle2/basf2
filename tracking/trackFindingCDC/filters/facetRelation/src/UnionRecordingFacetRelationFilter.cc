/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/UnionRecordingFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facetRelation/MVAFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facetRelation/FitFacetRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/facetRelation/BendFacetRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/facetRelation/BasicFacetRelationVarSet.h>
#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/facet/FitlessFacetVarSet.h>

#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.icc.h>

#include <tracking/trackFindingCDC/varsets/RelationVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::UnionRecordingFilter<FacetRelationFilterFactory>;

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
    return std::make_unique<BasicFacetRelationVarSet>();
  } else if (name == "relation") {
    return  std::make_unique<RelationVarSet<FitlessFacetVarSet> >();
  } else if (name == "bend") {
    return std::make_unique<BendFacetRelationVarSet>();
  } else if (name == "fit") {
    return std::make_unique<FitFacetRelationVarSet>();
  } else if (name == "mva") {
    MVAFacetRelationFilter mvaFacetRelationFilter;
    return std::move(mvaFacetRelationFilter).releaseVarSet();
  } else {
    return Super::createVarSet(name);
  }
}
