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

#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

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


    private:

      SVD3SampleCoGTimeCalibrations m_CoG3TimeCal; /**< SVD CoG3 Time calibration wrapper*/

    };

  }

}

