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
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

#include <vector>
#include <TMath.h>

namespace Belle2::SVD {

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
     * computes the cluster charge, SNR and seedCharge
     */
    virtual void computeClusterCharge(Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge) = 0;

    /**
     * virtual destructor
     */
    virtual ~SVDClusterCharge() {};

    /** MaxSample Charge Algorithm*/
    void applyMaxSampleCharge(const Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge);

    /** SumSamples Charge Algorithm*/
    void applySumSamplesCharge(const Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge);

    /** ELS3 Charge Algorithm*/
    void applyELS3Charge(const Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge);

  protected:

    /** Hardware Clocks*/
    DBObjPtr<HardwareClockSettings> m_hwClock;

    /** APV clock period*/
    double m_apvClockPeriod = 1. / m_hwClock->getClockFrequency(Const::EDetector::SVD, "sampling");

    /**SVDPulseShaper calibration wrapper*/
    SVDPulseShapeCalibrations m_PulseShapeCal;

    /**SVDNoise calibration wrapper*/
    SVDNoiseCalibrations m_NoiseCal;
  };


}

