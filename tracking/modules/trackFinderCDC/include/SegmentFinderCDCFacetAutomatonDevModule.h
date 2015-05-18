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

namespace Belle2 {

  /// Module for the cellular automaton tracking for the CDC on regular events
  class SegmentFinderCDCFacetAutomatonDevModule:
    public Belle2::TrackFindingCDC::SegmentFinderCDCFacetAutomatonImplModule<> {

  public:
    /// Constructor of the module. Setting up parameters and description.
    SegmentFinderCDCFacetAutomatonDevModule();

    /// Initialize the Module before event processing
    virtual void initialize() override;

    /// Event method exectured for each event.
    virtual void event() override;

  private:
    /**
       Factory for the cluster filter, knowing all the available filters and there respective parameters
    */
    Belle2::TrackFindingCDC::ClusterFilterFactory m_clusterFilterFactory;


    /** Parameter: Facet filter to be used during the construction of facets.
    Valid values are:
    + "none" (no facet is valid, stop at cluster generation.)
    + "all" (all facets are valid)
    + "mc" (monte carlo truth)
    + "fitless" (only checking the feasability of right left passage information)
    + "simple" (mc free with simple criteria)
    + "realistic" (mc free with more realistic criteria)
    + "recording" (records the encountered instances of facets including truth information)",
    */
    std::string m_param_facetFilter;

    /** Parameter: Facet filter parameters forwarded to the facet filter
     *  Meaning of the Key - Value pairs depend on the facet filter
     */
    std::map<std::string, std::string> m_param_facetFilterParameters;

    /** Parameter: Facet neighbor chooser to be used during the construction of the graph.
    Valid values are:

    + "none" (no neighbor is correct, stops segment generation)
    + "all" (all possible neighbors are valid)
    + "mc" (monte carlo truth)
    + "simple" (mc free with simple criteria)
    + "recording" (records the encountered instances of facets relations including truth information)",
    */
    std::string m_param_facetNeighborChooser;

    /** Parameter: Facet neighbor chooser parameters forwarded to the facet neighbor chooser
     *  Meaning of the Key - Value pairs depend on the facet neighbor chooser
     */
    std::map<std::string, std::string> m_param_facetNeighborChooserParameters;

  }; // end class
} // end namespace Belle2
