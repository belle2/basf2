/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//abstract class:
#include <svd/reconstruction/SVDClusterCharge.h>
//derived classes:
#include <svd/reconstruction/SVDMaxSampleCharge.h>
#include <svd/reconstruction/SVDSumSamplesCharge.h>
#include <svd/reconstruction/SVDELS3Charge.h>

namespace Belle2::SVD {

  /**
   * Cluster Charge Factory Class
   */
  class SVDRecoChargeFactory {

  public:

    /**
     * static function that returns the
     * class to compute the cluster charge
     */
    static SVDClusterCharge* NewCharge(const std::string& description)
    {
      if (description == "MaxSample")
        return new SVDMaxSampleCharge;
      if (description == "SumSamples")
        return new SVDSumSamplesCharge;
      if (description == "ELS3")
        return new SVDELS3Charge;

      B2WARNING("the SVD cluster charge algorithm is not recognized, using SVDMaxSample!");
      return new SVDMaxSampleCharge;
    }
  };

}

