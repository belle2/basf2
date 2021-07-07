/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
