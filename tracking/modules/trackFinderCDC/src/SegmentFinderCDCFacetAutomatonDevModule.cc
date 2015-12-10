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

SegmentFinderCDCFacetAutomatonDevModule::SegmentFinderCDCFacetAutomatonDevModule()
{
  setDescription("Versatile module with adjustable filters for segment generation.");

  // Set the default segment to symmetric
  setSegmentOrientation(ETrackOrientation::c_Symmetric);

  this->getClusterFilter()->setFilterName("all");
  this->getFacetFilter()->setFilterName("realistic");
  this->getFacetRelationFilter()->setFilterName("simple");
  this->getSegmentRelationFilter()->setFilterName("none");
}
