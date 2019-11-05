/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ewan Hill (ehill@mail.ubc.ca)                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TCanvas.h>
#include <TH1.h>
#include <TLine.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisEventT0Module : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
    * constructor
    */
    DQMHistAnalysisEventT0Module();

    /**
    * destructor
    */
    virtual ~DQMHistAnalysisEventT0Module();

    /**
    * init function for default values
    */
    virtual void initialize() override;

    /**
    * begin each run
    */
    virtual void beginRun() override;

    /**
    * event by event function
    */
    virtual void event() override;

    /**
    * end of each run
    */
    virtual void endRun() override;

    /**
    * terminating at the end of last run
    */
    virtual void terminate() override;

  private:

    /** Save event t0 values for ECL, CDC, and TOP for bhabha and hadronic events */

    TCanvas* c_h_eventT0_ECL_bhabha = nullptr; /**< canvas for ECL event t0 and bha bha events */
    TCanvas* c_h_eventT0_CDC_bhabha = nullptr; /**< canvas for CDC event t0 and bha bha events */
    TCanvas* c_h_eventT0_TOP_bhabha = nullptr; /**< canvas for TOP event t0 and bha bha events */
    TCanvas* c_h_eventT0_ECL_hadron = nullptr; /**< canvas for ECL event t0 and hadronic events*/
    TCanvas* c_h_eventT0_CDC_hadron = nullptr; /**< canvas for CDC event t0 and hadronic events*/
    TCanvas* c_h_eventT0_TOP_hadron = nullptr; /**< canvas for TOP event t0 and hadronic events*/

    TH1F* h_eventT0_ECL_bhabha = nullptr ; /**< event t0 histogram for ECL and bha bha events */
    TH1F* h_eventT0_CDC_bhabha = nullptr ; /**< event t0 histogram for CDC and bha bha events */
    TH1F* h_eventT0_TOP_bhabha = nullptr ; /**< event t0 histogram for TOP and bha bha events */
    TH1F* h_eventT0_ECL_hadron = nullptr ; /**< event t0 histogram for ECL and hadronic events */
    TH1F* h_eventT0_CDC_hadron = nullptr ; /**< event t0 histogram for CDC and hadronic events */
    TH1F* h_eventT0_TOP_hadron = nullptr ; /**< event t0 histogram for TOP and hadronic events */

  };
} // end namespace Belle2

