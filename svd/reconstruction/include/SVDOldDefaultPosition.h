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
#include <svd/reconstruction/SVDClusterPosition.h>

#include <vector>

namespace Belle2::SVD {

  /**
   * Derived Class representing the SVD cluster position
   * computed with the old algorithm (up to release-05).
   */
  class SVDOldDefaultPosition : public SVDClusterPosition {

  public:

    /**
     * computes the cluster position and position error
     * with the oldDefault algorithm
     */
    void computeClusterPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError) override;

    /**
     * virtual destructor
     */
    virtual ~SVDOldDefaultPosition() {};

  };

}

