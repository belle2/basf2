/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <boost/format.hpp>
#include <TROOT.h>

#include "TLine.h"
#include "TPaveText.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TColor.h"

#include "TF1.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2D.h"

#include <dqm/analysis/modules/DQMHistAnalysis.h>

namespace Belle2 {

  /**
   * DQM analysis module grab canvases from DQM module and perform higher level
   * operation like histogram fitting and add many useful cosmetic. Monitoring
   * object/canvas are also saved for mirabelle.
   */


  class DQMHistAnalysisCDCDedxModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
    * constructor
    */
    DQMHistAnalysisCDCDedxModule();

    /**
    * destructor
    */
    virtual ~DQMHistAnalysisCDCDedxModule();

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

    /**
    * funtion to get metadata from histogram
    */
    void getMetadata();

    /**
    * funtion to draw dEdx+Fit
    */
    void drawDedx();

    /**
    * funtion to draw dEdx+Fit for run variation
    */
    void drawDedxIR();

    /**
    * funtion to draw ADC-based dead wire status of CDC
    */
    void drawWireStatus();

    /**
    * funtion to draw dEdx vs costh and phi
    */
    void drawDedxCosPhi();

    /**
    * funtion to dedx bands P
    */
    void drawBandPlot();

    /**
    * funtion to fit gaussian dist.
    */
    void fitHistogram(TH1D*& temphist, std::string& status);

    /**
    * funtion to add plot style
    */
    void set_Plot_Style();

    /**
    * funtion to add text style
    */
    void set_Text_Style(TPaveText*& obj);

    /**
    * funtion to reset pad margins
    */
    void set_Hist_Style(TH1* obj);

    /**
    * funtion to reset pad margins
    */
    void set_Pad_Style(double l, double r, double t, double b);


  private:

    std::string m_iexprun{""}; /**<  info of exp/run */
    std::string m_status{""}; /**<  fit status */
    double m_mean{0.0}; /**< fit mean */
    double m_sigma{0.0}; /**< fit sigma */

    int m_exp{0}; /**< exp number */
    int m_run{0}; /**< run number */
    double m_dbrg{0.}; /**< previous run gain */
    int m_nallevt{0}; /**< all events */
    int m_nbhabhaevt{0}; /**< bhabha events */
    int m_nhadevt{0}; /**< hadron events */

    //DQM analysis and Mirabelle
    TCanvas* c_pr_dedx = nullptr; /**< dedx dist+fit */
    TCanvas* c_pr_dedxphi = nullptr; /**< dedx vs phi  */
    TCanvas* c_pr_dedxcos = nullptr; /**< dedx vs costh  */
    TCanvas* c_pr_bands = nullptr; /**< dedx band plot */
    TCanvas* c_pr_wires = nullptr; /**< dead wire status */
    TCanvas* c_ir_dedx = nullptr; /**< intra-run dedx status */
    TCanvas* c_ir_mean = nullptr; /**< intra-run dedx mean */
    TCanvas* c_ir_reso = nullptr; /**< intra-run dedx sigma */

    TF1* f_gaus = nullptr; /**< fit function */
    TLine* l_line = nullptr; /**< line for dedx mean */

    unsigned first{0};/**< substring start value*/
    unsigned last{0};/**< substring last value*/

    MonitoringObject* m_monObj = NULL; /**< MonitoringObject for mirabelle*/

  };
} // end namespace Belle2

