/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <svd/reconstruction/RawCluster.h>
#include <svd/reconstruction/SVDClusterPosition.h>

#include <vector>

namespace Belle2::SVD {

  /**
   * Derived Class representing the SVD cluster position
   * computed with the CoGOnly algorithm.
   */
  class SVDCoGOnlyPosition : public SVDClusterPosition {

  public:

    /**
     * computes the cluster position and position error
     * with the CoG algorithm
     */
    void computeClusterPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError) override;

    /**
     * virtual destructor
     */
    virtual ~SVDCoGOnlyPosition() {};

  private:


  };

}

