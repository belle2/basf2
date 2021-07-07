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
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>
#include <vector>

namespace Belle2::SVD {

  /**
   * Abstract Class representing the SVD cluster time
   */
  class SVDClusterTime {

  public:

    /**
     * Constructor to create an empty Cluster Time Object
     * by default returns the calibrated time
     */
    SVDClusterTime() {m_returnRawClusterTime = false;};


    /**
     * set to return the raw cluster time instead of the calibrated one
     */
    void setReturnRawClusterTime()
    {m_returnRawClusterTime = true;};

    /**
     * set the trigger bin
     */
    void setTriggerBin(const int triggerBin)
    { m_triggerBin = triggerBin; };

    /**
     * computes the cluster time, timeError and FirstFrame
     */
    virtual void computeClusterTime(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame) = 0;

    /**
     * virtual destructor
     */
    virtual ~SVDClusterTime() {};

    /** CoG6 Time Algorithm*/
    void applyCoG6Time(const Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame);

    /** CoG3 Time Algorithm*/
    void applyCoG3Time(const Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame);

    /** ELS3 Time Algorithm*/
    void applyELS3Time(const Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame);

  protected:

    /** to be used for time calibration  */
    bool m_returnRawClusterTime = false;

    /** trigger bin */
    int m_triggerBin = std::numeric_limits<int>::quiet_NaN();

    /** Hardware Clocks*/
    DBObjPtr<HardwareClockSettings> m_hwClock;

    /** APV clock period*/
    double m_apvClockPeriod = 1. / m_hwClock->getClockFrequency(Const::EDetector::SVD, "sampling");

    /**SVDPulseShaper calibration wrapper*/
    SVDPulseShapeCalibrations m_PulseShapeCal;

    /** CoG6 time calibration wrapper*/
    SVDCoGTimeCalibrations m_CoG6TimeCal;
    /** CoG3 time calibration wrapper*/
    SVD3SampleCoGTimeCalibrations m_CoG3TimeCal;
    /** ELS3 time calibration wrapper*/
    SVD3SampleELSTimeCalibrations m_ELS3TimeCal;

  };

}

