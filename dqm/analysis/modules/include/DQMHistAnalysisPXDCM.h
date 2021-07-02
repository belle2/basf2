/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDCM.h
// Description : DAQM Analysis for PXD Common Modes
//
// Author : Bjoern Spruck, University Mainz
// Date : 2018
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <vector>
#include <map>
#include <TH2.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Common Modes */

  class DQMHistAnalysisPXDCMModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDCMModule();
    //! Destructor
    ~DQMHistAnalysisPXDCMModule();
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
    /** Update entry intervall */
    int m_minEntries = 10000;

    /** warn level for mean adhoc plot */
    double m_warnMeanAdhoc{};
    /** error level for mean adhoc plot */
    double m_errorMeanAdhoc{};
    /** warn level for outside fraction */
    double m_warnOutsideAdhoc{};
    /** error level for outside fraction */
    double m_errorOutsideAdhoc{};
    /** threshold level/line for outside fraction */
    int m_upperLineAdhoc{};

    /** warn level for mean adhoc plot */
    double m_warnMeanFull{};
    /** error level for mean adhoc plot */
    double m_errorMeanFull{};
    /** warn level for outside fraction */
    double m_warnOutsideFull{};
    /** error level for outside fraction */
    double m_errorOutsideFull{};
    /** threshold level/line for outside fraction */
    int m_upperLineFull{};

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! histogram covering all modules
    TH2D* m_hCommonMode = nullptr;
    //! histogram covering all modules
    TH2D* m_hCommonModeDelta = nullptr;
    //! histogram covering all modules
    TH2D* m_hCommonModeOld = nullptr;
    //! Final Canvas
    TCanvas* m_cCommonMode = nullptr;
    //! Final Canvas
    TCanvas* m_cCommonModeDelta = nullptr;
    //! Line in the Canvas to guide the eye
    TLine* m_line1 = nullptr;
    //! Line in the Canvas to guide the eye
    TLine* m_lineA = nullptr;
    //! Line in the Canvas to guide the eye
    TLine* m_lineF = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

    /** flag if to export to EPICS */
    bool m_useEpics;

#ifdef _BELLE2_EPICS
    //! epics PVs
    std::vector <chid> mychid;
    //! epics PVs
    std::map <VxdID, chid> mychid_mean;
#endif
  };
} // end namespace Belle2

