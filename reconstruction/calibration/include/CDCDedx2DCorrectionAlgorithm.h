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

#include <reconstruction/dbobjects/CDCDedx2DCor.h>
#include <calibration/CalibrationAlgorithm.h>
#include "TF1.h"
#include "TH2F.h"

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx electron cos(theta) dependence
   *
   */
  class CDCDedx2DCorrectionAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedx2DCorrectionAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedx2DCorrectionAlgorithm() {}

  protected:

    /**
     * Run algorithm
     */
    virtual EResult calibrate();

  private:

    ClassDef(CDCDedx2DCorrectionAlgorithm, 0); /**< Class for CDC dE/dx 2D correction **/
  };
} // namespace Belle2
