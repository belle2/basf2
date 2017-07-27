/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <framework/gearbox/Const.h>
#include <top/modules/TOPGainEfficiencyMonitor/TOPLaserHitSelectorModule.h>

#include <string>

#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>

namespace Belle2 {

  /**
   * Module for channel-by-channel gain/efficiency analysis.
   * 2D histograms of hit timing and pulse height (or integrated charge), crated by TOPLaserHitSelectorModule,
   *
   */
  class TOPGainEfficiencyCalculatorModule : public HistoModule {

  public:

    /**
     * enum for the number of parameters used in fitting pulse height or charge distribution
     */
    enum { c_NParameterGainFit = 6 };

    /**
     * enum for the number of channels to show the result plots per page in an output PDF file
     */
    enum { c_NPlotsPerChannel = 3, c_NChannelPerPage = 4 };

    /**
     * Constructor
     */
    TOPGainEfficiencyCalculatorModule();

    /**
     * Destructor
     */
    virtual ~TOPGainEfficiencyCalculatorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Module funcions to define histograms
     */
    virtual void defineHisto();

    /**
     * Load 2D histograms from a given input file (output of TOPLaserHitSelector)
     * and create timing and height distribution as projection histograms for the x- and y-axis, respectively.
     * Timing cut is also applied for height distributiion
     */
    void LoadHistograms();

    /**
     * Fit height (or integrated charged) distribution to calculate gain and efficiency for each channel
     */
    void FitHistograms();

    /**
     * Draw results of gain/efficiency calculation for each channel to a given output file
     */
    void DrawResult();

    /**
     * Fit function of pulse height (or charnge) distribution for channel(pixel)-by-channel gain extraction, given by
     * "[0]*pow(x-[4],[1])*exp(-pow(x-[4],[2])/[3])"
     * smeared by Gaussian with a constant sigma to consider baseline fluctuation
     */
    static double TOPGainFunc(double* var, double* par);

  private:

    TTree* m_tree = 0; /**< ntuple to store summary of gain/efficiency calculation */
    TH2F* m_timeHeightHistogram[c_NChannelPerPMT] = {}; /**< 2D histogram of hit timing and pulse height (or charge), taken from an output file of TOPLaserHitSelector */
    TH1D* m_timeHistogram[c_NChannelPerPMT] = {}; /**< hit timing distribution, extracted from m_timeHeightHistogram as a projection along its x-axis. Used to define direct laser photon hit timing */
    TH1D* m_heightHistogram[c_NChannelPerPMT] = {};
    TF1* m_funcForFitRange[c_NChannelPerPMT] = {};
    TF1* m_funcForFullRange[c_NChannelPerPMT] = {};

    std::string m_inputFile = "";
    std::string m_outputPDFFile = "";
    short m_targetSlotId = 0;
    short m_targetPmtId = 0;

    float m_fitHalfWidth = 2.; /**< half fit width for direct laser hit peak in [ns] unit */
    float m_threshold = 100; /**< pulse height threshold, which defines lower limit of fit region and efficiency calculation */
    float m_fitMax = 0; /**<  */

    float m_fracFit = 0.99;
    float m_initialP0 = 1e-6;
    float m_initialP1 = 3.0;
    float m_initialP2 = 0.5;
    float m_initialX0 = 0.5;
    float m_pedestalSigma = 10.;

    short m_pixelId = 0;
    short m_pmtId = 0;
    short m_pmtChId = 0;
    float m_hitTiming = 0;
    float m_hitTimingSigma = 0;
    float m_gain = 0;
    float m_efficiency = 0;
    int m_nEntries = 0;
    float m_p0 = 0;
    float m_p1 = 0;
    float m_p2 = 0;
    float m_x0 = 0;
    float m_p0Error = 0;
    float m_p1Error = 0;
    float m_p2Error = 0;
    float m_x0Error = 0;
    float m_chisquare = 0;
    int m_ndf = 0;
    float m_funcFullRangeIntegral = 0;
    float m_funcFitRangeIntegral = 0;
    float m_histoFitRangeIntegral = 0;
  };

}  //namespace Belle2
