/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Tadeas Bilka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <alignment/PedeResult.h>
#include <alignment/PedeApplication.h>
#include <alignment/dataobjects/PedeSteering.h>

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


