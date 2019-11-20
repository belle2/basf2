/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing SVDCoGTimeCalibration calibration algorithm
   */
  class SVDCoGTimeCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to SVDCoGTimeCalibrationCollector
    explicit SVDCoGTimeCalibrationAlgorithm(const std::string& str);

    /// Destructor
    virtual ~SVDCoGTimeCalibrationAlgorithm() {}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    std::string m_id;

  };
} // namespace Belle2


