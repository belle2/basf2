/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Benjamin Schwenker, Jonas Roetter                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vxd/dataobjects/VxdID.h>
#include <vector>


namespace Belle2 {
  /**
   * Class implementing the PXD Landau calibration algorithm
   */
  class  PXDLandauCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDLandauCalibrationAlgorithm
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

    /// Estimate Landau parameters from collected clusters on part of PXD
    double EstimateLandau(VxdID sensorID, unsigned short uBin, unsigned short vBin);

    /// fit a Landau to an unsorted signal vector.
    double FitLandau(std::vector<double>& signals, const char* histname);
  };
} // namespace Belle2

