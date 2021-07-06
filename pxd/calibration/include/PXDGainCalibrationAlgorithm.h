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

    /// Estimate gain as ratio of medians from MC and data for a part of PXD
    double EstimateGain(VxdID sensorID, unsigned short uBin, unsigned short vBin);

    /// Calculate a median from unsorted signal vector. The input vector gets sorted.
    double CalculateMedian(std::vector<double>& signals);

    /// Calculate MPV from signal vector using a landau fit.
    double FitLandau(std::vector<double>& signals);

    /// Retrive charge median value from pulled in data base payload
    double GetChargeMedianFromDB(VxdID sensorID, unsigned short uBin, unsigned short vBin);

    /// Retrive current gain value from pulled in data base payload
    double GetCurrentGainFromDB(VxdID sensorID, unsigned short uBin, unsigned short vBin);
  };
} // namespace Belle2


