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

    // steering parameters
    double m_timeOffset; /**< time offset */
    int m_nx; /**< number of histogram bins */

    // collections
    StoreArray<TOPDigit> m_topDigits; /**< collection of TOP digits */

  };

} // end namespace Belle2
