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

    ClassDef(CDCDedxRunGainAlgorithm, 0); /**< Class for CDC dE/dx calibration algorithm **/
  };
} // namespace Belle2
