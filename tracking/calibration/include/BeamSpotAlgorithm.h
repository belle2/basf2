/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano de Marino, Tadeas Bilka                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing Millepede calibration algorithm
   */
  class BeamSpotAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to MillepedeCalibration
    BeamSpotAlgorithm();

    /// Destructor
    virtual ~BeamSpotAlgorithm() {}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:


  };
} // namespace Belle2


