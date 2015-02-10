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
    virtual ~NotAssignedHitsSearcherModule() {};

    /**
     * Initialize the module. Create the StoreArray.
     */
    virtual void initialize();

    /**
     * Empty begin run.
     */
    virtual void beginRun() {};

    /**
     * In the event the hits are sorted.
     */
    virtual void event();

    /**
     * Empty end run.
     */
    virtual void endRun() {};

    /**
     * Empty terminate.
     */
    virtual void terminate() {};

  protected:

  private:

    std::string m_param_tracksFromTrackFinder;        /**< TrackCandidates store array name from the track finder. */
    std::string m_param_cdcHits;                      /**< Name of the store array containing the hits. */
    std::string m_param_notAssignedCDCHits;           /**< Name of the store array containing the hits not used by the track finding algorithm. This store array is created by this module. */
  };
}
