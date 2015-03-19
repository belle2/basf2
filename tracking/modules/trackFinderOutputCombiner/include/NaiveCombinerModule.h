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

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  class NaiveCombinerModule : public Module {

  public:

    NaiveCombinerModule();

    ~NaiveCombinerModule() {};

    void initialize();

    void beginRun() {};

    void event();

    void endRun() {};

    void terminate() {};

  private:
    std::string
    m_param_tracksFromLegendreFinder;                        /**< Bad TrackCandidates collection name from the legendre finder. */
    std::string
    m_param_notAssignedTracksFromLocalFinder;                        /**< TrackCandidates collection name from the local finder. */
    std::string
    m_param_resultTrackCands;                   /**< TrackCandidates collection name from the combined results of the two recognition algorithm. The CDCHits are assumed to come from m_param_cdcHitsColName1. */
    std::string m_param_mcTrackCands;                   /**< When UseMCInformation is enabled, you need to set the MCTrackCands here. */

    std::string
    m_param_cdcHitsColName;                        /**< The CDCHits collection name for the first pattern recognition algorithm. */
    bool m_param_useMCInformation;                             /**< Uses the MC information from the MCTrackFinder and the MCMatcher to merge tracks */
  };
}
