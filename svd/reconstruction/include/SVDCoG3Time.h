/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <svd/reconstruction/RawCluster.h>
#include <svd/reconstruction/SVDClusterTime.h>

#include <vector>

namespace Belle2::SVD {

  /**
   * Derived Class representing the SVD cluster time
   * computed with the CoG3 algorithm.
   */
  class SVDCoG3Time : public SVDClusterTime {

  public:

    /**
     * computes the cluster time, timeError and FirstFrame
     * with the CoG3 algorithm
     */
    void computeClusterTime(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame) override;


    /**
     * virtual destructor
     */
    virtual ~SVDCoG3Time() {};


  };

}

