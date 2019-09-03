/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>
#include <vector>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   * Collector for carrier shifts of BS13d
   */
  class TOPAsicShiftsBS13dCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPAsicShiftsBS13dCollectorModule();

  private:

    /**
     * Replacement for initialize(). Register calibration dataobjects here as well.
     */
    virtual void prepare() final;

    /**
     * Replacement for event(). Fill your calibration data objects here.
     */
    virtual void collect() final;

    /**
     * Replacement for defineHisto(). Create calibration dataobjects here.
     */
    virtual void inDefineHisto() final;

    // steering parameters
    double m_timeOffset; /**< time offset */
    int m_nx; /**< number of histogram bins */

    // collections
    StoreArray<TOPDigit> m_topDigits; /**< collection of TOP digits */

    // histograms
    TH2F* m_time_vs_BS = 0; /**< time vs. boardstack */
    TH1F* m_timeReference = 0; /**< reference time distribution (from BS 13a, b and c) */
    std::vector<TH1F*> m_timeCarriers; /**< time distributions of cariers of 13d */

  };

} // end namespace Belle2
