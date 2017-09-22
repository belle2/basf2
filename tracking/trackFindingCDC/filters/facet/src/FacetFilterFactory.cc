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

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/AllFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/MCFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/FeasibleRLFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/SimpleFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/RealisticFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/Chi2FacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/UnionRecordingFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/MVAFacetFilter.h>

#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>


using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterFactory<BaseFacetFilter>;

FacetFilterFactory::FacetFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

std::string FacetFilterFactory::getIdentifier() const
{
  return "Facet";
}

std::string FacetFilterFactory::getFilterPurpose() const
{
  return "Facet filter to construct of a facet network";
}

std::map<std::string, std::string> FacetFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"none", "no facet is valid, stop at cluster generation."},
    {"all", "all facets are valid"},
    {"truth", "monte carlo truth"},
    {"feasible", "only checking the feasability of right left passage information"},
    {"simple", "mc free with simple criteria"},
    {"realistic", "mc free with realistic criteria"},
    {"realistic_loss", "mc with realistice criteria but losser cut"},
    {"chi2", "mc free based on chi2 fitting"},
    {"chi2_old", "old based on chi2 fitting with single cut regardless of superlayer"},
    {"unionrecording", "record many multiple choosable variable set"},
    {"mva", "filter facets with a mva method"},
  };
}

std::unique_ptr<BaseFacetFilter> FacetFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "none") {
    return std::make_unique<NoneFilter<BaseFacetFilter>>();
  } else if (filterName == "all") {
    return std::make_unique<AllFacetFilter>();
  } else if (filterName == "truth") {
    return std::make_unique<MCFacetFilter>();
  } else if (filterName == "feasible") {
    return std::make_unique<FeasibleRLFacetFilter>();
  } else if (filterName == "simple") {
    return std::make_unique<SimpleFacetFilter>();
  } else if (filterName == "realistic") {
    return std::make_unique<RealisticFacetFilter>();
  } else if (filterName == "realistic_loss") {
    return std::make_unique<RealisticFacetFilter>(25);
  } else if (filterName == "chi2") {
    return std::make_unique<Chi2FacetFilter>();
  } else if (filterName == "chi2_old") {
    return std::make_unique<Chi2FacetFilter>(75.0, 120.0);
  } else if (filterName == "unionrecording") {
    return std::make_unique<UnionRecordingFacetFilter>();
  } else if (filterName == "mva") {
    return std::make_unique<MVAFacetFilter>();
  } else {
    return Super::create(filterName);
  }
}
