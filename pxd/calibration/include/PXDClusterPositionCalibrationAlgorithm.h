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

namespace Belle2 {
  /**
   * Class implementing the PXD cluster position calibration algorithm
   */
  class  PXDClusterPositionCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDClusterPositionCalibrationAlgorithm
    PXDClusterPositionCalibrationAlgorithm();

    /// Destructor
    virtual ~PXDClusterPositionCalibrationAlgorithm() {}

    /// Minimum number of collected clusters for estimating shape likelyhood
    int minClusterForShapeLikelyhood;

    /// Minimum number of collected clusters for estimating cluster position offset
    int minClusterForPositionOffset;

  protected:

    /// Run algo on data
    virtual EResult calibrate();


  };
} // namespace Belle2


