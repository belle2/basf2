/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Framework headers. */
#include <framework/core/Module.h>

/* C++ headers. */
#include <stdint.h>

/* boost headers. */
#include <boost/format.hpp>

namespace Belle2 {

  /**
   * Periodically writes the number of processed events/runs to the
   * logging system to give a progress indication.
   *
   * The output is logarithmic, meaning it will output the first 10 events,
   * then every tenth event up to 100, then every hundreth event up to 1000, etc.
   *
   * The maximum interval between outputs can be configured.
   */
  class ProgressModule : public Module {

  public:

    /** Constructor */
    ProgressModule();

    /** Init the module */
    void initialize() override;

    /** Show beginRun message */
    void beginRun() override;

    /** Show progress */
    void event() override;

  protected:

    /** Maximum order of magnitude for interval between showing progress */
    uint32_t m_maxOrder{3};

    /** Number of processed events. This is not the real eventNumber, just counting */
    uint32_t m_evtNr{0};

    /** Number of processed runs. This is not the real runNumber, just counting */
    uint32_t m_runNr{0};

    /** Total number of events in the current process */
    uint32_t m_totalEvtNr{0};

    /** Compiled output format */
    boost::format m_output{"Processed: %3u runs, %6u/%6u events."};

  };
}
