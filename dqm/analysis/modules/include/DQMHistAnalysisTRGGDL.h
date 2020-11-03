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

    //name of skims
    static const int nskim_gdldqm = 11;
    std::string skim_smap[nskim_gdldqm] = {
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

    //efficiency histogram and bin size
    TH1D* m_h_psn_extra[nskim_gdldqm];
    TH1D* m_h_psn_pure_extra;
    TCanvas* m_c_eff[nskim_gdldqm]; /**<Canvas for TRGGDL efficiency*/
    TH1D* m_h_eff[nskim_gdldqm];
    TCanvas* m_c_eff_shifter; /**<Canvas for TRGGDL efficiency*/
    TH1D* m_h_eff_shifter;
    TCanvas* m_c_pure_eff; /**<Canvas for TRGGDL efficiency*/
    TH1D* m_h_pure_eff;
    //number of points
    static const int n_eff = 35;
    static const char* c_eff[n_eff];
    static const int n_eff_shifter = 7;
    static const char* c_eff_shifter[n_eff_shifter];
    static const int n_pure_eff = 8;
    static const char* c_pure_eff[n_pure_eff];
    //limit for CR
    TLine* m_line_limit_low_shifter[n_eff_shifter];
    TLine* m_line_limit_high_shifter[n_eff_shifter];
    double m_limit_low_shifter[n_eff_shifter] = {
      0.7, 0.7, 0.7, 0.7, 0.8, 0.8, 0.1
    };
    double m_limit_high_shifter[n_eff_shifter] = {
      0.9, 0.9, 0.9, 0.9, 1.0, 1.0, 0.2
    };
  };

} // Belle2 namespace

#endif
