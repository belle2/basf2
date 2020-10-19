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
#include <svd/reconstruction/SVDMaxSampleCharge.h>
#include <svd/reconstruction/SVDChargeReconstruction.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {


    double SVDMaxSampleCharge::getClusterCharge(const Belle2::SVD::RawCluster& rawCluster)
    {

      //as sum of the strip charges ( = max sample)

      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      double charge = 0;

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        SVDChargeReconstruction chargeReco(strip, rawCluster.getSensorID(), rawCluster.isUSide());

        float noiseInADC = strip.noise;
        float noiseInElectrons = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID,
                                                                  noiseInADC);
        chargeReco.setAverageNoise(noiseInADC, noiseInElectrons);

        charge += chargeReco.getMaxSampleCharge();
      }


      return charge;
    }

    double SVDMaxSampleCharge::getClusterChargeError(const Belle2::SVD::RawCluster& rawCluster)
    {

      //sum in quadrature of the strip noises

      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      double noiseSquared = 0;

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        SVDChargeReconstruction chargeReco(strip, rawCluster.getSensorID(), rawCluster.isUSide());

        float noiseInADC = strip.noise;
        float noiseInElectrons = m_PulseShapeCal.getChargeFromADC(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID,
                                                                  noiseInADC);
        chargeReco.setAverageNoise(noiseInADC, noiseInElectrons);

        double noise = chargeReco.getMaxSampleChargeError();

        noiseSquared += noise * noise;
      }

      return TMath::Sqrt(noiseSquared);
    }

  }  //SVD namespace
} //Belle2 namespace
