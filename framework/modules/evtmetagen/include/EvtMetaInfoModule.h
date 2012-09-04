/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTMETAINFOMODULE_H_
#define EVTMETAINFOMODULE_H_

#include <framework/core/Module.h>
#include <vector>


namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   * \addtogroup framework_modules
   * \ingroup modules
   * @{ EvtMetaInfoModule @} @}
   */

  /** The event meta data info module.
   *
   * This module prints the current event meta data information to
   * the command line.
   * (e.g. Could be used later to also send the info over TCP/IP to a GUI)
   */
  class EvtMetaInfoModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    EvtMetaInfoModule();

    /** Destructor of the module. */
    virtual ~EvtMetaInfoModule();

    /**
     * Prints a header for each new run.
     *
     * A header is printed which provides the information that a new
     * run was started and which run number we are currently running on.
     */
    virtual void beginRun();

    /** Prints the full information about the event, run and experiment number. */
    virtual void event();

    /**
     * Prints a footer for each run which ended.
     *
     * A footer is printed which provides the information that a run
     * was ended and which run number we were running on.
     */
    virtual void endRun();


  protected:


  private:

  };
}

#endif /* EVTMETAINFOMODULE_H_ */
