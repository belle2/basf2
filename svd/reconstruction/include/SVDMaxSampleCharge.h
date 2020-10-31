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

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster charge
     * computed summing the max sample of each strip
     */
    class SVDMaxSampleCharge : public SVDClusterCharge {

    public:

      /**
      * compute the cluster charge, charge error and SNR with MaxSample
       */
      void computeClusterCharge(Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge) override;

      /**
       * @return the cluster charge
       */
      double getClusterCharge(const Belle2::SVD::RawCluster& rawCluster) override;

      /**
       * @return the cluster charge error
       */
      double getClusterChargeError(const Belle2::SVD::RawCluster& rawCluster) override;


      /**
       * virtual destructor
       */
      virtual ~SVDMaxSampleCharge() {};


    };

  }

}

