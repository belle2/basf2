/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPMCTRACKMAKERMODULE_H
#define TOPMCTRACKMAKERMODULE_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * Constructs Tracks and ExtHits from MCParticles and TOPBarHits
   * Utility needed for testing and debugging of TOP reconstruction
   */
  class TOPMCTrackMakerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPMCTrackMakerModule();

    /**
     * Destructor
     */
    virtual ~TOPMCTrackMakerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:


  };

} // Belle2 namespace

#endif
