#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 **************************************************************************/

#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * Display a progress bar and an estimate of remaining time when number of events is known (e.g. reading from file, or -n switch used).
   *
   */
  class ProgressBarModule : public Module {

  public:

    /** Constructor */
    ProgressBarModule();

    /** Init the module */
    virtual void initialize();
    /** Show progress */
    virtual void event();
    /** Don't break the terminal */
    virtual void terminate();

  protected:
    /** Number of processed events. */
    long m_evtNr;
    /** Total number of events. */
    long m_nTotal;
    /** Start time (ns). */
    long m_startTime;
    /** Timestamp when we last printed something (ns). */
    long m_lastPrint;
  };
} // end namespace Belle2
