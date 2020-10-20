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
       * @return the cluster position
       */
      virtual double getClusterPosition(const Belle2::SVD::RawCluster& rawCluster) = 0;

      /**
       * @return the cluster position error
       */
      virtual double getClusterPositionError(const Belle2::SVD::RawCluster& rawCluster) = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterPosition() {};


    protected:


    };

  }

}

