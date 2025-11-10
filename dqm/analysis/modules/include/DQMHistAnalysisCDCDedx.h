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

#include <TLine.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <TGaxis.h>
#include <TColor.h>
#include <TLegend.h>

#include <TF1.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2D.h>

#include <dqm/core/DQMHistAnalysis.h>
#include <cdc/geometry/CDCGeometryParConstants.h>

namespace Belle2 {

  /**
   * DQM analysis module grab canvases from DQM module and perform higher level
   * operation like histogram fitting and add many useful cosmetic. Monitoring
   * object/canvas are also saved for mirabelle.
   */


  class DQMHistAnalysisCDCDedxModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
    * constructor
    */
    DQMHistAnalysisCDCDedxModule();

    /**
    * init function for default values
    */
    void initialize() override final;

    /**
    * begin each run
    */
    void beginRun() override final;

    /**
    * event by event function
    */
    void event() override final;

    /**
    * end of each run
    */
    void endRun() override final;

    /**
    * terminating at the end of last run
    */
    void terminate() override final;

    /**
    * function to get metadata from histogram
    */
    void getMetadata();

    /**
    * function to draw dEdx+Fit
    */
    void drawDedxPR();

    /**
    * function to draw dEdx+Fit for run variation
    */
    void drawDedxIR();

    /**
    * function to draw ADC-based dead wire status of CDC
    */
    void drawWireStatus();

    /**
    * function to draw dEdx vs costh and phi
    */
    void drawDedxCosPhi();

    /**
     * function to dedx bands P
     */
    void drawBandPlot();

    /**
     * function to draw the dEdx vs injection time
     */
    void drawDedxInjTime();

    /**
    * function to draw the mean/reso of dedx vs injection time
    */
    void drawDedxInjTimeBin();

    /**
    * function to fit gaussian dist.
    * @param temphist histogram to fit
    * @param status return the status of fitting
    */
    void fitHistogram(TH1*& temphist, std::string& status);

    /**
    * function to set the mean and sigma histograms
    * @param hist 2-D histogram
    * @param hmean histogram to store the mean
    * @param hsigma histogram to store the sigma
    * @param nbin number of bins
    */
    void setHistPars(TH2D*& hist, TH1F*& hmean, TH1F*& hsigma, int nbin);

    /**
    * function to add plot style
    */
    void setPlotStyle();

    /**
    * function to add text style
    * @param obj pavetext variable
    */
    void setTextStyle(TPaveText* obj);

    /**
    * function to set the style of histogram
    * @param obj histogram
    */
    void setHistStyle(TH1* obj);

    /**
    * function to reset pad margins
    * @param l left
    * @param r right
    * @param t top
    * @param b bottom
    */
    void setPadStyle(double l, double r, double t, double b);

    /**
    * function to set the bhabha event info
    * @param pt pavetext variable
    */
    void setBEvtInfo(TPaveText* pt);

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

    std::string m_mode; /**< monitoring mode all/basic */

    //DQM analysis and Mirabelle
    TCanvas* m_c_pr_dedx = nullptr; /**< dedx dist+fit */
    TCanvas* m_c_ir_dedx = nullptr; /**< intra-run dedx status */

    TF1* f_gaus = nullptr; /**< fit function */
    TLine* l_line = nullptr; /**< line for dedx mean */

    TPaveText* m_text_dedx_fit = nullptr; /**< add dE/dx fit information */
    TPaveText* m_text_dedx = nullptr; /**< add dE/dx bhabha event information */
    TPaveText* m_text_dedx_ir = nullptr; /**< add dE/dx intra run information */

    TPaveText* m_text_bandplot = nullptr; /**< add hadron event information */
    TPaveText* m_text_dedxWire = nullptr; /**< add Wire information */

    TPaveText* m_text_mean = nullptr; /**< add dE/dx mean information */
    TPaveText* m_text_sigma = nullptr; /**< add dE/dx sigma information */

    TH1F* m_hdEdxIRMean = nullptr; /**< histogram for dE/dx mean vs events per run */
    TH1F* m_hdEdxIRSigma = nullptr; /**< histogram for dE/dx sigma vs events per run */
    TH1F* m_hMeanHer = nullptr; /**< histogram for dE/dx mean vs injection time (HER) */
    TH1F* m_hSigmaHer = nullptr; /**< histogram for dE/dx sigma vs injection time (HER) */
    TH1F* m_hMeanLer = nullptr; /**< histogram for dE/dx mean vs injection time (LER) */
    TH1F* m_hSigmaLer = nullptr; /**< histogram for dE/dx sigma vs injection time (LER) */
    TH1* m_hdEdxIRInd = nullptr; /**< histogram to calculate dE/dx mean and sigma in bins */

    TLegend* m_lego = nullptr; /**< legends for LER/HER */
    TLegend* m_legoI = nullptr; /**< legends for LER/HER for mean and sigma*/

    unsigned m_first{0};  /**< substring start value*/
    unsigned m_last{0}; /**< substring last value*/

    MonitoringObject* m_monObj = NULL; /**< MonitoringObject for mirabelle*/

    static const Int_t m_NRGBs = 6; /**<  Number of end point colors that will form the gradients*/
    static const Int_t m_NCont = 255; /**< Total number of colors in the table */

    Int_t m_pal = 0;  /**< number of colors*/
    Int_t m_palarr[m_NCont];  /**< array of colors*/
  };
} // end namespace Belle2
