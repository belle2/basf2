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
//#include <pxd/dbobjects/PXDClusterShapeIndexPar.h>
//#include <pxd/dbobjects/PXDClusterPositionEstimatorPar.h>

#include <vector>
#include <set>

#include <TFile.h>

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

    /** Branches for tree */
    /** SensorID of collected digit */
    int m_sensorID;
    /** Name of cluster shape */
    std::string m_shapeName;
    /** Eta value of cluster */
    float m_clusterEta;
    /** uCellID of collected digit */
    int m_uCellID;
    /** vCellID of collected digit */
    int m_vCellID;
    /** Signal in ADU of collected digit */
    int m_signal;

    TFile* m_rootFile;

  };
} // namespace Belle2


