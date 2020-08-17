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
       * @return the first frame
       */
      int getFirstFrame() override;

      /**
       * @return the cluster raw time
       */
      double getClusterRawTime();

      /**
       * @return the cluster time
       */
      double getClusterTime() override;

      /**
       * @return the cluster time error
       */
      double getClusterTimeError() override;

      /**
       * @return the ELS time constant
       */
      double getTau() {return m_tau;};


    private:

      double m_tau = 55; /**< ELS time constant*/

      SVD3SampleELSTimeCalibrations m_ELS3TimeCal; /**< SVD ELS3 Time calibration wrapper*/

    };

  }

}

