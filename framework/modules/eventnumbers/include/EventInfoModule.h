#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   * \addtogroup framework_modules
   * \ingroup modules
   * @{ EventInfoModule @} @}
   */

  /** The event meta data info module.
   *
   * This module prints the current event meta data information using B2INFO.
   *
   * (e.g. Could be used later to also send the info over TCP/IP to a GUI)
   */
  class EventInfoModule : public Module {
  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    EventInfoModule();

    /** Destructor of the module. */
    virtual ~EventInfoModule();

    /** initialization. */
    virtual void initialize();

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
  };

  /** for compatibility.
   *
   * @deprecated
   */
  class EvtMetaInfoModule : public EventInfoModule {
  public:
    EvtMetaInfoModule() : EventInfoModule() {
      setDescription("Deprecated, use the EventInfo module instead.");
    }
    virtual void initialize() {
      B2WARNING("Deprecated, use the EventInfo module instead.");
      EventInfoModule::initialize();
    }
  };
}
