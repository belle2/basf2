/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
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

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster position
     * computed with the center-of-gravity (CoG) algorithm.
     */
    class SVDCoGPosition : public SVDClusterPosition {

    public:

      /**
       * @return the cluster position
       */
      double getClusterPosition() override;

      /**
       * @return the cluster position error
       */
      double getClusterPositionError() override;

      /**
       * virtual destructor
       */
      //      virtual ~SVDCoGPosition();

    private:

      /**
       * @return the cluster position error
       * for cluster of size = 1
       */
      double getClusterPositionErrorSize1();

      /**
       * @return the cluster position error
       * for cluster of size > 1
       */
      double getClusterPositionErrorSize1plus();



    };

  }

}

