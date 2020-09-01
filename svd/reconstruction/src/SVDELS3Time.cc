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
#include <svd/reconstruction/SVDELS3Time.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    int SVDELS3Time::getFirstFrame()
    {
      return m_rawCluster.getMaxSum3Samples().first;
    }

    double SVDELS3Time::getClusterTime()
    {
      float rawtime = getClusterRawTime();

      double time = m_ELS3TimeCal.getCorrectedTime(m_vxdID, m_isUside, -1, rawtime, m_triggerBin);

      return time;

    }
    double SVDELS3Time::getClusterRawTime()
    {

      //take the MaxSum 3 samples
      std::vector<float> clustered3s = m_rawCluster.getMaxSum3Samples().second;
      auto begin = clustered3s.begin();

      //calculate 'raw' ELS hit-time
      const double E = std::exp(- m_apvClockPeriod / m_tau);
      const double E2 = E * E;
      const double E4 = E * E * E * E;
      const double a0 = (*begin);
      const double a1 = (*(begin + 1));
      const double a2 = (*(begin + 2));
      const double w = (a0 -  E2 * a2) / (2 * a0 + E * a1);

      auto num  = 2 * E4 + w * E2;
      auto den =  1 - E4 - w * (2 + E2);

      float rawtime = - m_apvClockPeriod * num / den;

      return rawtime;

    }


    double SVDELS3Time::getClusterTimeError()
    {
      //take the MaxSum 3 samples
      std::vector<float> clustered3s = m_rawCluster.getMaxSum3Samples().second;
      auto begin = clustered3s.begin();

      //calculate 'raw' ELS hit-time error
      //neglecting errors from the calibration function

      const double E = std::exp(- m_apvClockPeriod / m_tau);
      const double E2 = E * E;
      const double E3 = E * E * E;
      const double E4 = E * E * E * E;
      const double a0 = (*begin);
      const double a1 = (*begin + 1);
      const double a2 = (*begin + 2);
      const double w = (a0 -  E2 * a2) / (2 * a0 + E * a1);

      //computing dT/dw:
      double num = E2 * (1 + 4 * E2 + E4);
      double den = TMath::Power(1 - E4 - w * (2 + E2), 2);
      double dTdw = - m_apvClockPeriod * num / den;

      //computing dw/da0
      num = E * a1 + 2 * E2 * a2;
      den = TMath::Power(2 * a0 + E * a1, 2);
      double dwda0 = num / den;

      //computing dw/da1
      num = -E * a0 + E3 * a2;
      den = TMath::Power(2 * a0 + E * a1, 2);
      double dwda1 = num / den;

      //computing dw/da2
      num = -E2;
      den = 2 * a0 + E * a1;
      double dwda2 = num / den;

      //error on a0,a1,a2 are equal (independent on the sample)
      //computing delta_a = sum in quadrature of the noise in electrons of the strips in the cluster
      std::vector<stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();
      double deltaa = 0;
      SVDNoiseCalibrations noise;
      for (auto strip : strips)
        deltaa += TMath::Power(noise.getNoiseInElectrons(m_vxdID, m_isUside, strip.cellID), 2);
      deltaa = std::sqrt(deltaa);

      double timeError = std::abs(deltaa * dTdw) * std::sqrt(dwda0 * dwda0 + dwda1 * dwda1 + dwda2 * dwda2);
      //      B2INFO("deltaa = " << deltaa << ", dTdw = " << dTdw << ", dwda0 = " << dwda0 << ", dwda1 = " << dwda1 << ", dwda2 = " << dwda2);

      return  timeError;

    }

  }  //SVD namespace
} //Belle2 namespace
