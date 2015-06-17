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
#include <tracking/trackFindingCDC/filters/facet_relation/FacetRelationFilters.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentFinderCDCFacetAutomatonDev);

SegmentFinderCDCFacetAutomatonDevModule::SegmentFinderCDCFacetAutomatonDevModule() :
  Super(c_Symmetric),
  m_clusterFilterFactory("all"),
  m_facetFilterFactory("realistic"),
  m_facetRelationFilterFilterFactory("simple"),
  m_segmentRelationFilterFilterFactory("none")
{
  setDescription("Versatile module with adjustable filters for segment generation.");

  m_clusterFilterFactory.exposeParameters(this);
  m_facetFilterFactory.exposeParameters(this);
  m_facetRelationFilterFilterFactory.exposeParameters(this);
  m_segmentRelationFilterFilterFactory.exposeParameters(this);
}

void SegmentFinderCDCFacetAutomatonDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseClusterFilter> ptrClusterFilter = m_clusterFilterFactory.create();
  setClusterFilter(std::move(ptrClusterFilter));

  std::unique_ptr<BaseFacetFilter> ptrFacetFilter = m_facetFilterFactory.create();
  setFacetFilter(std::move(ptrFacetFilter));

  std::unique_ptr<BaseFacetRelationFilter>
  ptrFacetRelationFilter = m_facetRelationFilterFilterFactory.create();
  setFacetRelationFilter(std::move(ptrFacetRelationFilter));

  std::unique_ptr<BaseSegmentRelationFilter>
  ptrSegmentRelationFilter = m_segmentRelationFilterFilterFactory.create();
  setSegmentRelationFilter(std::move(ptrSegmentRelationFilter));

  Super::initialize();

  if (getClusterFilter()->needsTruthInformation() or
      getFacetFilter()->needsTruthInformation() or
      getFacetRelationFilter()->needsTruthInformation() or
      getSegmentRelationFilter()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}

void SegmentFinderCDCFacetAutomatonDevModule::event()
{
  if (getClusterFilter()->needsTruthInformation() or
      getFacetFilter()->needsTruthInformation() or
      getFacetRelationFilter()->needsTruthInformation() or
      getSegmentRelationFilter()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  Super::event();
}
