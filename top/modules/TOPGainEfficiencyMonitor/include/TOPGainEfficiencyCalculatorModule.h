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
     * Load time vs height 2D histogram from a given input file (paramter "inputFile")
     * and prepare hit timing and pulse height distribution for each channel.
     */
    virtual void initialize();

    /**
     * The main processes, fitting height distribution and calculating gain/efficiency,
     * are done in this function.
     */
    virtual void beginRun();

    /**
     * This will be empty as the all the processes are done in beginRun() function
     * thus input file can be a dummy file.
     */
    virtual void event();

    /**
     * Draw plots to show fitting results for each channel and save them into a given PDF file (outputPDFFile).
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Define TTree branches to store fit results for each channel
     * This TTree is saved in an output file given by "histoFileName" parameter of "HistoManager" module.
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

    /**
     * Find peak and return its position for a limited range of x (x smaller than the given value (xmax))
     */
    static double FindPeakForSmallerXThan(TH1* histo, double xmax = 0);

  private:

    TTree* m_tree = 0; /**< ntuple to store summary of gain/efficiency calculation */
    TH2F* m_timeHeightHistogram[c_NChannelPerPMT] = {}; /**< 2D histogram of hit timing and pulse height (or charge), taken from an output file of TOPLaserHitSelector */
    TH1D* m_timeHistogram[c_NChannelPerPMT] = {}; /**< hit timing distribution, extracted from m_timeHeightHistogram as a projection along its x-axis. Used to define direct laser photon hit timing */
    TH1D* m_heightHistogram[c_NChannelPerPMT] = {}; /**< pulse height distribution, extracted from m_timeHeightHistogram as a projection along its y-axis with timing cut. Used gain/efficiency calculation. */
    TH1F* m_nCalPulseHistogram =
      0; /**< histogram to store the number of events with calibration pulse(s) identified for each asic (1,024) in total */
    TF1* m_funcForLaser[c_NChannelPerPMT] = {}; /**< array of TF1 pointer to store fit function for hit timing distribution */
    TF1* m_funcForFitRange[c_NChannelPerPMT] = {}; /**< array of TF1 pointer to store fit function for pulse height distribution, defined only for fit region */
    TF1* m_funcForFullRange[c_NChannelPerPMT] = {}; /**< array of TF1 pointer to store fit function for pulse height distribution, defined only for full range of pulse height */

    std::string m_inputFile = ""; /**< input file containing timing vs height 2D histograms (output of TOPLaserHitSelector) */
    std::string m_outputPDFFile =
      ""; /**< output PDF file to store plots of 2D histogram, timing, and height distribution for each channel */
    short m_targetSlotId = 0; /**< slot ID */
    short m_targetPmtId = 0; /**< PMT ID */

    float m_fitHalfWidth = 1.; /**< half fit width for direct laser hit peak in [ns] unit */
    float m_threshold = 100; /**< pulse height threshold, which defines lower limit of fit region and efficiency calculation */
    float m_fitMax = 0; /**< upper limit of fit region for pulse height distribution, determined based on m_fracFit value */
    float m_fracFit = 0.99; /**< fraction of events which are covered by an area [0,m_fitMax] */
    float m_initialP0 = 1e-6; /**< initial value of the fit parameter p0 */
    float m_initialP1 = 3.0; /**< initial value of the fit parameter p1 */
    float m_initialP2 = 0.5; /**< initial value of the fit parameter p2 */
    float m_initialX0 = 500; /**< initial value of the fit parameter x0 */
    float m_pedestalSigma = 10.; /**< sigma of pedestal */

    short m_pixelId = 0; /**< pixel ID, calculated from PMT ID and PMT channel ID */
    short m_pmtChId = 0; /**< PMT channel ID */
    float m_hitTiming = 0; /**< timing of laser direct photon hits, given by Gaussian fit mean */
    float m_hitTimingSigma = 0; /**< Gaussian fit sigma for a peak of laser direct photons in hit timing distribution */
    int m_nEntries = 0; /**< entries of pulse height distribution */
    int m_nCalPulse = 0; /**< the number of events with calibration pulse(s) identified */
    int m_nOverflowEvents = 0; /**< the number of events outside histogram range */
    float m_meanPulseHeight = 0; /**< histogram mean of pulse height distribution */
    float m_gain = 0; /**< calculated gain from fitting of pulse height distribution */
    float m_efficiency = 0; /**< calculated efficiency from fitting of pulse height distribution */
    float m_p0 = 0; /**< fit result of p0 */
    float m_p1 = 0; /**< fit result of p1 */
    float m_p2 = 0; /**< fit result of p2 */
    float m_x0 = 0; /**< fit result of x0 */
    float m_p0Error = 0; /**< fit error of p0 */
    float m_p1Error = 0; /**< fit error of p1 */
    float m_p2Error = 0; /**< fit error of p2 */
    float m_x0Error = 0; /**< fit error of x0 */
    float m_chisquare = 0; /**< chi2 of fitting */
    int m_ndf = 0; /**< NDF of fitting */
    float m_funcFullRangeIntegral = 0; /**< integral of fit function for its full range  */
    float m_funcFitRangeIntegral = 0; /**< integral of fit function for a range [threshold, fitMax] */
    float m_histoFitRangeIntegral = 0; /**< integral of histogram for a range [threshold, fitMax] */
    float m_histoMeanAboveThre = 0; /**< mean of histogram above threshold, ignore overflow bin */
  };

}  //namespace Belle2
