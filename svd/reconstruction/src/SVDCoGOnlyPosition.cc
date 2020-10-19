/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SVDCoGOnlyPosition.h>
#include <svd/reconstruction/SVDPositionReconstruction.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {


    double SVDCoGOnlyPosition::getClusterPosition(const Belle2::SVD::RawCluster& rawCluster)
    {

      //as weighted average of the strip position with strip max sample

      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      SVDPositionReconstruction* positionReco = new SVDPositionReconstruction(strips, rawCluster.getSensorID(), rawCluster.isUSide());

      double position = positionReco->getCoGPosition();

      return position;
    }

    double SVDCoGOnlyPosition::getClusterPositionError(const Belle2::SVD::RawCluster& rawCluster)
    {

      //as weighted average of the strip position with strip max sample

      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      SVDPositionReconstruction* positionReco = new SVDPositionReconstruction(strips, rawCluster.getSensorID(), rawCluster.isUSide());

      double positionError = positionReco->getCoGPositionError();

      return positionError;
    }


  }  //SVD namespace
} //Belle2 namespace
