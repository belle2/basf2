/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vxd/dataobjects/VxdID.h>
#include <vector>


namespace Belle2 {
  /**
   * Class implementing the PXD median cluster charge calibration algorithm
   */
  class  PXDChargeCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDChargeCalibrationAlgorithm
    PXDChargeCalibrationAlgorithm();

    /// Minimum number of collected clusters for estimating median charge
    int minClusters;

    /// Artificial noise sigma for smearing cluster charge
    float noiseSigma;

    /// Safety factor for determining whether the collected number of clusters is enough
    float safetyFactor;

    /// Force continue in low statistics runs instead of returning c_NotEnoughData
    bool forceContinue;

    /// strategy to used for gain calibration, 0 for medians, 1 for landau fit
    int strategy;
  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    /// Estimate median charge form collected clusters on part of PXD
    double EstimateCharge(VxdID sensorID, unsigned short uBin, unsigned short vBin);

    /// Calculate a median from unsorted signal vector. The input vector gets sorted.
    double CalculateMedian(std::vector<double>& signals);

    /// calculate MPV of unsorted signal vector using a Landau fit
    double FitLandau(std::vector<double>& signals);
  };
} // namespace Belle2


