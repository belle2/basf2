/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/SVDELS3Charge.h>
//#include <svd/reconstruction/SVDELS3Time.h>
//#include <svd/calibration/SVDNoiseCalibrations.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    void SVDELS3Charge::computeClusterCharge(Belle2::SVD::RawCluster& rawCluster, double& charge, double& SNR, double& seedCharge)
    {
      applyELS3Charge(rawCluster, charge, SNR, seedCharge);
    }


  }  //SVD namespace
} //Belle2 namespace
