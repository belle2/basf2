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
//
// Author : B. Spruck
// Date : 11 - Dec - 2017 ; first commit
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputFileModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputFileModule();
    virtual ~DQMHistAnalysisOutputFileModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:

    std::string m_filename;/**< file name of root file */
    std::string m_histogramDirectoryName;/**< Directory name within root file where to place things */
    bool m_saveHistos;/**< Write all Histos to file */
    bool m_saveCanvases;/**< Write all Canvases to file */
  };
} // end namespace Belle2

