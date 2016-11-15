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

  /**
   * Naive combiner module for testing purposes.
   * It can either put all segments and tracks into the same store array or use the MC information for making the correct matches.
   *
   * When the flag to use MC information is false, you can choose whether you want to use the taken flag of the hits:
   * If yes, only those hits of the segments are added to the tracks created from the segments that are not already found by the Legendre track candidate.
   * If no, the whole segment is used (that may lead to strange hit efficiencies).
   */
  class NaiveCombinerModule : public TrackFinderCDCFromSegmentsModule {

  public:

    /** Constructor. */
    NaiveCombinerModule();

  private:
    /**
     * Fill all segments and tracks to the output list.
     */
    void generate(std::vector<TrackFindingCDC::CDCSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;

    /** Uses the MC information from the MCTrackFinder and the MCMatcher to merge tracks. */
    bool m_param_useMCInformation;

    /** If yes (and not using MC information), copy only those hits of the segments to new tracks which are not already found by the Legendre track finder. */
    bool m_param_useTakenFlagOfHits;
  };
}
