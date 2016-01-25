/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/FacetRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/facetRelation/FacetRelationFilters.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FilterFactory<Filter<Relation<const CDCFacet> > >::FilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string FilterFactory<Filter<Relation<const CDCFacet> > >::getFilterPurpose() const
{
  return "Facet relation filter to be used during the construction of the facet network.";
}

std::string FilterFactory<Filter<Relation<const CDCFacet> > >::getModuleParamPrefix() const
{
  return "FacetRelation";
}

std::map<std::string, std::string>
FilterFactory<Filter<Relation<const CDCFacet> > >::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"all", "all facet relations are valid"},
    {"truth", "facet relations from monte carlo truth"},
    {"simple", "mc free with simple criteria"},
    {"tmva", "filter facets with a tmva method"},
    {"none", "no facet relation is valid, stop at facet creation."},
    {"unionrecording", "record multiple choosable variable sets"},
  });

  return filterNames;
}

std::unique_ptr<Filter<Relation<const CDCFacet> > >
FilterFactory<Filter<Relation<const CDCFacet> > >::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<Filter<Relation<const CDCFacet> > >(new BaseFacetRelationFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<Filter<Relation<const CDCFacet> > >(new AllFacetRelationFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<Filter<Relation<const CDCFacet> > >(new MCFacetRelationFilter());
  } else if (filterName == string("tmva")) {
    return std::unique_ptr<Filter<Relation<const CDCFacet> > >(new TMVAFacetRelationFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<Filter<Relation<const CDCFacet> > >(new SimpleFacetRelationFilter());
  } else if (filterName == string("unionrecording")) {
    return std::unique_ptr<Filter<Relation<const CDCFacet> > >(new UnionRecordingFacetRelationFilter());
  } else {
    return Super::create(filterName);
  }
}
