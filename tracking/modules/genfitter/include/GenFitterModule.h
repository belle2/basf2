/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GENFITTERMODULE_H_
#define GENFITTERMODULE_H_

#include <framework/core/Module.h>

namespace Belle2 {

  /** Module for fitting tracks using the GENFIT package.
   *
   */
  class GenFitterModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    GenFitterModule();

    /** Destructor of the module. */
    ~GenFitterModule();

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
    std::string m_trackToCDCRecoHitCollectionName;
  };
}

#endif /* GENFITTERMODULE_H_ */
