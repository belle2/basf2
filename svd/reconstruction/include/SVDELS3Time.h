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
#include <svd/reconstruction/SVDClusterTime.h>

#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster time
     * computed with the ELS3 algorithm.
     */
    class SVDELS3Time : public SVDClusterTime {

    public:

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
      virtual ~SVDELS3Time() {};

    private:

      SVD3SampleELSTimeCalibrations m_ELS3TimeCal; /**< SVD ELS3 Time calibration wrapper*/

    };

  }

}

