#pragma once
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    virtual void initialize() override;
    /** Show progress */
    virtual void event() override;
    /** Don't break the terminal */
    virtual void terminate() override;

  protected:
    /** Number of processed events. */
    long m_evtNr;
    /** Total number of events. */
    long m_nTotal;
    /** Start time (ns). */
    double m_startTime;
    /** Timestamp when we last printed something (ns). */
    double m_lastPrint;
    /** progress in percent since we last printed */
    int m_progress{0};
    /** If true we don't show the interactive progress bar but just print a new one every time the progress amount changes */
    bool m_isTTY;
  };
} // end namespace Belle2
