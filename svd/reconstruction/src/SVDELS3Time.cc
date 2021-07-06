/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
