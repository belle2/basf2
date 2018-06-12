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


#include <vector>


#include <TFile.h>
#include <TH1D.h>

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /**
   * Class implementing the PXD gain calibration algorithm
   */
  class  PXDGainCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDGainCalibrationAlgorithm
    PXDGainCalibrationAlgorithm();

    /// Minimum number of collected digits for estimating gains
    int minDigits;

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:

    /// Compute Data vs. MC residual for gain correction
    float computeResidual(float gain, const std::string& treename, const VxdID& sensorID, int areaID);

    /// Create validation histograms for Data and MC
    void createValidationHistograms(TH1D& dataHist, TH1D& mcHist, float gain, const std::string& treename, const VxdID& sensorID,
                                    int areaID);

    /** SensorID of collected digit */
    int m_sensorID;
    /** uCellID of collected digit */
    int m_uCellID;
    /** vCellID of collected digit */
    int m_vCellID;
    /** Signal in ADU of collected digit */
    int m_signal;
    /** Flag for MC data  */
    bool m_isMC;

    TFile* m_rootFile;

  };
} // namespace Belle2


