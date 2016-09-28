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

using namespace Belle2;
using namespace TrackFindingCDC;


FacetFilterFactory::FacetFilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string FacetFilterFactory::getFilterPurpose() const
{
  return "Facet filter to be used during the construction of facets.";
}

std::string FacetFilterFactory::getModuleParamPrefix() const
{
  return "Facet";
}

std::map<std::string, std::string>
FacetFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"all", "all facets are valid"},
    {"feasible", "only checking the feasability of right left passage information"},
    {"truth", "monte carlo truth"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"tmva", "filter facets with a tmva method"},
    {"none", "no facet is valid, stop at cluster generation."},
    {"simple", "mc free with simple criteria"},
    {"realistic", "mc free with realistic criteria"},
    {"realistic_loss", "mc with realistice criteria but losser cut"},
    {"chi2", "mc free based on chi2 fitting"},
  });
  return filterNames;
}

std::unique_ptr<Filter<CDCFacet> >
FacetFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::unique_ptr<Filter<CDCFacet> >(new BaseFacetFilter());
  } else if (filterName == "all") {
    return std::unique_ptr<Filter<CDCFacet> >(new AllFacetFilter());
  } else if (filterName == "truth") {
    return std::unique_ptr<Filter<CDCFacet> >(new MCFacetFilter());
  } else if (filterName == "feasible") {
    return std::unique_ptr<Filter<CDCFacet> >(new FeasibleRLFacetFilter());
  } else if (filterName == "simple") {
    return std::unique_ptr<Filter<CDCFacet> >(new SimpleFacetFilter());
  } else if (filterName == "realistic") {
    return std::unique_ptr<Filter<CDCFacet> >(new RealisticFacetFilter());
  } else if (filterName == "realistic_loss") {
    return std::unique_ptr<Filter<CDCFacet> >(new RealisticFacetFilter(25));
  } else if (filterName == "chi2") {
    return std::unique_ptr<Filter<CDCFacet> >(new Chi2FacetFilter());
  } else if (filterName == "tmva") {
    return std::unique_ptr<Filter<CDCFacet> >(new TMVAFacetFilter());
  } else if (filterName == "unionrecording") {
    return std::unique_ptr<Filter<CDCFacet> >(new UnionRecordingFacetFilter());
  } else {
    return Super::create(filterName);
  }
}
