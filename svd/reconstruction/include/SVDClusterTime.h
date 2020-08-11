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

      /** Constructor to create an empty Cluster Time Object */
      SVDClusterTime() {};

      /**
       * set the vxdID
       */
      void setSensorID(VxdID vxdID) {m_vxdID = vxdID;};

      /**
       * set the side
       */
      void setIsUside(bool isUside) {m_isUside = isUside;};

      /**
       * set the RawCluster
       */
      void setRawCluster(Belle2::SVD::RawCluster& rawCluster)
      { m_rawCluster = rawCluster; };

      /**
       * @return the cluster time
       */
      virtual double getClusterTime() = 0;

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

      double m_apvClockPeriod = 31.44; /** APV clock period*/

      /** raw cluster used to compute the time*/
      Belle2::SVD::RawCluster m_rawCluster;

      /** VxdID of the cluster */
      VxdID m_vxdID;

      /** side of the cluster */
      bool m_isUside;

    };

  }

}

