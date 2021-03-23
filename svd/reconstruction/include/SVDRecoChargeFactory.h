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

