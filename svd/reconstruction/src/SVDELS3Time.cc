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

      double time = m_ELS3TimeCal.getCorrectedTime(m_rawCluster.getSensorID(), m_rawCluster.isUSide(), -1, rawtime, m_triggerBin);

      return time;

    }
    double SVDELS3Time::getClusterRawTime()
    {

      //take the MaxSum 3 samples
      std::vector<float> clustered3s = m_rawCluster.getMaxSum3Samples().second;
      auto begin = clustered3s.begin();

      //calculate 'raw' ELS hit-time
      constexpr auto tau = 55; //ELS time constant, default is 55

      auto num = 2 * m_apvClockPeriod * std::exp(-4 * m_apvClockPeriod / tau) + std::exp(-2 * m_apvClockPeriod / tau) * m_apvClockPeriod /
                 2 * (*begin - std::exp(
                        -2 * m_apvClockPeriod / tau) * (*(begin + 2))) / (*begin + std::exp(-m_apvClockPeriod / tau) * (*(begin + 1)) / 2);
      auto denom = 1 - std::exp(-4 * m_apvClockPeriod / tau) - (1 + std::exp(-2 * m_apvClockPeriod / tau) / 2) * (*begin - std::exp(
                     -2 * m_apvClockPeriod / tau) * (*(begin + 2))) / (*begin + std::exp(-m_apvClockPeriod / tau) * (*(begin + 1)) / 2);
      float rawtime = - num / denom;

      return rawtime;

    }


    double SVDELS3Time::getClusterTimeError()
    {

      return  6;
    }

  }  //SVD namespace
} //Belle2 namespace
