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

#include <tracking/trackFindingCDC/filters/cluster/ClusterFilters.h>
#include <tracking/trackFindingCDC/filters/facet/FacetFilters.h>
#include <tracking/trackFindingCDC/filters/facet_facet/FacetNeighborChoosers.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentFinderCDCFacetAutomatonDev);

SegmentFinderCDCFacetAutomatonDevModule::SegmentFinderCDCFacetAutomatonDevModule() :
  SegmentFinderCDCFacetAutomatonImplModule<>(c_Symmetric),
  m_param_clusterFilter("all"),
  m_param_facetFilter("realistic"),
  m_param_facetNeighborChooser("simple")
{
  setDescription("Versatile module with adjustable filters for segment generation.");

  addParam("ClusterFilter",
           m_param_clusterFilter,
           "Cluster filter investigates hit clusters and lets only hits pass "
           "that are not background."
           "Valid values are: "
           "\"all\" (all hits are valid), "
           "\"tmva\" (test clusters for background with a tmva method)."
           "\"recording\" (record cluster variables to a TTree).",
           string("all"));

  addParam("ClusterFilterParameters",
           m_param_clusterFilterParameters,
           "Key - Value pairs depending on the cluster filter",
           map<string, string>());

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
           "\"realistic\" (mc free with more realistic criteria)."
           "\"recording\" (record the encountered instances of facets including truth information)",
           string("realistic"));

  addParam("FacetFilterParameters",
           m_param_facetFilterParameters,
           "Key - Value pairs depending on the facet filter",
           map<string, string>());

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

  addParam("FacetNeighborChooserParameters",
           m_param_facetNeighborChooserParameters,
           "Key - Value pairs depending on the facet neighbor chooser",
           map<string, string>());
}

void SegmentFinderCDCFacetAutomatonDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseClusterFilter> ptrClusterFilter(new BaseClusterFilter());

  if (m_param_clusterFilter == string("all")) {
    ptrClusterFilter.reset(new AllClusterFilter());
  } else if (m_param_clusterFilter == string("tmva")) {
    ptrClusterFilter.reset(new TMVAClusterFilter());
  } else if (m_param_clusterFilter == string("recording")) {
    ptrClusterFilter.reset(new RecordingClusterFilter());
  } else {
    B2ERROR("Unrecognised ClusterFilter option " << m_param_clusterFilter <<
            ". Allowed values are " <<
            "\"all\", " <<
            "\"recording\", " <<
            "\"tmva\"."
           );
  }

  // Takes ownership
  setClusterFilter(std::move(ptrClusterFilter));
  getClusterFilter()->setParameters(m_param_clusterFilterParameters);

  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseFacetFilter> ptrFacetFilter(new BaseFacetFilter());;

  if (m_param_facetFilter == string("none")) {
    ptrFacetFilter.reset(new BaseFacetFilter());
  } else if (m_param_facetFilter == string("all")) {
    ptrFacetFilter.reset(new AllFacetFilter());
  } else if (m_param_facetFilter == string("mc")) {
    ptrFacetFilter.reset(new MCFacetFilter());
  } else if (m_param_facetFilter == string("fitless")) {
    ptrFacetFilter.reset(new FitlessFacetFilter());
  } else if (m_param_facetFilter == string("simple")) {
    ptrFacetFilter.reset(new SimpleFacetFilter());
  } else if (m_param_facetFilter == string("realistic")) {
    ptrFacetFilter.reset(new RealisticFacetFilter());
  } else if (m_param_facetFilter == string("recording")) {
    ptrFacetFilter.reset(new RecordingFacetFilter());
  } else {
    B2ERROR("Unrecognised FacetFilter option " << m_param_facetFilter <<
            ". Allowed values are " <<
            "\"all\", " <<
            "\"mc\", " <<
            "\"fitless\", " <<
            "\"realistic\" or " <<
            "\"simple\"."
           );
  }

  // Takes ownership
  setFacetFilter(std::move(ptrFacetFilter));
  getFacetFilter()->setParameters(m_param_facetFilterParameters);

  std::unique_ptr<BaseFacetNeighborChooser> ptrFacetNeighborChooser(new BaseFacetNeighborChooser());
  if (m_param_facetNeighborChooser == string("none")) {
    ptrFacetNeighborChooser.reset(new BaseFacetNeighborChooser());
  } else if (m_param_facetNeighborChooser == string("all")) {
    ptrFacetNeighborChooser.reset(new AllFacetNeighborChooser());
  } else if (m_param_facetNeighborChooser == string("mc")) {
    ptrFacetNeighborChooser.reset(new MCFacetNeighborChooser());
  } else if (m_param_facetNeighborChooser == string("simple")) {
    ptrFacetNeighborChooser.reset(new SimpleFacetNeighborChooser());
  } else {
    B2ERROR("Unrecognised FacetNeighborChooser option " << m_param_facetNeighborChooser <<
            ". Allowed values are \"none\", \"all\", \"mc\", and \"simple\".");
  }

  // Takes ownership
  setFacetNeighborChooser(std::move(ptrFacetNeighborChooser));
  getFacetNeighborChooser()->setParameters(m_param_facetNeighborChooserParameters);
  SegmentFinderCDCFacetAutomatonImplModule<>::initialize();

  if (getClusterFilter()->needsTruthInformation() or
      getFacetFilter()->needsTruthInformation() or
      getFacetNeighborChooser()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}


void SegmentFinderCDCFacetAutomatonDevModule::event()
{
  if (getClusterFilter()->needsTruthInformation() or
      getFacetFilter()->needsTruthInformation() or
      getFacetNeighborChooser()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  SegmentFinderCDCFacetAutomatonImplModule<>::event();
}
