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

using namespace Belle2;
using namespace TrackFindingCDC;

FacetRelationFilterFactory::FacetRelationFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string FacetRelationFilterFactory::getIdentifier() const
{
  return "FacetRelation";
}

std::string FacetRelationFilterFactory::getFilterPurpose() const
{
  return "Facet relation filter to construct of a facet network";
}

std::map<std::string, std::string>
FacetRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no facet relation is valid, stop at facet creation."},
    {"all", "all facet relations are valid"},
    {"truth", "facet relations from monte carlo truth"},
    {"simple", "mc free with simple criteria"},
    {"chi2", "mc free based on chi2 fitting"},
    {"unionrecording", "record multiple choosable variable sets"},
    {"tmva", "filter facets with a tmva method"},
  };
}

std::unique_ptr<BaseFacetRelationFilter >
FacetRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<BaseFacetRelationFilter>();
  } else if (filterName == "all") {
    return makeUnique<AllFacetRelationFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCFacetRelationFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleFacetRelationFilter>();
  } else if (filterName == "chi2") {
    return makeUnique<Chi2FacetRelationFilter>();
  } else if (filterName == "unionrecording") {
    return makeUnique<UnionRecordingFacetRelationFilter>();
  } else if (filterName == "tmva") {
    return makeUnique<TMVAFacetRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
