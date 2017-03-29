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

#include <tracking/trackFindingCDC/findlets/combined/SegmentTrackCombiner.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    /**
     * Module for the combination of tracks and segments.
     *
     * This module uses configurable filters to create matches between segments (from the local finder)
     * and tracks (from the global finder). It also has capabilities to filter the remaining tracks for fakes
     * and the remaining segments for new tracks/background.
     *
     * Use this module at the end of the path, after you have run the local and the global track finder.
     *
     * It uses several (filtering) steps, to do its job, namely:
     *  1. Creation of a fast segment and track lookup.
     *  2. First matching of segment and tracks that share one or more hits.
     *  3. Filtering of fake tracks in the made combinations.
     *  4. Cleanup of the lookup cache.
     */
    class  TFCDC_SegmentTrackCombinerModule : public FindletModule<SegmentTrackCombiner> {

      /// The base class
      using Super = FindletModule<SegmentTrackCombiner>;

    public:
      /// Constructor
      TFCDC_SegmentTrackCombinerModule();
    };
  }
}
