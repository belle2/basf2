/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/core/EventProcessor.h>
#include <framework/pcore/ProcHandler.h>

namespace Belle2 {
  class Path;
  class RxModule;
  class TxModule;
  class RingBuffer;
  /** Framework-internal module for graceful handling of crashes in other modules. */
  class CrashHandlerModule : public Module, public EventProcessor {
  public:
    CrashHandlerModule();
    ~CrashHandlerModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    std::shared_ptr<Path> m_path; /**< Path to execute. */
    int m_processID =
      -1; /**< when using multi-processing contains the ID of the process where event() is called (in that process only). -1 otherwise. */

    /** receiving module. */
    RxModule* m_rx = nullptr;

    /** transmitting module. */
    TxModule* m_tx = nullptr;

    /** number of events. */
    int m_nEvents = 0;
    /** number of crashes encountered. */
    int m_nCrashedEvents = 0;

    ProcHandler m_procHandler; /**< handles forking of processes. */
  };
}
