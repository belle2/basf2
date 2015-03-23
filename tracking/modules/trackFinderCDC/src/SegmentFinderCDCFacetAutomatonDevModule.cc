/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonDevModule.h>

#include <tracking/trackFindingCDC/filters/facet/FacetFilters.h>
#include <tracking/trackFindingCDC/filters/facet_facet/FacetNeighborChoosers.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentFinderCDCFacetAutomatonDev);

SegmentFinderCDCFacetAutomatonDevModule::SegmentFinderCDCFacetAutomatonDevModule() :
  SegmentFinderCDCFacetAutomatonImplModule<>(c_Symmetric),
  m_param_facetFilter("simple"),
  m_param_facetNeighborChooser("simple")
{
  setDescription("Versatile module with adjustable filters for segment generation.");

  addParam("FacetFilter",
           m_param_facetFilter,
           "Facet filter to be used during the construction of facets. "
           "Valid values are: "
           "\"none\" (no facet is valid, stop at cluster generation.), "
           "\"all\" (all facets are valid), "
           "\"mc\" (monte carlo truth), "
           "\"mc_symmetric\" (monte carlo truth and their mirror image), "
           "\"fitless\" (only checking the feasability of right left passage information), "
           "\"fitless_hard\" (also exclude the boarder line feasable combinations), "
           "\"simple\" (mc free with simple criteria)."
           "\"realistic\" (mc free with more realistic criteria).",
           string("simple"));

  addParam("FacetNeighborChooser",
           m_param_facetNeighborChooser,
           "Facet neighbor chooser to be used during the construction of the graph. "
           "Valid values are: "
           "\"none\" (no neighbor is correct, stops segment generation), "
           "\"all\" (all possible neighbors are valid), "
           "\"mc\" (monte carlo truth), "
           "\"mc_symmetric\" (monte carlo truth and the reversed version are excepted), "
           "\"simple\" (mc free with simple criteria).",
           string("simple"));
}

void SegmentFinderCDCFacetAutomatonDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  BaseFacetFilter* ptrFacetFilter = nullptr;

  if (m_param_facetFilter == string("none")) {
    ptrFacetFilter = new BaseFacetFilter();
  } else if (m_param_facetFilter == string("all")) {
    ptrFacetFilter = new AllFacetFilter();
  } else if (m_param_facetFilter == string("mc")) {
    ptrFacetFilter = new MCFacetFilter(false);
  } else if (m_param_facetFilter == string("mc_symmetric")) {
    ptrFacetFilter = new MCFacetFilter(true);
  } else if (m_param_facetFilter == string("fitless")) {
    ptrFacetFilter = new FitlessFacetFilter(false);
  } else if (m_param_facetFilter == string("fitless_hard")) {
    ptrFacetFilter = new FitlessFacetFilter(true);
  } else if (m_param_facetFilter == string("simple")) {
    ptrFacetFilter = new SimpleFacetFilter();
  } else if (m_param_facetFilter == string("realistic")) {
    ptrFacetFilter = new RealisticFacetFilter();
  } else {
    B2ERROR("Unrecognised FacetFilter option " << m_param_facetFilter <<
            ". Allowed values are " <<
            "\"all\", " <<
            "\"mc\", " <<
            "\"mc_symmetric\", " <<
            "\"fitless\", " <<
            "\"fitless_hard\", " <<
            "\"realistic\" or " <<
            "\"simple\"."
           );
  }

  if (ptrFacetFilter) {
    // Takes ownership
    setFacetFilter(ptrFacetFilter);
  }

  BaseFacetNeighborChooser* ptrFacetNeighborChooser = nullptr;
  if (m_param_facetNeighborChooser == string("none")) {
    ptrFacetNeighborChooser = new BaseFacetNeighborChooser();
  } else if (m_param_facetNeighborChooser == string("all")) {
    ptrFacetNeighborChooser = new AllFacetNeighborChooser();
  } else if (m_param_facetNeighborChooser == string("mc")) {
    ptrFacetNeighborChooser = new MCFacetNeighborChooser(false);
  } else if (m_param_facetNeighborChooser == string("mc_symmetric")) {
    ptrFacetNeighborChooser = new MCFacetNeighborChooser(true);
  } else if (m_param_facetNeighborChooser == string("simple")) {
    ptrFacetNeighborChooser = new SimpleFacetNeighborChooser();
  } else {
    B2ERROR("Unrecognised FacetNeighborChooser option " << m_param_facetNeighborChooser <<
            ". Allowed values are \"none\", \"all\", \"mc\", \"mc_symmetric\" and \"simple\".");
  }
  if (ptrFacetNeighborChooser) {
    // Takes ownership
    setFacetNeighborChooser(ptrFacetNeighborChooser);
  }

  SegmentFinderCDCFacetAutomatonImplModule<>::initialize();

  if ((m_param_facetFilter == string("mc")) or
      (m_param_facetFilter == string("mc_symmetric")) or
      (m_param_facetNeighborChooser == string("mc")) or
      (m_param_facetNeighborChooser == string("mc_symmetric"))) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }

}


void SegmentFinderCDCFacetAutomatonDevModule::event()
{
  if ((m_param_facetFilter == string("mc")) or
      (m_param_facetFilter == string("mc_symmetric")) or
      (m_param_facetNeighborChooser == string("mc")) or
      (m_param_facetNeighborChooser == string("mc_symmetric"))) {

    CDCMCManager::getInstance().fill();
  }

  SegmentFinderCDCFacetAutomatonImplModule<>::event();
}
