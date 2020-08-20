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
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDSumSamplesCharge::getStripCharge(int indexInRawCluster)
    {

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double charge = 0 ;

      //take the sum of the samples
      for (int i = 0; i < 6; i++)
        charge +=  strips.at(indexInRawCluster).samples[i];

      int cellID = strips.at(indexInRawCluster).cellID;
      // correct by the CalPeak
      charge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, cellID, charge);

      return charge;
    }

    double SVDSumSamplesCharge::getStripChargeError(int indexInRawCluster)
    {

      //the strip charge error is simply the noise

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();
      //take the noise
      double noise = strips.at(indexInRawCluster).noise;
      int cellID = strips.at(indexInRawCluster).cellID;

      noise = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, cellID, noise);
      return noise;

    }

    double SVDSumSamplesCharge::getClusterCharge()
    {

      //as sum of the strip charges ( = sum of the samples)

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double charge = 0;

      for (int i = 0; i < (int)strips.size(); i++)
        charge += getStripCharge(i);


      return charge;
    }

    double SVDSumSamplesCharge::getClusterChargeError()
    {

      //sum in quadrature of the strip charge errors

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double weightSum = 0;
      double noise = 0;
      for (int i = 0; i < (int)strips.size(); i++) {
        noise = getStripChargeError(i);
        weightSum = noise * noise;
      }

      return TMath::Sqrt(weightSum);
    }

    double SVDSumSamplesCharge::getClusterSeedCharge()
    {

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double rawSeedCharge = m_rawCluster.getSeedMaxSample();
      double seedCellID = strips.at(m_rawCluster.getSeedInternalIndex()).cellID;

      double seedCharge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, seedCellID, rawSeedCharge);

      return seedCharge;

    }


  }  //SVD namespace
} //Belle2 namespace
