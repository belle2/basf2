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
#include <svd/reconstruction/SVDSumSamplesCharge.h>
#include <svd/reconstruction/SVDChargeReconstruction.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {


    double SVDSumSamplesCharge::getClusterCharge()
    {

      //as sum of the strip charges ( = sum of the samples)

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double charge = 0;

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::stripInRawCluster strip = strips.at(i);

        SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(strip, m_rawCluster.getSensorID(), m_rawCluster.isUSide());

        float noiseInADC = strip.noise;
        float noiseInElectrons = m_PulseShapeCal.getChargeFromADC(m_rawCluster.getSensorID(), m_rawCluster.isUSide(), strip.cellID,
                                                                  noiseInADC);
        chargeReco->setAverageNoise(noiseInADC, noiseInElectrons);

        charge += chargeReco->getSumSamplesCharge();
      }


      return charge;
    }

    double SVDSumSamplesCharge::getClusterChargeError()
    {

      //sum in quadrature of the strip charge errors

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double noiseSquared = 0;

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::stripInRawCluster strip = strips.at(i);

        SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(strip, m_rawCluster.getSensorID(), m_rawCluster.isUSide());
        float noiseInADC = strip.noise;
        float noiseInElectrons = m_PulseShapeCal.getChargeFromADC(m_rawCluster.getSensorID(), m_rawCluster.isUSide(), strip.cellID,
                                                                  noiseInADC);
        chargeReco->setAverageNoise(noiseInADC, noiseInElectrons);

        double noise = chargeReco->getSumSamplesChargeError();

        noiseSquared += noise * noise;
      }

      return TMath::Sqrt(noiseSquared);
    }

  }  //SVD namespace
} //Belle2 namespace
