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
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster time
     * computed with the ELS3 algorithm.
     */
    class SVDELS3Time : public SVDClusterTime {

    public:

      /**
       * @return the cluster time
       */
      double getClusterTime();

      /**
       * @return the cluster time error
       */
      double getClusterTimeError();

      /**
       * virtual destructor
       */
      //    virtual ~SVDELS3Time();

    };

  }

}

