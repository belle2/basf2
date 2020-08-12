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

//abstract class:
#include <svd/reconstruction/SVDClusterTime.h>
//derived classes:
#include <svd/reconstruction/SVDCoG6Time.h>
#include <svd/reconstruction/SVDCoG3Time.h>
#include <svd/reconstruction/SVDELS3Time.h>

namespace Belle2 {

  namespace SVD {

    /**
     * Cluster Time Factory Class
     */
    class SVDRecoTimeFactory {

    public:

      static SVDClusterTime* NewTime(const std::string& description)
      {
        if (description == "CoG6")
          return new SVDCoG6Time;
        if (description == "CoG3")
          return new SVDCoG3Time;
        if (description == "ELS3")
          return new SVDELS3Time;

        B2WARNING("the SVD cluster time algorithm is not recognized, using SVDCoG6Time!");
        return new SVDCoG6Time;
      }
    };

  }

}

