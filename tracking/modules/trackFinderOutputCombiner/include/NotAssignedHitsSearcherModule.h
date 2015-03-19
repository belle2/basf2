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

namespace Belle2 {

  class NotAssignedHitsSearcherModule : public Module {

  public:

    /**
     * Constructor to set the module parameters.
     */
    NotAssignedHitsSearcherModule();

    /**
     * Empty destructor.
     */
    ~NotAssignedHitsSearcherModule() {};

    /**
     * Initialize the module. Create the StoreArray.
     */
    void initialize();

    /**
     * Empty begin run.
     */
    void beginRun() {};

    /**
     * In the event the hits are sorted.
     */
    void event();

    /**
     * Empty end run.
     */
    void endRun() {};

    /**
     * Empty terminate.
     */
    void terminate() {};

  protected:

  private:

    std::string m_param_tracksFromTrackFinder;        /**< TrackCandidates store array name from the track finder. */
    std::string m_param_splittedTracks;               /**< TrackCandidates store array name for the splitted tracks. */
    std::string m_param_cdcHits;                      /**< Name of the store array containing the hits. */
    std::string
    m_param_notAssignedCDCHits;           /**< Name of the store array containing the hits not used by the track finding algorithm. This store array is created by this module. */

    double m_param_minimumDistanceToSplit;            /**< Minimal distance to split up a track. This distance is measured in percentage of the whole track length. */
    bool m_param_initialAssignmentValue;              /**< If true, not add the now unassigned hits to the NotAssignedCDCHits StoreArray but only the hits from splitting. If false, add the hits from splitting and the now unassigned hits. */
    unsigned int m_param_minimalHits;                 /**< . */
  };
}
