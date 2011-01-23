/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECOHITMAKERMODULE_H_
#define RECOHITMAKERMODULE_H_

#include <framework/core/Module.h>
#include <vector>


namespace Belle2 {

  /** Module for making CDCRecoHits of CDCHits.
   *
   */
  class CDCRecoHitMakerModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    CDCRecoHitMakerModule();

    /** Destructor of the module. */
    ~CDCRecoHitMakerModule();

    void initialize();

    /**
     * Prints a header for each new run.
     *
     * A header is printed which provides the information that a new
     * run was started and which run number we are currently running on.
     */
    void beginRun();

    /** Prints the full information about the event, run and experiment number. */
    void event();

    /**
     * Prints a footer for each run which ended.
     *
     * A footer is printed which provides the information that a run
     * was ended and which run number we were running on.
     */
    void endRun();

    void terminate();


  private:


  };
}

#endif /* EVTMETAINFO_H_ */
