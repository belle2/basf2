/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SVDELS3Time.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDELS3Time::computeClusterTime(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame)
    {
      applyELS3Time(rawCluster, time, timeError, firstFrame);
    }

  }  //SVD namespace
} //Belle2 namespace
