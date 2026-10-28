/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

namespace Belle2 {

  /**
   * DQM framework core module to load DQM reference histograms
   * It must be run after the input modules and before the first real
   * analysis module which may require a reference plot.
   */

  class DQMHistReferenceModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistReferenceModule();

    /**
     * Called when entering a new run.
     * This loads the run type dependend reference histograms into the framework.
     */
    void beginRun() override final;

  private:

    /** Reference Histogram Root file name */
    std::string m_referenceFileName;

    /** Reads reference histograms from input root file */
    void loadReferenceHistos();

  };
} // end namespace Belle2

