/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    explicit SVDOccupancyCalibrationsAlgorithm(const std::string& str);

    /// Destructor
    virtual ~SVDOccupancyCalibrationsAlgorithm() {}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    std::string m_id; ///< identifier

  };
} // namespace Belle2


