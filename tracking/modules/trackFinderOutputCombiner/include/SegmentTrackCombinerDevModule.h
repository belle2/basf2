/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombinerModule.h>

namespace Belle2 {

  /// Module for the combination of tracks and segments. Development edition.
  class SegmentTrackCombinerDevModule:
    public Belle2::TrackFindingCDC::SegmentTrackCombinerImplModule<> {

  public:
    /// Constructor of the module. Setting up parameters and description.
    SegmentTrackCombinerDevModule();

    /// Initialize the Module before event processing
    virtual void initialize() override;

    /// Event method exectured for each event.
    virtual void event() override;

  private:
    /** Parameter: Chooser to be used for matching segments and tracks
    Valid values are:
    + "simple"
    + "recording"
    # "tmva"
    */
    std::string m_param_segmentTrackChooser;

    /** Parameter: Segment Track Chooser parameters forwarded to the chooser
     *  Meaning of the Key - Value pairs depend on the filter
     */
    std::map<std::string, std::string> m_param_segmentTrackChooserParameters;

    /** Parameter: Filter to be used for construction segment trains
    Valid values are:
    + "simple"
    */
    std::string m_param_segmentTrainFilter;

    /** Parameter: Segment Train Filter  parameters forwarded to the chooser
     *  Meaning of the Key - Value pairs depend on the filter
     */
    std::map<std::string, std::string> m_param_segmentTrainFilterParameters;

    /** Parameter: Filter to be used to do an uniqe segment train <-> track matching
    Valid values are:
    + "simple"
    */
    std::string m_param_segmentTrackFilter;

  }; // end class
} // end namespace Belle2
