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

#include <vxd/dataobjects/VxdID.h>
#include <svd/reconstruction/RawCluster.h>
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Abstract Class representing the SVD cluster position
     */
    class SVDClusterPosition {

    public:

      /**
       * Constructor to create an empty Cluster Position Object
       */
      SVDClusterPosition() {};

      /**
       * computes the cluster position and position error
       */
      virtual void computeClusterPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError) = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterPosition() {};

      /** CoG Position Algorithm*/
      void applyCoGPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError);

      /** AHT Position Algorithm*/
      void applyAHTPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError);


    protected:

      /** helper, returns the sum pf the strip charges*/
      double getSumOfStripCharges(Belle2::SVD::RawCluster& rawCluster);


    };

  }

}

