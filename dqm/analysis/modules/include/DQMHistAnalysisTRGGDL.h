/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRGGDLDQMANALYSISMODULE_H
#define TRGGDLDQMANALYSISMODULE_H

#pragma once

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

//ARICH
#include <arich/utility/ARICHChannelHist.h>

#include <vector>

#include <TCanvas.h>
#include <TLine.h>
#include <TH2Poly.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisTRGGDLModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisTRGGDLModule();

    /**
     * Destructor
     */
    virtual ~DQMHistAnalysisTRGGDLModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

    /**
    * Find canvas by name
    * @param cname Name of the canvas
    * @return The pointer to the canvas, or nullptr if not found.
    */
    TCanvas* find_canvas(TString cname);

  protected:
    bool m_debug;/**<debug*/
    bool m_enableAlert;/**<Enable alert by base color of canvases*/


    static const int nskim_gdldqm = 11;      /**number of HLT skims*/
    std::string skim_smap[nskim_gdldqm] = {  /**name of HLT skims*/
      "all",
      "hadron",
      "tautau",
      "mumu1trk",
      "mumu2trk",
      "gammagamma",
      "bhabha",
      "hadronb",
      "hadronb1",
      "hadronb2",
      "mumutight"
    };

    TH1D* m_h_psn_extra[nskim_gdldqm] = {}; /**<DQM Histogram for PSN bits in each HLT skim*/
    TH1D* m_h_psn_pure_extra = nullptr;   /**<DQM Histogram for PSN bits with offline selection*/
    TCanvas* m_c_eff[nskim_gdldqm] = {}; /**<Canvas for TRGGDL efficiency in each HLT skim*/
    TH1D* m_h_eff[nskim_gdldqm] = {};   /**<Histogram for TRGGDL efficiency in each HLT skim*/
    static const int n_eff = 35;        /**number of bins for the efficiency histogram*/
    /**label of bins for the efficiency histogram*/
    const char* c_eff[n_eff] = {
      "fff with c4|hie",
      "ffo with c4|hie",
      "ffb with c4|hie",
      "ffy with c4|hie",
      "fyo with c4|hie",
      "fyb with c4|hie",
      "hie with fff|ffo|ffb",
      "c4 with fff|ffo|ffb",
      "lml0 with fff|ffo|ffb",
      "lml1 with fff|ffo|ffb",
      "lml2 with fff|ffo|ffb",
      "lml3 with fff|ffo|ffb",
      "lml4 with fff|ffo|ffb",
      "lml5 with fff|ffo|ffb",
      "lml6 with fff|ffo|ffb",
      "lml7 with fff|ffo|ffb",
      "lml8 with fff|ffo|ffb",
      "lml9 with fff|ffo|ffb",
      "lml10 with fff|ffo|ffb",
      "lml12 with fff|ffo|ffb",
      "lml13 with fff|ffo|ffb",
      "bha3d with all",
      "mu_b2b with fff|ffo|ffb",
      "mu_b2b with lml|eclmumu",
      "mu_eb2b with lml|eclmumu",
      "cdcklm1 with fff|ffo|ffb",
      "cdcklm2 with fff|ffo|ffb",
      "fff with lml|eclmumu",
      "ffo with lml|eclmumu",
      "ffb with lml|eclmumu",
      "ff with lml|eclmumu",
      "f with lml|eclmumu",
      "ffy with lml|eclmumu",
      "fyo with lml|eclmumu",
      "fyb with lml|eclmumu"
    };
    TCanvas* m_c_eff_shifter = nullptr;                 /**<Canvas for TRGGDL efficiency, simplified one for CR shifter*/
    TH1D* m_h_eff_shifter = nullptr;                    /**<Histogram for TRGGDL efficiency, simplified one for CR shifter*/
    static const int n_eff_shifter = 7;                 /**number of bins for the simplified efficiency histogram*/
    const char* c_eff_shifter[n_eff_shifter] = {        /**label of bins for the simplified efficiency histogram*/
      "CDC fff",
      "CDC ffo",
      "CDC ffy",
      "CDC fyo",
      "ECL hie",
      "ECL c4",
      "KLM b2b"
    };
    TCanvas* m_c_pure_eff = nullptr;              /**<Canvas for TRGGDL efficiency with offline selection*/
    TH1D* m_h_pure_eff = nullptr;                 /**<Histogram for TRGGDL efficiency with offline selection*/
    static const int n_pure_eff = 8;              /**number of bins for the efficiency histogram with offline selection*/
    /**label of bins for the efficiency histogram with offline selection*/
    const char* c_pure_eff[n_pure_eff] = {
      "fff with c4|hie",
      "ffo with c4|hie",
      "ffb with c4|hie",
      "ffy with c4|hie",
      "fyo with c4|hie",
      "fyb with c4|hie",
      "hie with fff|ffo|ffb",
      "c4 with fff|ffo|ffb"
    };

    TLine* m_line_limit_low_shifter[n_eff_shifter] = {}; /**lower limit line for the simplified efficiency histogram*/
    TLine* m_line_limit_high_shifter[n_eff_shifter] = {}; /**upper limit line for the simplified efficiency histogram*/
    double m_limit_low_shifter[n_eff_shifter] = {       /**lower limit value in each bin*/
      0.7, 0.7, 0.7, 0.7, 0.8, 0.8, 0.1
    };
    double m_limit_high_shifter[n_eff_shifter] = {      /**upper limit value in each bin*/
      0.9, 0.9, 0.9, 0.9, 1.0, 1.0, 0.2
    };
  };

} // Belle2 namespace

#endif