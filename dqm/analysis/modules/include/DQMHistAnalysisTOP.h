/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    /** Canvas for 16 good Hits XY */
    TCanvas* m_c_good_hits_xy_[17] = {};
    /** Canvas for 16 bad Hits XY */
    TCanvas* m_c_bad_hits_xy_[17] = {};
    /** Canvas for 16 good Hits ASICS */
    TCanvas* m_c_good_hits_asics_[17] = {};
    /** Canvas for 16 bad Hits ASICS */
    TCanvas* m_c_bad_hits_asics_[17] = {};

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

