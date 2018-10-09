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
  class  PXDLandauCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDMedianChargeCalibrationAlgorithm
    PXDLandauCalibrationAlgorithm();

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
    virtual EResult calibrate();

  private:

    /// Estimate median charge form collected clusters on part of PXD
    std::pair<float, float> EstimateLandau(VxdID sensorID, unsigned short uBin, unsigned short vBin);

    /// Calculate a median from unsorted signal vector. The input vector gets sorted.
    std::pair<float, float> FitLandau(std::vector<double>& signals);
  };
} // namespace Belle2

