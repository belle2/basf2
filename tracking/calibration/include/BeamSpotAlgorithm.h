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
   * Class implementing BeamSpot calibration algorithm
   */
  class BeamSpotAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to BeamSpotCollector
    BeamSpotAlgorithm();

    /// Destructor
    virtual ~BeamSpotAlgorithm() {}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

    /// Decide if a run should be a payload boundary. Only used in certain Python Algorithm Starategies.
    virtual bool isBoundaryRequired(const Calibration::ExpRun& /*currentRun*/) override;

    std::map<std::pair<int, int>, std::pair<double, double>> getRunInfo();
    std::pair<double, double> getMinMaxTime();


  private:

    /// During findPayloadBoundaries and isBoundaryRequired this is used to define the boundaries we want.
    std::vector<Calibration::ExpRun> m_requestedBoundaries;
    std::optional<long double> m_previousRunEndTime; //time when the previous run finished


  };
} // namespace Belle2


