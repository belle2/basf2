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
   * Class implementing BeamSpot calibration algorithm
   */
  class BeamSpotAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to BeamSpotCollector
    BeamSpotAlgorithm();

    /// Destructor
    virtual ~BeamSpotAlgorithm() {}

    /// Set the target lengths of the calibration intervals
    void setIntervalsLength(double tSize, double tPos) { m_tSize = tSize; m_tPos = tPos; }

    /// Set the penalty for the time-gap in the calib. interval
    void setGapPenalty(double gapPenalty) { m_gapPenalty = gapPenalty; }

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    double m_tSize = 2;    ///< Target length of the BeamSpot-size calib. interval [hours]
    double m_tPos  = 0.5;  ///< Target length of the BeamSpot-position calib. interval [hours]
    double m_gapPenalty = 10; ///< a constant scaling the time-gap penalty term in the lossFunction

  };
} // namespace Belle2


