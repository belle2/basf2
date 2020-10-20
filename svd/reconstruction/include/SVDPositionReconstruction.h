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
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDClusterCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Class for SVD Position computation
     */
    class SVDPositionReconstruction {

    public:

      /**
       * constructor
       */
      explicit SVDPositionReconstruction(const std::vector<Belle2::SVD::StripInRawCluster>& strips, VxdID sensorID, bool isU)
        : m_strips(strips)
        , m_vxdID(sensorID)
        , m_isUside(isU)
      { };



      /**
       * virtual destructor
       */
      virtual ~SVDPositionReconstruction() {};

      /** CoG Position */
      double getCoGPosition();
      /** CoG Position error*/
      double getCoGPositionError();
      /** CoG Position error with simple propagation formula*/
      double getCoGPositionErrorPropagation();
      /** AHT Position */
      double getAHTPosition();
      /** AHT Position error*/
      double getAHTPositionError();

    private:
      /** get the sum of strip charges*/
      double getSumOfStripCharges();

      /** get the cluster noise as sum in quadrature of strip noise (in electrons)*/
      double getClusterNoise();

    protected:

      /** strips in the cluster*/
      std::vector<Belle2::SVD::StripInRawCluster> m_strips;

      /** VxdID of the strip */
      VxdID m_vxdID = 0;

      /** side of the strip */
      bool m_isUside = 0;

      /**SVDClusterCalibration calibration wrapper*/
      SVDClusterCalibrations m_ClusterCal;

      /**SVDPulseShape calibration wrapper*/
      SVDPulseShapeCalibrations m_PulseShapeCal;
      /** Noise calibration wrapper*/
      SVDNoiseCalibrations m_NoiseCal;

    };

  }

}

