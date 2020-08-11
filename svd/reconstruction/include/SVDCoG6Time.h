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
     * computed with the CoG6 algorithm.
     */
    class SVDCoG6Time : public SVDClusterTime {

    public:

      /**
       * @return the cluster time
       */
      double getClusterTime();

      /**
       * @return the strip time
       */
      double getStripTime(Belle2::SVDShaperDigit::APVFloatSamples samples);

      /**
       * @return the strip time error
       */
      double getStripTimeError(Belle2::SVDShaperDigit::APVFloatSamples samples, int noise);

      /**
       * @return the cluster time error
       */
      double getClusterTimeError();

      /**
       * virtual destructor
       */
      //    virtual ~SVDCoG6Time();

    };

  }

}

