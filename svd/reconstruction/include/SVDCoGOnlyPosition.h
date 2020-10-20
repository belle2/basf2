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

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster position
     * computed with the CoGOnly algorithm.
     */
    class SVDCoGOnlyPosition : public SVDClusterPosition {

    public:

      /**
       * @return the first frame and the cluster position
       */
      double getClusterPosition(const Belle2::SVD::RawCluster& rawCluster) override;

      /**
       * @return the cluster position error
       */
      double getClusterPositionError(const Belle2::SVD::RawCluster& rawCluster) override;

      /**
       * virtual destructor
       */
      virtual ~SVDCoGOnlyPosition() {};

    private:


    };

  }

}

