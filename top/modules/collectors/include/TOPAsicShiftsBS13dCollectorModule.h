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
#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>

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
    bool m_requireRecBunch; /**< if true, require reconstructed bunch */

    // collections
    StoreArray<TOPDigit> m_topDigits; /**< collection of TOP digits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

  };

} // end namespace Belle2
