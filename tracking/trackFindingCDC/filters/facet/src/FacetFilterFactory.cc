/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/FacetFilterFactory.h>
#include <tracking/trackFindingCDC/filters/facet/FacetFilters.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FilterFactory<Filter<CDCFacet> >::FilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string FilterFactory<Filter<CDCFacet> >::getFilterPurpose() const
{
  return "Facet filter to be used during the construction of facets.";
}

std::string FilterFactory<Filter<CDCFacet> >::getModuleParamPrefix() const
{
  return "Facet";
}

std::map<std::string, std::string>
FilterFactory<Filter<CDCFacet> >::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"all", "all facets are valid"},
    {"fitless", "only checking the feasability of right left passage information"},
    {"truth", "monte carlo truth"},
    {"tmva", "filter facets with a tmva method"},
    {"none", "no facet is valid, stop at cluster generation."},
    {"realistic", "mc free with more realistic criteria"},
    {"recording", "record the encountered instances of facets"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"simple", "mc free with simple criteria"},
  });
  return filterNames;
}

std::unique_ptr<Filter<CDCFacet> >
FilterFactory<Filter<CDCFacet> >::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<Filter<CDCFacet> >(new BaseFacetFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<Filter<CDCFacet> >(new AllFacetFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<Filter<CDCFacet> >(new MCFacetFilter());
  } else if (filterName == string("fitless")) {
    return std::unique_ptr<Filter<CDCFacet> >(new FitlessFacetFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<Filter<CDCFacet> >(new SimpleFacetFilter());
  } else if (filterName == string("realistic")) {
    return std::unique_ptr<Filter<CDCFacet> >(new RealisticFacetFilter());
  } else if (filterName == string("tmva")) {
    return std::unique_ptr<Filter<CDCFacet> >(new TMVAFacetFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<Filter<CDCFacet> >(new RecordingFacetFilter());
  } else if (filterName == string("unionrecording")) {
    return std::unique_ptr<Filter<CDCFacet> >(new UnionRecordingFacetFilter());
  } else {
    return Super::create(filterName);
  }
}
