/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "all") {
    return std::make_unique<AllFacetFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "truth") {
    return std::make_unique<MCFacetFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "feasible") {
    return std::make_unique<FeasibleRLFacetFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "simple") {
    return std::make_unique<SimpleFacetFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "realistic") {
    return std::make_unique<RealisticFacetFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "realistic_loss") {
    return std::make_unique<RealisticFacetFilter>(25);
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "chi2") {
    return std::make_unique<Chi2FacetFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "chi2_old") {
    return std::make_unique<Chi2FacetFilter>(75.0, 120.0);
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "unionrecording") {
    return std::make_unique<UnionRecordingFacetFilter>();
    // cppcheck-suppress knownConditionTrueFalse
  } else if (filterName == "mva") {
    return std::make_unique<MVAFacetFilter>();
  } else {
    return Super::create(filterName);
  }
}
