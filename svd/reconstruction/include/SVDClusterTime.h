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
#include <svd/calibration/SVDCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Abstract Class representing the SVD cluster time
     */
    class SVDClusterTime {

    public:

      /**
       * Constructor to create an empty Cluster Time Object
       */
      SVDClusterTime() {};

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
       * @return the first frame and the cluster time
       */
      virtual std::pair<int, double> getFirstFrameAndClusterTime(const Belle2::SVD::RawCluster& rawCluster) = 0;

      /**
       * @return the cluster time error
       */
      virtual double getClusterTimeError(const Belle2::SVD::RawCluster& rawCluster) = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterTime() {};

      /** CoG6 Time Algorithm*/
      void applyCoG6Time(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame);

      /** CoG3 Time Algorithm*/
      void applyCoG3Time(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame);

      /** ELS3 Time Algorithm*/
      void applyELS3Time(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame);

    protected:

      /** trigger bin */
      int m_triggerBin = std::numeric_limits<int>::quiet_NaN();

      /** Hardware Clocks*/
      DBObjPtr<HardwareClockSettings> m_hwClock;
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

}

