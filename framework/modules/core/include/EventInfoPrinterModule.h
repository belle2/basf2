#pragma once
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    virtual void initialize() override;

    /**
     * Prints a header for each new run.
     *
     * A header is printed which provides the information that a new
     * run was started and which run number we are currently running on.
     */
    virtual void beginRun() override;

    /** Prints the full information about the event, run and experiment number. */
    virtual void event() override;

    /**
     * Prints a footer for each run which ended.
     *
     * A footer is printed which provides the information that a run
     * was ended and which run number we were running on.
     */
    virtual void endRun() override;

  private:
    std::string formatDateTime(unsigned long long int); /**< Format date & time for output */

    bool m_printTime;  /**< Print time in addition to exp/run/evt numbers. */
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< EventMetaData. */
  };

}
