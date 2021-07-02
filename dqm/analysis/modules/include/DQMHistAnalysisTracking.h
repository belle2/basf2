/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisTracking.h
// Description : DQM Tracking Analysis
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for Tracking */

  class DQMHistAnalysisTrackingModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisTrackingModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;

    //! Final Canvas
    TCanvas* m_cTrackingError = nullptr;

  };
} // end namespace Belle2

