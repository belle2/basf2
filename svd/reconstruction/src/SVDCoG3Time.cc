/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/SVDCoG3Time.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDCoG3Time::computeClusterTime(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame)
    {
      applyCoG3Time(rawCluster, time, timeError, firstFrame);
    }

  }  //SVD namespace
} //Belle2 namespace
