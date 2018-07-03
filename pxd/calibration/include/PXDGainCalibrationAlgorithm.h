/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Benjamin Schwenker                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vector>


class TMinuit;

namespace Belle2 {
  /**
   * Class implementing the PXD gain calibration algorithm
   */
  class  PXDGainCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDGainCalibrationAlgorithm
    PXDGainCalibrationAlgorithm();

    /// Minimum number of collected clusters for estimating gains
    int minClusters;

    /// Artificial noise sigma for smearing cluster charge
    float noiseSigma;

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:

    /// Estimate gain as ratio of medians from MC and data
    double EstimateGain();

    /// Calculate a median from unsorted signal vector. The input vector gets sorted.
    double CalculateMedianOfSignals(std::vector<double>& signals);
  };
} // namespace Belle2


