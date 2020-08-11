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
#include <svd/reconstruction/SVDCoG3Time.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDCoG3Time::getClusterTime()
    {

      //take the MaxSum 3 samples
      std::vector<float> clustered3s = m_rawCluster.getMaxSum3Samples().second;
      auto begin = clustered3s.begin();
      const auto end = clustered3s.end();

      auto retval = 0., norm = 0.;
      for (auto step = 0.; begin != end; ++begin, step += m_apvClockPeriod) {
        norm += static_cast<double>(*begin);
        retval += static_cast<double>(*begin) * step;
      }
      float rawtime = retval / norm;

      return rawtime;

    }


    double SVDCoG3Time::getClusterTimeError()
    {

      //take the MaxSum 3 samples
      std::vector<float> clustered3s = m_rawCluster.getMaxSum3Samples().second;
      auto begin = clustered3s.begin();
      const auto end = clustered3s.end();


      //sum of samples amplitudes
      float Atot = 0;
      //sum of time residuals squared
      float tmpResSq = 0;

      for (auto step = 0.; begin != end; ++begin, step += m_apvClockPeriod) {
        Atot += static_cast<double>(*begin);
        tmpResSq += TMath::Power(step - getClusterTime(), 2);
      }

      //compute average noise
      int aveNoise = 0;
      for (auto s : m_rawCluster.getStripsInCluster())
        aveNoise += 1. / s.noise / s.noise;

      aveNoise = 1. / TMath::Sqrt(aveNoise);

      return aveNoise / Atot * TMath::Sqrt(tmpResSq);

    }

  }  //SVD namespace
} //Belle2 namespace
