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
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDMaxSampleCharge::getStripCharge(int indexInRawCluster)
    {

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      //take the max sample
      double charge =  strips.at(indexInRawCluster).maxSample;
      int cellID =  strips.at(indexInRawCluster).cellID;
      // calibrate (ADC -> electrons)
      charge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, cellID, charge);

      return charge;
    }

    double SVDMaxSampleCharge::getStripChargeError(int indexInRawCluster)
    {

      //the strip charge error is simply the noise

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      //take the noise and the cellID
      double noise =  strips.at(indexInRawCluster).noise;
      int cellID =  strips.at(indexInRawCluster).cellID;
      // calibrate (ADC -> electrons)
      noise = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, cellID, noise);
      return noise;

    }

    double SVDMaxSampleCharge::getClusterCharge()
    {

      //as sum of the strip charges ( = max sample)

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double charge = 0;

      for (int i = 0; i < (int)strips.size(); i++)
        charge += getStripCharge(i);


      return charge;
    }

    double SVDMaxSampleCharge::getClusterChargeError()
    {

      //sum in quadrature of the strip noises

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double noiseSquared = 0;

      for (int i = 0; i < (int)strips.size(); i++) {
        double noise = getStripChargeError(i);
        noiseSquared += noise * noise;
      }

      return TMath::Sqrt(noiseSquared);
    }

  }  //SVD namespace
} //Belle2 namespace
