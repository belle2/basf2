/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDGeneral.h
// Description : module for DQM histogram analysis of SVD sensors occupancies
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TFile.h>
#include <TText.h>
#include <TPaveText.h>
#include <TCanvas.h>
#include <TH2F.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDUnpackerModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisSVDUnpackerModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisSVDUnpackerModule();

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

    // parameters
    bool m_printCanvas; /**< if true print the pdf of the canvases */
    bool m_3Samples; /**< if true enable 3 samples histograms analysis */
    Double_t m_unpackError = 0; /**< Maximum bin_content/ # events allowed before throwing ERROR*/

    //! Parameters accesible from basf2 scripts
    //  protected:

    TCanvas* m_cUnpacker = nullptr; /**<unpacker plot canvas */
    TPaveText* m_legError = nullptr; /**<  plot legend, error*/
    TString m_runtype = nullptr; /**< string with runtype: physics or cosmic */

    std::string m_pvPrefix; /**< string prefix for EPICS PVs */

  };
} // end namespace Belle2

