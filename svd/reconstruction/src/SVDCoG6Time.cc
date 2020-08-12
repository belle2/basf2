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
#include <svd/reconstruction/SVDCoG6Time.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDCoG6Time::getStripTime(Belle2::SVDShaperDigit::APVFloatSamples samples)
    {

      //calculate weighted average time

      float averagetime = 0;
      float sumAmplitudes = 0;
      for (int k = 0; k < 6; k ++) {
        averagetime += k * samples[k];
        sumAmplitudes += samples[k];
      }
      if (sumAmplitudes != 0) {
        averagetime /= (sumAmplitudes);
        averagetime *= m_apvClockPeriod;
      } else {
        averagetime = -1;
        B2WARNING("Trying to divide by 0 (ZERO)! Sum of amplitudes is nullptr! Skipping this SVDShaperDigit!");
      }

      return averagetime;
    }

    double SVDCoG6Time::getStripTimeError(Belle2::SVDShaperDigit::APVFloatSamples samples, int noise)
    {

      //assuming that:
      // 1. noise of the samples are totally UNcorrelated (correct in MC)
      // 2. error on sampling time is negligible

      //sum of samples amplitudes
      float Atot = 0;
      //sum of time residuals squared
      float tmpResSq = 0;

      for (int k = 0; k < 6; k ++) {
        Atot  += samples[k];
        tmpResSq += TMath::Power(k * m_apvClockPeriod - getStripTime(samples), 2);
      }

      return noise / Atot * TMath::Sqrt(tmpResSq);

    }


    double SVDCoG6Time::getClusterTime()
    {

      //as weighted average of the strip time with strip max sample

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double time = 0;
      double sumAmplitudes = 0;

      for (auto s : strips) {
        time += s.maxSample * getStripTime(s.samples);
        sumAmplitudes += s.maxSample;
      }

      return time / sumAmplitudes;
    }

    double SVDCoG6Time::getClusterTimeError()
    {

      //as error on weighted avearage, neglecting error on weights

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double weightSum = 0;

      for (auto s : strips)
        weightSum += getStripTimeError(s.samples, s.noise) * getStripTimeError(s.samples, s.noise);

      return 1. / TMath::Sqrt(weightSum);
    }

  }  //SVD namespace
} //Belle2 namespace
