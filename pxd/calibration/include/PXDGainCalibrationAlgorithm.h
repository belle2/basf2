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

    /// Number of bins for cluster charge
    int nBins;

    /// Artificial noise sigma for smearing cluster charge
    float noiseSigma;

    /// Lower edge of fit range in ADU
    float fitRangeLower;

    /// Upper edge of fit range in ADU
    float fitRangeUpper;

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:

    /// Optimized fit
    void FitGain(double& gain, double& chi2);

    /// Minuit minimizer for optimized fit
    TMinuit* m_Minit2h;

  };
} // namespace Belle2


