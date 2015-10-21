/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

namespace Belle2 {

  class NaiveCombinerModule : public TrackFinderCDCFromSegmentsModule {

  public:

    /** Constructor */
    NaiveCombinerModule();

  private:
    /**
     * Fill all segments and tracks to the output list
     */
    void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

    /** Uses the MC information from the MCTrackFinder and the MCMatcher to merge tracks */
    bool m_param_useMCInformation;
    bool m_param_useTakenFlagOfHits;
  };
}
