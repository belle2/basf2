/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#ifndef EventCounterModule_H_
#define EventCounterModule_H_

#include <framework/core/Module.h>
// #include <fstream>
#include <string>


namespace Belle2 {

  /** The event counter module
   *
   * this module simply counts the number of events (and prints every x-th event onto the screen, where x is user-defined). Useful when executing modules which do not provide this feature themselves
   *
   */
  class EventCounterModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    EventCounterModule();

    /** Destructor of the module. */
    virtual ~EventCounterModule();

    /** Initializes the Module.
     */
    virtual void initialize();

    /**
     * Prints a header for each new run.
     */
    virtual void beginRun();

    /** Prints the full information about the event, run and experiment number. */
    virtual void event();

    /**
     * Prints a footer for each run which ended.
     */
    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  protected:


    int m_eventCounter; /**< knows current event number */
    int m_stepSize; /**< Informes the user that  event: (eventCounter-modulo(stepSize)) is currently executed */

  private:

  };
}

#endif /* EventCounterModule_H_ */
