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
#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPTimeZero.h>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalModuleT0.h>


namespace Belle2 {

  /**
   * Collector for module T0 calibration with chi2 minimization of time differences
   * between slots (method DeltaT).
   *
   * Useful for initial (rough) calibration, since the results are found slightly biased.
   * For the final (precise) calibration one has to use LL method.
   */
  class TOPModuleT0DeltaTCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPModuleT0DeltaTCollectorModule();

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
    int m_numBins;      /**< number of histogram bins */
    double m_timeRange; /**< histogram time range [ns]  (symmetric around zero)*/

    // collections
    StoreArray<TOPTimeZero> m_timeZeros; /**< collection of TOP time zero's */

    // database
    DBObjPtr<TOPCalModuleT0> m_moduleT0;   /**< module T0 calibration constants */

  };

} // end namespace Belle2
