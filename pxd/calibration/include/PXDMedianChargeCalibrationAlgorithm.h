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
#include <vxd/dataobjects/VxdID.h>
#include <vector>


namespace Belle2 {
  /**
   * Class implementing the PXD median cluster charge calibration algorithm
   */
  class  PXDMedianChargeCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDMedianChargeCalibrationAlgorithm
    PXDMedianChargeCalibrationAlgorithm();

    /// Minimum number of collected clusters for estimating median charge
    int minClusters;

    /// Artificial noise sigma for smearing cluster charge
    float noiseSigma;

    /// Safety factor for determining whether the collected number of clusters is enough
    float safetyFactor;

    /// Force continue in low statistics runs instead of returning c_NotEnoughData
    bool forceContinue;

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    /// Estimate median charge form collected clusters on part of PXD
    double EstimateMedianCharge(VxdID sensorID, unsigned short uBin, unsigned short vBin);

    /// Calculate a median from unsorted signal vector. The input vector gets sorted.
    double CalculateMedian(std::vector<double>& signals);
  };
} // namespace Belle2


