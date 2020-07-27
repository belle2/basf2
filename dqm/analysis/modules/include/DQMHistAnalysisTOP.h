/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TH1.h>
#include <TString.h>
#include <TLine.h>
#include <TPaveText.h>

namespace Belle2 {
  /**
   * Class for TOP histogram analysis.
   */

  class DQMHistAnalysisTOPModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisTOPModule();
    virtual ~DQMHistAnalysisTOPModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    /**
     * Find histogram corresponding to canvas.
     * @param hname Name of the histogram
     * @return The pointer to the histogram, or nullptr if not found.
     */
    TH1* find_histo_in_canvas(TString hname);
    /**
     * Find canvas by name
     * @param cname Name of the canvas
     * @return The pointer to the canvas, or nullptr if not found.
     */
    TCanvas* find_canvas(TString cname);
    //! Data members
  private:
    /** Canvas for the mean of the good hits. */
    TCanvas* m_c_goodHitsMean = nullptr;
    /** Canvas for the RMS of the good hits. */
    TCanvas* m_c_goodHitsRMS = nullptr;
    /** Canvas for the mean of the bad hits. */
    TCanvas* m_c_badHitsMean = nullptr;
    /** Canvas for the RMS of the bad hits. */
    TCanvas* m_c_badHitsRMS = nullptr;

    /** Histogram for the mean of the good hits. */
    TH1F* m_h_goodHitsMean = nullptr;
    /** Histogram for the RMS of the good hits. */
    TH1F* m_h_goodHitsRMS = nullptr;
    /** Histogram for the mean of the bad hits. */
    TH1F* m_h_badHitsMean = nullptr;
    /** Histogram for the RMS of the bad hits. */
    TH1F* m_h_badHitsRMS = nullptr;

    /** The line for the upper bound of the nornal window. */
    TLine* m_line1 = nullptr;
    /** The line for the lower bound of the nornal window. */
    TLine* m_line2 = nullptr;
    /** The text for the conditions of the nornal window. */
    TPaveText* m_text1 = nullptr;
    /** The text for the conditions of the nornal window. */
    TPaveText* m_text2 = nullptr;
  };
} // end namespace Belle2

