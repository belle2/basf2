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

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TH1.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Efficiency */

  class DQMHistInjectionModule : public DQMHistAnalysisModule {

#ifdef _BELLE2_EPICS
    typedef struct MYNODE_struct {
      chid    mychid; /**< channel ID */
      TH1*  histo = {}; /**< initial histogram */
      std::vector <double> data; /**< vector of data */
    } MYNODE;

#endif

    // Public functions
  public:

    //! Constructor
    DQMHistInjectionModule();
    //! Destructor
    ~DQMHistInjectionModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void terminate(void) override final;

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;
    //! whether to use EPICs
    bool m_useEpics;

    //! Clean up PVs
    // cppcheck-suppress unusedPrivateFunction
    void cleanPVs(void);

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

#ifdef _BELLE2_EPICS
    //! EPICS PVs
    std::vector <MYNODE>  m_nodes;
#endif
  };
} // end namespace Belle2

