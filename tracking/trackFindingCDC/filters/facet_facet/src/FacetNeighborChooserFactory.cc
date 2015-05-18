/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/FacetNeighborChooserFactory.h"
#include "../include/FacetNeighborChoosers.h"

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FacetNeighborChooserFactory::FacetNeighborChooserFactory(const std::string& defaultFilterName) :
  FilterFactory<BaseFacetNeighborChooser>(defaultFilterName)
{
}

std::string FacetNeighborChooserFactory::getFilterPurpose() const
{
  return "Facet filter to be used during the construction of facets.";
}

std::string FacetNeighborChooserFactory::getModuleParamPrefix() const
{
  return "FacetRelation";
}

std::map<std::string, std::string>
FacetNeighborChooserFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all facet relations are valid"},
    {"mc", "facet relations from monte carlo truth"},
    {"none", "no facet relation is valid, stop at facet creation."},
    {"recording", "record the encountered instances of facet relations"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<BaseFacetNeighborChooser>
FacetNeighborChooserFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseFacetNeighborChooser>(new BaseFacetNeighborChooser());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseFacetNeighborChooser>(new AllFacetNeighborChooser());
  } else if (filterName == string("mc")) {
    return std::unique_ptr<BaseFacetNeighborChooser>(new MCFacetNeighborChooser());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseFacetNeighborChooser>(new SimpleFacetNeighborChooser());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseFacetNeighborChooser>(new RecordingFacetNeighborChooser());
  } else {
    return std::unique_ptr<BaseFacetNeighborChooser>(nullptr);
  }
}
