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
#include <alignment/dataobjects/PedeSteering.h>

namespace Belle2 {
  class PedeSteering;
  class PedeResult;
  /**
   * Class implementing Millepede calibration algorithm
   */
  class MillepedeAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to MillepedeCalibration
    MillepedeAlgorithm();

    /// Destructor
    virtual ~MillepedeAlgorithm() {}

    PedeSteering& steering() {return m_steering;}
    alignment::PedeResult& result() {return m_result;}

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:
    PedeSteering m_steering{"PedeSteering.txt"};
    alignment::PedeResult m_result{};

    ClassDef(MillepedeAlgorithm, 0); /**< Millepede class implementing calibration algorithm */

  };
} // namespace Belle2


