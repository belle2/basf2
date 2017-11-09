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

#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx run gains
   *
   */
  class CDCDedxRunGainAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedxRunGainAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedxRunGainAlgorithm() {}

  protected:

    /**
     * Run algorithm
     */
    virtual EResult calibrate();


  private:

  };
} // namespace Belle2
