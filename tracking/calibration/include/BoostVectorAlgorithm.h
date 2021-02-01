/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing BoostVector calibration algorithm
   */
  class BoostVectorAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to BoostVectorCollector
    BoostVectorAlgorithm();

    /// Destructor
    virtual ~BoostVectorAlgorithm() {}

    /// Set outer loss function (for calibration intervals)
    void setOuterLoss(const std::string& loss) { m_lossFunctionOuter = loss; }

    /// Set inner loss function (for calibration subintervals)
    void setInnerLoss(const std::string& loss) { m_lossFunctionInner = loss; }

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    /// Outer loss function (for calibration intervals of BoostVector spread parameters)
    TString m_lossFunctionOuter = "pow(rawTime - 8.0, 2) + 10 * pow(maxGap, 2)";

    /// Inner loss function (for calibraion subintervals of mean BoostVector values)
    TString m_lossFunctionInner = "pow(rawTime - 8.0, 2) + 10 * pow(maxGap, 2)";
  };
} // namespace Belle2


