/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
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
#include <string>

namespace Belle2 {

  /**
   * Collector for preparing masks of hot and dead channels
   */
  class TOPChannelMaskCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPChannelMaskCollectorModule();

  private:

    /**
     * Replacement for initialize(). Register calibration dataobjects here as well
     */
    virtual void prepare() final;

    /**
     * Replacement for event(). Fill your calibration data objects here
     */
    virtual void collect() final;

    // collections
    StoreArray<TOPDigit> m_digits; /**< collection of TOP digits */

    // other
    std::vector<std::string> m_names; /**< histogram names: channel occupancies */
    std::vector<std::string> m_asicNames; /**< histogram names: window vs. asic */

  };

} // end namespace Belle2

