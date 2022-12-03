/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisOut.h
// Description : DQM Analysis, dump histograms to file (as reference histograms)
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputFileModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisOutputFileModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisOutputFileModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

    // Data members
  private:

    std::string m_filename;/**< file name of root file */
    std::string m_histogramDirectoryName;/**< Directory name within root file where to place things */
    bool m_saveHistos;/**< Write all Histos to file */
    bool m_saveCanvases;/**< Write all Canvases to file */
  };
} // end namespace Belle2

