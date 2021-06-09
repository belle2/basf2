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
#include <svd/calibration/SVDNoiseCalibrations.h>

#include <vector>

namespace Belle2::SVD {

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

    /** set which algorithm to use for strip charge in cluster position reconstruction*/
    void set_stripChargeAlgo(const std::string& user_stripChargeAlgo) {m_stripChargeAlgo = user_stripChargeAlgo;}

    /** set which algorithm to use for strip time in cluster position reconstruction, 'dontdo' will skip it*/
    void set_stripTimeAlgo(const std::string& user_stripTimeAlgo) {m_stripTimeAlgo = user_stripTimeAlgo;}

  protected:

    /** helper, returns the sum of the strip charges*/
    double getSumOfStripCharges(const Belle2::SVD::RawCluster& rawCluster);
    /** helper, returns the sum in quadrature of the strip noise*/
    double getClusterNoise(const Belle2::SVD::RawCluster& rawCluster);

    SVDClusterCalibrations m_ClusterCal; /**< SVDCluster calibrations for the position error scale factors for OldDefault algorithm*/
    SVDNoiseCalibrations m_NoiseCal; /**< Noise calibrations for the position error*/

  private:

    std::string m_stripChargeAlgo; /**< algorithm used to reconstruct strip charge for cluster position*/
    std::string m_stripTimeAlgo; /**< algorithm used to reconstruct strip time for cluster position*/
  };

}

