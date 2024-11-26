/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistInjection.h
// Description : DQM module, which gives occupancies in time after injection
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TH1.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Efficiency */

  class DQMHistInjectionModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistInjectionModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * Called when entering a new run.
     */
    void beginRun(void) override final;

    /**
     * This method is called for each event.
     */
    void event(void) override final;

  private:

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    //! PXD Hits
    TH1F* m_hInjectionLERPXD = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERPXD = nullptr;
    //! PXD Occ norm
    TH1F* m_hInjectionLERPXDOcc = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERPXDOcc = nullptr;
    //! SVD Hits
    TH1F* m_hInjectionLERSVD = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERSVD = nullptr;
    //! SVD Occ norm
    TH1F* m_hInjectionLERSVDOcc = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERSVDOcc = nullptr;
    //! ECL Hits
    TH1F* m_hInjectionLERECL = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERECL = nullptr;
    //! ECL Bursts
    TH1F* m_hBurstLERECL = nullptr;
    //! Final Canvas
    TCanvas* m_cBurstLERECL = nullptr;
    //! TOP Occ
    TH1F* m_hInjectionLERTOP = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERTOP = nullptr;
    //! ARICH Occ
    TH1F* m_hInjectionLERARICH = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERARICH = nullptr;
    //! KLM occupancy after LER injection
    TH1F* m_hInjectionLERKLM = nullptr;
    //! Canvas for KLM occupancy after LER injection
    TCanvas* m_cInjectionLERKLM = nullptr;
    //! PXD Hits
    TH1F* m_hInjectionHERPXD = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERPXD = nullptr;
    //! PXD Occ norm
    TH1F* m_hInjectionHERPXDOcc = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERPXDOcc = nullptr;
    //! SVD Hits
    TH1F* m_hInjectionHERSVD = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERSVD = nullptr;
    //! SVD Occ norm
    TH1F* m_hInjectionHERSVDOcc = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERSVDOcc = nullptr;
    //! ECL Hits
    TH1F* m_hInjectionHERECL = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERECL = nullptr;
    //! ECL Bursts
    TH1F* m_hBurstHERECL = nullptr;
    //! Final Canvas
    TCanvas* m_cBurstHERECL = nullptr;
    //! TOP Occ
    TH1F* m_hInjectionHERTOP = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERTOP = nullptr;
    //! ARICH Occ
    TH1F* m_hInjectionHERARICH = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERARICH = nullptr;
    //! KLM occupancy after HER injection
    TH1F* m_hInjectionHERKLM = nullptr;
    //! Canvas for KLM occupancy after HER injection
    TCanvas* m_cInjectionHERKLM = nullptr;

  };
} // end namespace Belle2

