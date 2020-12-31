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
#include <framework/dbobjects/HardwareClockSettings.h>
#include <svd/calibration/SVDClusterCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Abstract Class representing the SVD cluster position
     */
    class SVDClusterPosition {

    public:

      /**
       * Constructor to create an empty Cluster Position Object
       */
      SVDClusterPosition() {};

      /**
       * computes the cluster position and position error
       */
      virtual void computeClusterPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError) = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterPosition() {};

      /** CoG Position Algorithm*/
      void applyCoGPosition(const Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError);

      /** AHT Position Algorithm*/
      void applyAHTPosition(const Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError);

      /** reconstruct strips*/
      void reconstructStrips(Belle2::SVD::RawCluster& rawCluster);

      void set_stripChargeAlgo(const std::string& user_stripChargeAlgo) {m_stripChargeAlgo = user_stripChargeAlgo;}

      void set_stripTimeAlgo(const std::string& user_stripTimeAlgo) {m_stripTimeAlgo = user_stripTimeAlgo;}

    protected:

      /** helper, returns the sum pf the strip charges*/
      double getSumOfStripCharges(const Belle2::SVD::RawCluster& rawCluster);

    private:

      SVDClusterCalibrations m_ClusterCal; /**<SVDCluster calibrations for the position error scale factors for oldDefault algorithm*/

      std::string m_stripChargeAlgo;
      std::string m_stripTimeAlgo;
    };

  }

}

