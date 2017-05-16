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

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/AllFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/MCFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/SimpleFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/Chi2FacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/UnionRecordingFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/MVAFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

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
    {"chi2_old", "old based on chi2 fitting with single cut regardless of superlayer"},
    {"unionrecording", "record multiple choosable variable sets"},
    {"mva", "filter facets with a mva method"},
  };
}

std::unique_ptr<BaseFacetRelationFilter >
FacetRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return makeUnique<NoneFilter<BaseFacetRelationFilter>>();
  } else if (filterName == "all") {
    return makeUnique<AllFacetRelationFilter>();
  } else if (filterName == "truth") {
    return makeUnique<MCFacetRelationFilter>();
  } else if (filterName == "simple") {
    return makeUnique<SimpleFacetRelationFilter>();
  } else if (filterName == "chi2") {
    return makeUnique<Chi2FacetRelationFilter>();
  } else if (filterName == "chi2_old") {
    return makeUnique<Chi2FacetRelationFilter>(130.0, 200.0);
  } else if (filterName == "unionrecording") {
    return makeUnique<UnionRecordingFacetRelationFilter>();
  } else if (filterName == "mva") {
    return makeUnique<MVAFacetRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
