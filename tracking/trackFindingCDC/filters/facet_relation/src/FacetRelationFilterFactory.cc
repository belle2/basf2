/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/FacetRelationFilterFactory.h"
#include "../include/FacetRelationFilters.h"

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FacetRelationFilterFactory::FacetRelationFilterFactory(const std::string& defaultFilterName) :
  FilterFactory<BaseFacetRelationFilter>(defaultFilterName)
{
}

std::string FacetRelationFilterFactory::getFilterPurpose() const
{
  return "Facet relation filter to be used during the construction of the facet network.";
}

std::string FacetRelationFilterFactory::getModuleParamPrefix() const
{
  return "FacetRelation";
}

std::map<std::string, std::string>
FacetRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all facet relations are valid"},
    {"mc", "facet relations from monte carlo truth"},
    {"none", "no facet relation is valid, stop at facet creation."},
    {"recording", "record the encountered instances of facet relations"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<BaseFacetRelationFilter>
FacetRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseFacetRelationFilter>(new BaseFacetRelationFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseFacetRelationFilter>(new AllFacetRelationFilter());
  } else if (filterName == string("mc")) {
    return std::unique_ptr<BaseFacetRelationFilter>(new MCFacetRelationFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseFacetRelationFilter>(new SimpleFacetRelationFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseFacetRelationFilter>(new RecordingFacetRelationFilter());
  } else {
    return std::unique_ptr<BaseFacetRelationFilter>(nullptr);
  }
}
