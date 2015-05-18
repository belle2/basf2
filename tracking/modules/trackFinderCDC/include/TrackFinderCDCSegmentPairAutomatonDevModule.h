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

#include <tracking/modules/trackFinderCDC/TrackFinderCDCSegmentPairAutomatonModule.h>

namespace Belle2 {



  /// Module for the cellular automaton tracking for the CDC on regular events
  class TrackFinderCDCSegmentPairAutomatonDevModule: public Belle2::TrackFindingCDC::TrackFinderCDCSegmentPairAutomatonImplModule<> {

  public:
    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCSegmentPairAutomatonDevModule();

    /// Initialize the Module before event processing
    virtual void initialize() override;

    /// Event method exectured for each event.
    virtual void event() override;

  private:
    /** Parameter: Segment pair filter to be used during the construction of segment pairs.
    Valid values are:

    + "all" (all segment pairs are valid)
    + "mc" (monte carlo truth)
    + "mc_symmetric" (monte carlo truth and their mirror image)
    + "simple" (mc free with simple criteria)
    */
    std::string m_param_segmentPairFilter;

    /** Parameter: Segment Pair filter parameters forwarded to the segment pair filter
     *  Meaning of the Key - Value pairs depend on the segment pair filter
     */
    std::map<std::string, std::string> m_param_segmentPairFilterParameters;

    /** Parameter: Segment pair relation filter to be used during the construction of the graph.
    Valid values are:

    + "none" (no neighbor is correct, stops segment generation)
    + "all" (all possible neighbors are valid)
    + "mc" (monte carlo truth)
    + "mc_symmetric" (monte carlo truth and the reversed version are excepted)
    + "simple" (mc free with simple criteria)
    */
    std::string m_param_segmentPairRelationFilter;

    /** Parameter: Segment Pair relation filter parameters forwarded to the segment pair relation filter
     *  Meaning of the Key - Value pairs depend on the segment pair relation filter
     */
    std::map<std::string, std::string> m_param_segmentPairRelationFilterParameters;

  }; // end class
} // end namespace Belle2


