/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <svd/reconstruction/RawCluster.h>
#include <svd/reconstruction/SVDClusterCharge.h>

#include <svd/calibration/SVDPulseShapeCalibrations.h>

#include <vector>

namespace Belle2::SVD {

  /**
   * Derived Class representing the SVD cluster charge
   * computed with the ELS3 algorithm.
   * ELS3:
   * Least-Squares method with the Exponentially decaying waveform.
   */
  class SVDELS3Charge : public SVDClusterCharge {

  public:

    /**
     * virtual destructor
     */
    virtual ~SVDELS3Charge() {};

    /**
    * compute the cluster charge, charge error and SNR with ELS3
     */
    void computeClusterCharge(Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge) override;

  };

}

