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
   * Class implementing BeamSpot calibration algorithm
   */
  class BeamSpotAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to BeamSpotCollector
    BeamSpotAlgorithm();

    /// Destructor
    virtual ~BeamSpotAlgorithm() {}

    /// Set outer loss function (for calibration intervals)
    void setOuterLoss(const std::string& loss) { m_lossFunctionOuter = loss; }

    /// Set inner loss function (for calibration subintervals)
    void setInnerLoss(const std::string& loss) { m_lossFunctionInner = loss; }
  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    /// Outer loss function (for calibration intervals of BeamSpot size parameters)
    TString m_lossFunctionOuter = "pow(rawTime - 2.0, 2) + 10 * pow(maxGap, 2)";

    /// Inner loss function (for calibration subintervals of BeamSpot position parameters)
    TString m_lossFunctionInner = "pow(rawTime - 0.5, 2) + 10 * pow(maxGap, 2)";

  };
} // namespace Belle2


