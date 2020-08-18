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

#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster time
     * computed with the CoG6 algorithm.
     */
    class SVDCoG6Time : public SVDClusterTime {

    public:

      /**
       * @return the strip time
       */
      double getStripTime(Belle2::SVDShaperDigit::APVFloatSamples samples, int cellID);

      /**
       * @return the strip time error
       */
      double getStripTimeError(Belle2::SVDShaperDigit::APVFloatSamples samples, int noise, int cellID);

      /**
       * @return the first frame
       */
      int getFirstFrame() override;

      /**
       * @return the cluster time
       */
      double getClusterTime() override;

      /**
       * @return the cluster time error
       */
      double getClusterTimeError() override;

      /**
       * virtual destructor
       */
      //      virtual ~SVDCoG6Time();

    private:

      SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShaper calibration wrapper*/
      SVDCoGTimeCalibrations m_CoG6TimeCal; /**< SVD CoG6 Time calibration wrapper*/

    };

  }

}

