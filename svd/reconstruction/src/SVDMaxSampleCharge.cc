/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/reconstruction/SVDMaxSampleCharge.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {


    void SVDMaxSampleCharge::computeClusterCharge(Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge)
    {
      applyMaxSampleCharge(rawCluster, charge, SNR, seedCharge);
    }

  }  //SVD namespace
} //Belle2 namespace
