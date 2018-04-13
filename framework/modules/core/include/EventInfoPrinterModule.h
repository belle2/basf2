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

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  /** The event meta data info module.
   *
   * This module prints the current event meta data information using B2INFO.
   *
   * (e.g. Could be used later to also send the info over TCP/IP to a GUI)
   */
  class EventInfoPrinterModule : public Module {
  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    EventInfoPrinterModule();

    /** Destructor of the module. */
    virtual ~EventInfoPrinterModule();

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

  private:
    std::string formatDateTime(long long int); /**< Format date & time for output */

    bool m_printTime;  /**< Print time in addition to exp/run/evt numbers. */
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< EventMetaData. */
  };

}
