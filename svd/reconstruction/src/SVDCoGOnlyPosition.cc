/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/SVDCoGOnlyPosition.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDCoGOnlyPosition::computeClusterPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError)
    {
      reconstructStrips(rawCluster);

      applyCoGPosition(rawCluster, position, positionError);

      //apply scale factors for the position errors
      // TO DO: REMOVE this line when SVDCoGOnlyPositionError will
      // store scale factors != 1
      positionError = m_CoGOnlyCal.getCorrectedClusterPositionError(rawCluster.getSensorID(), rawCluster.isUSide(), rawCluster.getSize(),
                      positionError);

    }

  }  //SVD namespace
} //Belle2 namespace
