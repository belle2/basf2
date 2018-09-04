/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx electron cos(theta) dependence
   *
   */
  class CDCDedxCosineAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedxCosineAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedxCosineAlgorithm() {}

    /** * function to make flag active for method of sep */
    void setMethodSep(bool value = true) {isMethodSep = value;}

    /** * function to make flag active for plotting */
    void setMonitoringPlots(bool value = false) {isMakePlots = value;}

  protected:

    /**
     * Run algorithm
     */
    virtual EResult calibrate();

  private:

    bool isMethodSep = false; /**< if e+e- need to be consider sep */
    bool isMakePlots = false; /**< produce plots for status */

  };
} // namespace Belle2
