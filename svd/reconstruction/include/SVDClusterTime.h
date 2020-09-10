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
       * set the RawCluster, including vxdID and isUside
       */
      void setRawCluster(const Belle2::SVD::RawCluster& rawCluster)
      {
        m_rawCluster = rawCluster;
        m_vxdID = m_rawCluster.getSensorID();
        m_isUside = m_rawCluster.isUSide();
      };

      /**
       * set the trigger bin
       */
      void setTriggerBin(const int triggerBin)
      { m_triggerBin = triggerBin; };

      /**
       * @return the first frame and the cluster time
       */
      virtual std::pair<int, double> getFirstFrameAndClusterTime() = 0;

      /**
       * @return the cluster time error
       */
      virtual double getClusterTimeError() = 0;

      /**
       * virtual destructor
       */
      virtual ~SVDClusterTime() {};

      /**
       * @return the VxdID of the cluster sensor
       */
      VxdID getSensorID() {return m_vxdID;}

      /**
       * @return true if the cluster is on the U/P side
       */
      bool isUSide() {return m_isUside;}


    protected:

      /** trigger bin */
      int m_triggerBin = std::numeric_limits<int>::quiet_NaN();

      /** raw cluster used to compute the time*/
      Belle2::SVD::RawCluster m_rawCluster;

      /** VxdID of the cluster */
      VxdID m_vxdID = std::numeric_limits<int>::quiet_NaN();

      /** side of the cluster */
      bool m_isUside = std::numeric_limits<bool>::quiet_NaN();

    };

  }

}

