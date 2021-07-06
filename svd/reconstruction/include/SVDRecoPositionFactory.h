/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//abstract class:
#include <svd/reconstruction/SVDClusterPosition.h>
//derived classes:
#include <svd/reconstruction/SVDCoGOnlyPosition.h>
#include <svd/reconstruction/SVDOldDefaultPosition.h>

namespace Belle2::SVD {

  /**
   * Cluster Position Factory Class
   */
  class SVDRecoPositionFactory {

  public:

    /**
     * static function that returns the
     * class to compute the cluster position
     */
    static SVDClusterPosition* NewPosition(const std::string& description)
    {
      if (description == "CoGOnly")
        return new SVDCoGOnlyPosition;
      if (description == "OldDefault")
        return new SVDOldDefaultPosition;

      B2WARNING("the SVD cluster position algorithm is not recognized, using OldDefault!");

      return new SVDOldDefaultPosition;
    }
  };

}

