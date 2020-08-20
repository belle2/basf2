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
#include <svd/reconstruction/SVDClusterCharge.h>

#include <svd/calibration/SVDPulseShapeCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster charge
     * computed with the ELS3 algorithm.
     */
    class SVDELS3Charge : public SVDClusterCharge {

    public:

      /**
       * @return the cluster raw charge
       */
      double getClusterRawCharge();

      /**
       * @return the cluster charge
       */
      double getClusterCharge() override;

      /**
       * @return the cluster charge error
       */
      double getClusterChargeError() override;

      /**
       * @return the cluster seed charge
       */
      double getClusterSeedCharge() override;

    private:

      SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShaper calibration wrapper*/

    };

  }

}

