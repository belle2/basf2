/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDFits.h
// Description : An example module for DQM histogram analysis
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TH2F.h>
#include <TF1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisPXDFitsModule : public DQMHistAnalysisModule {

    enum { NUM_MODULES = 40}; // we want that from geometry
    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisPXDFitsModule();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

  private:

    /** Histogram doirectory */
    std::string m_histogramDirectoryName;
    /** maps from VXDid to index */
    std::map <int, int> m_id_to_inx;
    /** maps from index to VXDid*/
    std::map <int, int> m_inx_to_id;

    /** Histogram */
    TH1F* m_hSignalAll = nullptr;
    /** Histogram */
    TH1F* m_hCommonAll = nullptr;
    /** Histogram */
    TH1F* m_hCountsAll = nullptr;
    /** Histogram */
    TH1F* m_hOccupancyAll = nullptr;
    /** Canvas */
    TCanvas* m_cSignalAll = nullptr;
    /** Canvas */
    TCanvas* m_cCommonAll = nullptr;
    /** Canvas */
    TCanvas* m_cCountsAll = nullptr;
    /** Canvas */
    TCanvas* m_cOccupancyAll = nullptr;

    /** Histograms */
    TH2F* m_hSignal[NUM_MODULES];
    /** Histograms */
    TH2F* m_hCommon[NUM_MODULES];
    /** Histograms */
    TH2F* m_hCounts[NUM_MODULES];
    /** Canvases */
    TCanvas* m_cSignal[NUM_MODULES];
    /** Canvases */
    TCanvas* m_cCommon[NUM_MODULES];
    /** Canvases */
    TCanvas* m_cCounts[NUM_MODULES];
    /** Fit functions */
    TF1* m_fLandau = nullptr; // only one fit function
    /** Fit functions */
    TF1* m_fGaus = nullptr; // only one fit function

  };
} // end namespace Belle2

