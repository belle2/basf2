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

#include <tracking/trackFindingCDC/filters/cluster/ClusterFilterFactory.h>
#include <tracking/trackFindingCDC/filters/facet/FacetFilterFactory.h>
#include <tracking/trackFindingCDC/filters/facet_relation/FacetRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segment_relation/SegmentRelationFilterFactory.h>

namespace Belle2 {

  /// Module for the cellular automaton tracking for the CDC on regular events
  class SegmentFinderCDCFacetAutomatonDevModule:
    public Belle2::TrackFindingCDC::SegmentFinderCDCFacetAutomatonImplModule<> {

  private:
    /// Type of the base class
    typedef SegmentFinderCDCFacetAutomatonImplModule<> Super;

  public:
    /// Constructor of the module. Setting up parameters and description.
    SegmentFinderCDCFacetAutomatonDevModule();

    /// Initialize the Module before event processing
    virtual void initialize() override;

    /// Processes the current event
    virtual void event() override;

  private:
    /**
       Factory for the cluster filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::ClusterFilterFactory m_clusterFilterFactory;

    /**
       Factory for the facet filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::FacetFilterFactory m_facetFilterFactory;

    /**
       Factory for the facet relation filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::FacetRelationFilterFactory m_facetRelationFilterFilterFactory;


    /**
       Factory for the segment relation filter, knowing all the available filters and
       their respective parameters
    */
    Belle2::TrackFindingCDC::SegmentRelationFilterFactory m_segmentRelationFilterFilterFactory;

  }; // end class
} // end namespace Belle2
