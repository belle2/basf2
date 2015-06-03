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

#include <tracking/modules/trackFinderOutputCombiner/StereoSegmentTrackMatcherModule.h>

namespace Belle2 {

  /// Module for the combination of tracks and stereo segments. Development edition.
  class StereoSegmentTrackMatcherDevModule:
    public Belle2::TrackFindingCDC::StereoSegmentTrackMatcherModuleImpl<> {

  public:
    /// Constructor of the module. Setting up parameters and description.
    StereoSegmentTrackMatcherDevModule();

    /// Initialize the Module before event processing
    virtual void initialize() override;

    /// Event method executed for each event. - Fill the MCFilters if needed.
    virtual void event() override;

  private:
    /** Parameter: Chooser to be used for matching segments and tracks
    Valid values are:
    + "simple"
    + "recording"
    + "tmva"
    + "mc"
    + "all"
    */
    std::string m_param_segmentTrackChooser;

    /** Parameter: Segment Track Chooser parameters forwarded to the chooser
     *  Meaning of the Key - Value pairs depend on the filter
     */
    std::map<std::string, std::string> m_param_segmentTrackChooserParameters;

  }; // end class
} // end namespace Belle2
