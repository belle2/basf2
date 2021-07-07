/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalCommonT0.h>

namespace Belle2 {

  /**
   * Collector for common T0 calibration with a fit of bunch finder residuals (method BF)
   */
  class TOPCommonT0BFCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPCommonT0BFCollectorModule();

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
    int m_bunchesPerSSTclk; /**< number of bunches per SST clock */
    int m_nx;  /**< number of histogram bins */

    // collections
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    // database
    DBObjPtr<TOPCalCommonT0> m_commonT0; /**< common T0 calibration constants */

    // other
    double m_bunchTimeSep = 0; /**< bunch separation in time [ns] */

  };

} // end namespace Belle2
