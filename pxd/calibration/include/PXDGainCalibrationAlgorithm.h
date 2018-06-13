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
//#include <pxd/dbobjects/PXDGainMapPar.h>
#include <vxd/dataobjects/VxdID.h>

#include <vector>

#include <TFile.h>
#include <TH1D.h>


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

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:

    /// Create validation histograms for Data and MC
    void createValidationHistograms(TH1D& dataHist, TH1D& mcHist, float gain);

    /// Optimized fit
    void FitGain(double& gain, double& chi2);

    /** Root file containing validation histos*/
    TFile* m_rootFile;

    /** minuit minimizer for optimized fit*/
    TMinuit* m_Minit2h;

  };
} // namespace Belle2


