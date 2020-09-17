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
#include <svd/reconstruction/SVDClusterPosition.h>
//derived classes:
#include <svd/reconstruction/SVDCoGOnlyPosition.h>

namespace Belle2 {

  namespace SVD {

    /**
     * Cluster Position Factory Class
     */
    class SVDRecoPositionFactory {

    public:

      static SVDClusterPosition* NewPosition(const std::string& description)
      {
        /*        if (description == "CoG6")
                return new SVDCoG6Position;
              if (description == "CoG3")
                return new SVDCoG3Position;
              if (description == "ELS3")
                return new SVDELS3Position;

              B2WARNING("the SVD cluster position algorithm is not recognized, using SVDCoG6Position!");
        */

        return new SVDCoGOnlyPosition;
      }
    };

  }

}

