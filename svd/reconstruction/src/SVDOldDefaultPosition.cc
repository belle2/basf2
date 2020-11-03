/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/SVDOldDefaultPosition.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDOldDefaultPosition::computeClusterPosition(const Belle2::SVD::RawCluster& rawCluster, double& position,
                                                       double& positionError)
    {

      if (rawCluster.getSize() < 3)
        applyCoGPosition(rawCluster, position, positionError);
      else
        applyAHTPosition(rawCluster, position, positionError);
    }

  }  //SVD namespace
} //Belle2 namespace
