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

#include <svd/calibration/SVDPulseShapeCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Abstract Class representing the SVD cluster charge
     */
    class SVDClusterCharge {

    public:

      /**
       * Constructor to create an empty Cluster Charge Object
       */
      SVDClusterCharge() {};

      /**
       * @return the cluster seed charge  (as strip max sample)
       */
      double getClusterSeedCharge(const Belle2::SVD::RawCluster& rawCluster)
      {

        std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

        double rawSeedCharge = rawCluster.getSeedMaxSample();
        double seedCellID = strips.at(rawCluster.getSeedInternalIndex()).cellID;

        double seedCharge = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), seedCellID, rawSeedCharge);

        return seedCharge;

      }

      /**
       * @return the cluster charge
       */
      virtual double getClusterCharge(const Belle2::SVD::RawCluster& rawCluster) = 0;

      /**
       * @return the cluster charge error
       * the SVDCluster does not have the chargeError data member
       * but this method maybe useful for position error estimate
       */
      virtual double getClusterChargeError(const Belle2::SVD::RawCluster& rawCluster) = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterCharge() {};


    protected:


      /**SVDPulseShaper calibration wrapper*/
      SVDPulseShapeCalibrations m_PulseShapeCal;

    };

  }

}

