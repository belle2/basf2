/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonModule.h>

#include <tracking/trackFindingCDC/filters/cluster/ChooseableClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/ChooseableFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/ChooseableFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segment_relation/ChooseableSegmentRelationFilter.h>

namespace Belle2 {

  /// Module for the cellular automaton tracking for the CDC on regular events
  class SegmentFinderCDCFacetAutomatonDevModule:
    public Belle2::TrackFindingCDC::SegmentFinderCDCFacetAutomatonImplModule <
    TrackFindingCDC::ChooseableClusterFilter,
    TrackFindingCDC::ChooseableFacetFilter,
    TrackFindingCDC::ChooseableFacetRelationFilter,
    TrackFindingCDC::ChooseableSegmentRelationFilter > {

  private:
    /// Type of the base class
    typedef SegmentFinderCDCFacetAutomatonImplModule<> Super;

  public:
    /// Constructor of the module. Setting up parameters and description.
    SegmentFinderCDCFacetAutomatonDevModule();

  }; // end class
} // end namespace Belle2
