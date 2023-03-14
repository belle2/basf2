/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistComparitor.h
// Description :
//-

#pragma once

// EPICS
#ifdef _BELLE2_EPICS
#include "cadef.h"
// #include "dbDefs.h"
// #include "epicsString.h"
// #include "cantProceed.h"
#endif

#include <dqm/core/DQMHistAnalysis.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>

namespace Belle2 {
  /** Class definition for the reference histogram display. */

  class DQMHistComparitorModule final : public DQMHistAnalysisModule {

    /**
     * The struct for reference histogram comparison.
     */
    typedef struct {
#ifdef _BELLE2_EPICS
      chid    mychid;
#endif
      /** Whether to use EPICS. */
      bool epicsflag;
      /** The name of the histogram to be compared. */
      TString histo1;
      /** The name of the reference histogram. */
      TString histo2;
      /** The canvas to display both original and reference histograms. */
      TCanvas* canvas;
      /** The warning level for the histogram difference. */
      float warning;
      /** The error level for the histogram difference. */
      float error;
      /** The mininum entries for histogram comparison. */
      int min_entries;
    } CMPNODE;

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistComparitorModule();

    /**
     * Destructor.
     */
    ~DQMHistComparitorModule();

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

    // Data members
  private:
    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;
    /** Struct for extracted parameters + EPICS PV */
    std::vector<CMPNODE*> m_pnode;
    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;
    /** Whether to use the color code for warnings and errors. */
    bool m_color = true;

    /**
     * Get histogram by its name.
     * @param histoname The name of the histogram.
     * @return The pointer to the histogram, or nullptr if not found.
     */
    TH1* GetHisto(TString histoname);

  };
} // end namespace Belle2

