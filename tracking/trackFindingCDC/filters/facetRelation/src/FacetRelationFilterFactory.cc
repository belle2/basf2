/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/FacetRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/AllFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/MCFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/SimpleFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/Chi2FacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/UnionRecordingFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/MVAFacetRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseFacetRelationFilter>;

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
    return std::make_unique<NoneFilter<BaseFacetRelationFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllFacetRelationFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCFacetRelationFilter>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleFacetRelationFilter>();
  } else if (filterName == "chi2") {
    return std::make_unique<Chi2FacetRelationFilter>();
  } else if (filterName == "chi2_old") {
    return std::make_unique<Chi2FacetRelationFilter>(130.0, 200.0);
  } else if (filterName == "unionrecording") {
    return std::make_unique<UnionRecordingFacetRelationFilter>();
  } else if (filterName == "mva") {
    return std::make_unique<MVAFacetRelationFilter>();
  } else {
    return Super::create(filterName);
  }
}
