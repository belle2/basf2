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
       * set the RawCluster, including vxdID and isUside
       */
      void setRawCluster(const Belle2::SVD::RawCluster& rawCluster)
      {
        m_rawCluster = rawCluster;
        m_vxdID = m_rawCluster.getSensorID();
        m_isUside = m_rawCluster.isUSide();
      };

      /**
       * @return the cluster seed charge  (as strip max sample)
       */
      double getClusterSeedCharge()
      {

        std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

        double rawSeedCharge = m_rawCluster.getSeedMaxSample();
        double seedCellID = strips.at(m_rawCluster.getSeedInternalIndex()).cellID;

        double seedCharge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, seedCellID, rawSeedCharge);

        return seedCharge;

      }

      /**
       * @return the cluster charge
       */
      virtual double getClusterCharge() = 0;

      /**
       * @return the cluster charge error
       * the SVDCluster does not have the chargeError data member
       * but this method maybe useful for position error estimate
       */
      virtual double getClusterChargeError() = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterCharge() {};

      /**
       * @return the VxdID of the cluster sensor
       */
      VxdID getSensorID() {return m_vxdID;}

      /**
       * @return true if the cluster is on the U/P side
       */
      bool isUSide() {return m_isUside;}

    protected:

      /** raw cluster used to compute the charge*/
      Belle2::SVD::RawCluster m_rawCluster;

      /** VxdID of the cluster */
      VxdID m_vxdID = 0;

      /** side of the cluster */
      bool m_isUside = 0;

    private:

      /**SVDPulseShaper calibration wrapper*/
      SVDPulseShapeCalibrations m_PulseShapeCal;

    };

  }

}

