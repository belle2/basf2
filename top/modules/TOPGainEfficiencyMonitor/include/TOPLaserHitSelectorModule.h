/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke, Okuto Rikuya                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <framework/gearbox/Const.h>

#include <TH2.h>

namespace Belle2 {

  /**
   * enum for maximum number of array elements (# of hits per event)
   */
  enum { c_NChannelPerAsic = 8, c_NModule = 16, c_NChannelPerPMT = 16,
         c_NChannelPerPMTRow = 4, c_NPMTPerRow = 16,
         c_NPMTPerModule = 32, c_NPixelPerRow = 64, c_NPixelPerModule = 512
       };

  /**
   * Module for pixel-by-pixel gain/efficiency analysis
   */
  class TOPLaserHitSelectorModule : public HistoModule {

  public:

    /**
     * structure to hold hit information, used in double cal. pulse identification
     */
    typedef struct {
      float m_time; /**< timing of the hit */
      float m_height; /**< pulse height of the hit */
    } hitInfo_t;

    /**
     * Constructor
     */
    TOPLaserHitSelectorModule();

    /**
     * Destructor
     */
    virtual ~TOPLaserHitSelectorModule();

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
     * create timing-height 2D histograms for all 8192 pixels
     */
    virtual void defineHisto();

  private:

    TH2F* m_TimeHeightHistogramForFit[c_NPixelPerModule *
                                      c_NModule]; /**< array of histogram pointer to 2D histogram of hit timing vs pulse height distribution for each pixel (all 8,192 pixels) for gain*/
    TH2F* m_TimeHeightHistogramForHitRate[c_NPixelPerModule *
                                          c_NModule]; /**< array of histogram pointer to 2D histogram of hit timing vs pulse height distribution for each pixel (all 8,192 pixels) for efficiency */
    TH2F* m_TimeIntegralHistogramForFit[c_NPixelPerModule *
                                        c_NModule]; /**< array of histogram pointer to 2D histogram of hit timing vs integral distribution for each pixel (all 8,192 pixels) for gain*/
    TH1F* m_nCalPulseHistogram; /**< histogram to store the number of events with calibration pulse(s) identified for each asic (1,024 in total),
         the x-axis means global asic ID, defined as (slotNum-1)*64+(pixelID-1) */

    std::vector<int>
    m_timeHistogramBinning; /**< histogram binning of hit timing distribution, in the order of number of bins, lower limit, upper limit */
    std::vector<int>
    m_chargeHistogramBinning; /**< histogram binning of pulse height distribution, in the order of number of bins, lower limit, upper limit */

    bool m_useDoublePulse = true; /**< set true when you require both of double calibration pulses for reference timing */
    float m_calibrationPulseThreshold1 =
      600; /**< minimum pulse height for the first calibration pulse to be qualified as calibration signals */
    float m_calibrationPulseThreshold2 =
      450; /**< minimum pulse height for the secon calibration pulse to be qualified as calibration signals */
    float m_calibrationPulseInterval = 21.85; /**< nominal DeltaT value (time interval of two calibration signals) in a unit of ns */
    float m_calibrationPulseIntervalRange = 2; /**< tolerable shift of DeltaT from its nominal before calibration in a unit of ns */
    int m_windowSelect = 0; /**< select window number is [All=0, Odd=2, Even=1]*/
    bool m_includePrimaryChargeShare = false; /**< set true when you require without chargeshare cut for making 2D histogram */
    bool m_includeAllChargeShare = false; /**< set true when you require without chargeshare cut for making 2D histogram */
  };

}  //namespace Belle2
