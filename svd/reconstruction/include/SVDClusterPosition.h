/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <svd/reconstruction/RawCluster.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <svd/calibration/SVDClustering.h>
#include <svd/calibration/SVDCoGOnlyPositionError.h>
#include <svd/calibration/SVDCoGOnlyErrorScaleFactors.h>
#include <svd/calibration/SVDOldDefaultErrorScaleFactors.h>
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

    /** helper, returns the average strip noise*/
    double getAverageStripNoise(const Belle2::SVD::RawCluster& rawCluster);

    SVDCoGOnlyPositionError m_CoGOnlyErr; /**< CoGOnly Position Error*/
    SVDCoGOnlyErrorScaleFactors m_CoGOnlyCal; /**< Scaling Factors for the CoGOnly algorithm*/
    SVDOldDefaultErrorScaleFactors m_OldDefaultCal; /**< Scaling Factors for the OldDefault algorithm*/
    SVDClustering m_ClusterCal; /**< SVD clustering parameters*/
    SVDNoiseCalibrations m_NoiseCal; /**< Noise calibrations for the position error*/

  private:

    std::string m_stripChargeAlgo; /**< algorithm used to reconstruct strip charge for cluster position*/
    std::string m_stripTimeAlgo; /**< algorithm used to reconstruct strip time for cluster position*/
  };

}

