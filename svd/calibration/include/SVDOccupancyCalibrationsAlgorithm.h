/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani (2019)                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing SVDOccupancyCalibrations calibration algorithm
   */
  class SVDOccupancyCalibrationsAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to SVDOccupancyCalibrationsCollector
    SVDOccupancyCalibrationsAlgorithm(std::string str);

    /// Destructor
    virtual ~SVDOccupancyCalibrationsAlgorithm() {}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    std::string m_id;

  };
} // namespace Belle2


