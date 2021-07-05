/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//abstract class:
#include <svd/reconstruction/SVDClusterTime.h>
//derived classes:
#include <svd/reconstruction/SVDCoG6Time.h>
#include <svd/reconstruction/SVDCoG3Time.h>
#include <svd/reconstruction/SVDELS3Time.h>

namespace Belle2::SVD {

  /**
   * Cluster Time Factory Class
   */
  class SVDRecoTimeFactory {

  public:

    /**
     * static function that returns the
     * class to compute the cluster time
     */
    static SVDClusterTime* NewTime(const std::string& description, const bool& returnRawClusterTime)
    {
      if (description == "CoG6") {
        SVDCoG6Time* cog6 = new SVDCoG6Time();
        if (returnRawClusterTime)
          cog6->setReturnRawClusterTime();
        return cog6;
      }
      if (description == "CoG3") {
        SVDCoG3Time* cog3 = new SVDCoG3Time();
        if (returnRawClusterTime)
          cog3->setReturnRawClusterTime();
        return cog3;
      }
      if (description == "ELS3") {
        SVDELS3Time* els3 = new SVDELS3Time();
        if (returnRawClusterTime)
          els3->setReturnRawClusterTime();
        return els3;
      }
      B2WARNING("the SVD cluster time algorithm is not recognized, using SVDCoG6Time!");
      SVDCoG6Time* cog6 = new SVDCoG6Time();
      if (returnRawClusterTime)
        cog6->setReturnRawClusterTime();
      return cog6;
    }
  };

}

