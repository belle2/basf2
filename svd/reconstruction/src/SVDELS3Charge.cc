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

#include <svd/reconstruction/SVDELS3Time.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDELS3Charge::getClusterCharge()
    {

      //take raw cluster charge
      double charge = getClusterRawCharge();

      //compute gain as weighted average, weighting strip gain (e-/ADC) with the strip max sample

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double weight = 0;
      double sumMaxSamples = 0;
      for (auto s : strips) {
        weight += s.maxSample * m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, s.cellID, 1);

        sumMaxSamples += s.maxSample;
      }
      weight = weight / sumMaxSamples;

      charge = weight * charge;

      return charge;
    }

    double SVDELS3Charge::getClusterRawCharge()
    {

      //take the MaxSum 3 samples
      std::vector<float> clustered3s = m_rawCluster.getMaxSum3Samples().second;
      auto begin = clustered3s.begin();

      //calculate 'raw' ELS hit-charge

      SVDELS3Time helper;
      helper.setRawCluster(m_rawCluster);

      double tau = helper.getTau();
      double apv = helper.getAPVClockPeriod();
      double rawtime = helper.getClusterRawTime();

      const double E = std::exp(apv / tau);

      double num0 = (1 + 2. / E / E) * (*(begin));
      double num1 = (E - 1. / E / E / E) * (*(begin + 1));
      double num2 = (2 + 1. / E / E) * (*(begin + 2));
      double numTot = num1 + num2 - num0;

      double den0 = apv / tau * std::exp(1 + rawtime / tau);
      double den1 = 1 + 4. / E / E + 1. / E / E / E / E;
      double denTot = den0 * den1;

      double rawcharge = numTot / denTot;


      return  rawcharge;

    }


    double SVDELS3Charge::getClusterChargeError()
    {

      return  6;
    }


    double SVDELS3Charge::getClusterSeedCharge()
    {

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double rawSeedCharge = m_rawCluster.getSeedMaxSample();
      double seedCellID = strips.at(m_rawCluster.getSeedInternalIndex()).cellID;

      double seedCharge = m_PulseShapeCal.getChargeFromADC(m_vxdID, m_isUside, seedCellID, rawSeedCharge);

      return seedCharge;

    }

  }  //SVD namespace
} //Belle2 namespace
