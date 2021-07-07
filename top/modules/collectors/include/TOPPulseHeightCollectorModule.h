/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>
#include <vector>
#include <string>

namespace Belle2 {

  /**
   * Collector for channel pulse-height distributions
   */
  class TOPPulseHeightCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPPulseHeightCollectorModule();

  private:

    /**
     * Replacement for initialize(). Register calibration dataobjects here as well
     */
    virtual void prepare() final;

    /**
     * Replacement for event(). Fill your calibration data objects here
     */
    virtual void collect() final;

    // steering parameters
    int m_nx; /**< number of histogram bins */
    double m_xmax; /**< histogram upper bound [ADC counts] */
    std::vector<double> m_widthWindow; /**< lower and upper bound of pulse-width window */
    std::vector<double> m_timeWindow; /**< lower and upper bound of time window */

    // collections
    StoreArray<TOPDigit> m_digits; /**< collection of TOP digits */

    // other
    std::vector<std::string> m_names; /**< histogram names */

  };

} // end namespace Belle2
