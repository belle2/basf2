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
#include <svd/reconstruction/SVDClusterTime.h>

#include <svd/calibration/SVDCoGTimeCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster time
     * computed with the CoG6 algorithm.
     */
    class SVDCoG6Time : public SVDClusterTime {

    public:

      /**
       * computes the cluster time, timeError and FirstFrame
       * with the CoG6 algorithm
       */
      void computeClusterTime(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame);

      /**
       * @return the first frame and the cluster time
       */
      std::pair<int, double> getFirstFrameAndClusterTime(const Belle2::SVD::RawCluster& rawCluster) override;

      /**
       * @return the cluster time error
       */
      double getClusterTimeError(const Belle2::SVD::RawCluster& rawCluster) override;

      /**
       * virtual destructor
       */
      virtual ~SVDCoG6Time() {};

    private:

      SVDCoGTimeCalibrations m_CoG6TimeCal; /**< SVD CoG6 Time calibration wrapper*/

    };

  }

}

