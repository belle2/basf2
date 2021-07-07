/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <svd/reconstruction/RawCluster.h>
#include <svd/reconstruction/SVDClusterCharge.h>

#include <vector>

namespace Belle2::SVD {

  /**
   * Derived Class representing the SVD cluster charge
   * computed summing the samples of each strip
   */
  class SVDSumSamplesCharge : public SVDClusterCharge {

  public:

    /**
    * compute the cluster charge, charge error and SNR with SumSample
     */
    void computeClusterCharge(Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge) override;

    /**
     * virtual destructor
     */
    virtual ~SVDSumSamplesCharge() {};


  };

}

