/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet_relation/FacetRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/facet_relation/FacetRelationFilters.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FilterFactory<Filter<Relation<CDCFacet> > >::FilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string FilterFactory<Filter<Relation<CDCFacet> > >::getFilterPurpose() const
{
  return "Facet relation filter to be used during the construction of the facet network.";
}

std::string FilterFactory<Filter<Relation<CDCFacet> > >::getModuleParamPrefix() const
{
  return "FacetRelation";
}

std::map<std::string, std::string>
FilterFactory<Filter<Relation<CDCFacet> > >::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"all", "all facet relations are valid"},
    {"truth", "facet relations from monte carlo truth"},
    {"none", "no facet relation is valid, stop at facet creation."},
    {"recording", "record the encountered instances of facet relations"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"simple", "mc free with simple criteria"},
  });

  return filterNames;
}

std::unique_ptr<Filter<Relation<CDCFacet> > >
FilterFactory<Filter<Relation<CDCFacet> > >::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<Filter<Relation<CDCFacet> > >(new BaseFacetRelationFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<Filter<Relation<CDCFacet> > >(new AllFacetRelationFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<Filter<Relation<CDCFacet> > >(new MCFacetRelationFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<Filter<Relation<CDCFacet> > >(new SimpleFacetRelationFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<Filter<Relation<CDCFacet> > >(new RecordingFacetRelationFilter());
  } else if (filterName == string("unionrecording")) {
    return std::unique_ptr<Filter<Relation<CDCFacet> > >(new UnionRecordingFacetRelationFilter());
  } else {
    return Super::create(filterName);
  }
}
