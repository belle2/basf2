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
#include <vxd/geometry/GeoCache.h>

#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/RawCluster.h>

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
       * set the RawCluster, including vxdID and isUside
       */
      void setRawCluster(const Belle2::SVD::RawCluster& rawCluster)
      {
        m_rawCluster = rawCluster;
        m_vxdID = m_rawCluster.getSensorID();
        m_isUside = m_rawCluster.isUSide();
      };

      /**
       * set the cut in SNR for the adjacent strip
       */
      void setCutAdjacent(float cutAdjacent)
      { m_cutAdjacent = cutAdjacent; };

      /**
       * @return the strip position
       */
      double getStripPosition(int cellID);

      /**
       * @return the strip position error
       */
      double getStripPositionError();

      /**
       * @return the cluster position
       */
      virtual double getClusterPosition() = 0;

      /**
       * @return the cluster position error
       */
      virtual double getClusterPositionError() = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterPosition() {};

      /**
       * @return the VxdID of the cluster sensor
       */
      VxdID getSensorID() {return m_vxdID;}

      /**
       * @return true if the cluster is on the U/P side
       */
      bool isUSide() {return m_isUside;}

    protected:

      /** min SNR of a strip to be considered for clustering*/
      float m_cutAdjacent = 3;

      /** raw cluster used to compute the position*/
      Belle2::SVD::RawCluster m_rawCluster;

      /** VxdID of the cluster */
      VxdID m_vxdID = 0;

      /** side of the cluster */
      bool m_isUside = 0;

    };

  }

}

