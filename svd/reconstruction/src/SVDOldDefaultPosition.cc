/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/reconstruction/SVDOldDefaultPosition.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDOldDefaultPosition::computeClusterPosition(Belle2::SVD::RawCluster& rawCluster, double& position,
                                                       double& positionError)
    {

      reconstructStrips(rawCluster);

      if (rawCluster.getSize() < 3)
        applyCoGPosition(rawCluster, position, positionError);
      else
        applyAHTPosition(rawCluster, position, positionError);

      //apply scale factors for the position errors
      positionError = m_OldDefaultCal.getCorrectedClusterPositionError(rawCluster.getSensorID(), rawCluster.isUSide(),
                      rawCluster.getSize(),
                      positionError);
    }

  }  //SVD namespace
} //Belle2 namespace
