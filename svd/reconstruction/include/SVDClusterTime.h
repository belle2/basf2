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
     * Abstract Class representing the SVD cluster time
     */
    class SVDClusterTime {

    public:

      /**
       * Constructor to create an empty Cluster Time Object
       */
      SVDClusterTime() {};

      /**
       * set the trigger bin
       */
      void setTriggerBin(const int triggerBin)
      { m_triggerBin = triggerBin; };

      /**
       * @return the first frame and the cluster time
       */
      virtual std::pair<int, double> getFirstFrameAndClusterTime(const Belle2::SVD::RawCluster& rawCluster) = 0;

      /**
       * @return the cluster time error
       */
      virtual double getClusterTimeError(const Belle2::SVD::RawCluster& rawCluster) = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterTime() {};


    protected:

      /** trigger bin */
      int m_triggerBin = std::numeric_limits<int>::quiet_NaN();

    };

  }

}

