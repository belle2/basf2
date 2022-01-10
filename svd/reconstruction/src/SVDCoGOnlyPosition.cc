/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/reconstruction/SVDCoGOnlyPosition.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDCoGOnlyPosition::computeClusterPosition(Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError)
    {
      reconstructStrips(rawCluster);
      applyUnfolding(rawCluster);
      applyCoGPosition(rawCluster, position, positionError);

      //apply scale factors for the position errors
      positionError = m_CoGOnlyCal.getCorrectedClusterPositionError(rawCluster.getSensorID(), rawCluster.isUSide(), rawCluster.getSize(),
                      positionError);

    }

  }  //SVD namespace
} //Belle2 namespace
