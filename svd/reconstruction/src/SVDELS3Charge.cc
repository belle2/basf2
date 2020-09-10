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
#include <svd/reconstruction/SVDELS3Charge.h>
#include <svd/reconstruction/SVDChargeReconstruction.h>

#include <svd/reconstruction/SVDELS3Time.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDELS3Charge::getClusterCharge(const Belle2::SVD::RawCluster& rawCluster)
    {
      bool inElectrons = true;

      SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(rawCluster.getClsSamples(inElectrons),
          rawCluster.getSensorID(), rawCluster.isUSide());

      chargeReco->setSamplesAreInElectrons();

      double charge = chargeReco->getELS3Charge();

      return  charge;

    }


    double SVDELS3Charge::getClusterChargeError(const Belle2::SVD::RawCluster& rawCluster)
    {
      bool inElectrons = true;


      SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(rawCluster.getClsSamples(inElectrons),
          rawCluster.getSensorID(), rawCluster.isUSide());

      chargeReco->setSamplesAreInElectrons();

      double chargeError = chargeReco->getELS3ChargeError();

      return  chargeError;
    }


  }  //SVD namespace
} //Belle2 namespace
