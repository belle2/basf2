/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SVDClusterTime.h>
#include <svd/reconstruction/SVDMaxSumAlgorithm.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <TMath.h>
#include <numeric>

namespace Belle2 {

  namespace SVD {

    void SVDClusterTime::applyCoG6Time(const Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame)
    {

      // ISSUES:
      // 1. time error not computed

      //the first frame is 0 by definition
      firstFrame = 0;

      //take the strips in the rawCluster
      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      //initialize time, stripTime and sumAmplitudes
      time = 0;
      timeError = 0;
      float sumAmplitudes = 0;

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        double stripTime = 0;
        float stripSumAmplitudes = 0;

        for (int k = 0; k < 6; k ++) {
          stripTime += k * strip.samples[k];
          stripSumAmplitudes += strip.samples[k];
        }
        if (stripSumAmplitudes != 0) {
          stripTime /= (stripSumAmplitudes);
          stripTime *= m_apvClockPeriod;
        } else {
          stripTime = -1;
          B2WARNING("Trying to divide by 0 (ZERO)! Sum of amplitudes is nullptr! Skipping this SVDShaperDigit!");
        }

        // correct strip by the CalPeak
        stripTime -= m_PulseShapeCal.getPeakTime(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID);

        if (std::isnan(float(m_triggerBin)))
          B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

        if (! m_returnRawClusterTime)
          // calibrate strip time (cellID not used)
          stripTime =  m_CoG6TimeCal.getCorrectedTime(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID, stripTime, m_triggerBin);

        //update cluster time
        time += stripTime * strip.maxSample;
        sumAmplitudes += strip.maxSample;
      }

      //finally compute cluster time
      time = time / sumAmplitudes;
    }

    void SVDClusterTime::applyCoG3Time(const Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame)
    {
      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(rawCluster.getClsSamples(false));

      std::vector<float> selectedSamples = maxSum.getSelectedSamples();
      firstFrame = maxSum.getFirstFrame();


      auto begin = selectedSamples.begin();
      const auto end = selectedSamples.end();

      auto retval = 0., norm = 0.;
      for (auto step = 0.; begin != end; ++begin, step += m_apvClockPeriod) {
        norm += static_cast<double>(*begin);
        retval += static_cast<double>(*begin) * step;
      }
      float rawtime = retval / norm;

      if (std::isnan(float(m_triggerBin)))
        B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

      if (m_returnRawClusterTime)
        time = rawtime;
      else
        //cellID = 10 not used for calibration
        time = m_CoG3TimeCal.getCorrectedTime(rawCluster.getSensorID(), rawCluster.isUSide(), 10, rawtime, m_triggerBin);



      // now compute the CoG3 time error
      // assumptions:
      // 1. calibration function parameters error not taken into account
      // 2. 100% correlation among different strip noises
      // 3. error on the sample amplitude = strip noise for all samples

      //compute the noise of the clustered sample
      //it is the same for all samples
      //computed assuming 2. (-> linear sum, not quadratic)
      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();
      float noise = std::accumulate(strips.begin(), strips.end(), 0., [](float sum, const Belle2::SVD::StripInRawCluster & strip) { return sum + strip.noise; });

      //compute the noise of the raw time
      //assuming only the clustered sample amplitude carries an uncertainty
      double rawtimeError = 0;
      begin = selectedSamples.begin();
      for (float i = 0.; begin != end; ++begin, i += 1)
        rawtimeError += TMath::Power((m_apvClockPeriod * i - rawtime) / norm, 2);
      rawtimeError = sqrt(rawtimeError) * noise;

      //compute the error on the calibrated time
      timeError = m_CoG3TimeCal.getCorrectedTimeError(rawCluster.getSensorID(), rawCluster.isUSide(), 10, rawtime, rawtimeError,
                                                      m_triggerBin);
    }


    void SVDClusterTime::applyELS3Time(const Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame)
    {

      // ISSUES:
      // 1. ELS3 tau hardcoded
      // 2. time error not computer

      timeError = 0;
      float m_ELS3tau = 55;

      //take the MaxSum 3 samples
      SVDMaxSumAlgorithm maxSum = SVDMaxSumAlgorithm(rawCluster.getClsSamples(false));

      std::vector<float> selectedSamples = maxSum.getSelectedSamples();
      firstFrame = maxSum.getFirstFrame();

      auto begin = selectedSamples.begin();
      const double E = std::exp(- m_apvClockPeriod / m_ELS3tau);
      const double E2 = E * E;
      const double E4 = E2 * E2;
      double a0 = (*begin);
      double a1 = (*(begin + 1));
      double a2 = (*(begin + 2));

      //compute raw time
      const double w = (a0 -  E2 * a2) / (2 * a0 + E * a1);
      auto rawtime_num  = 2 * E4 + w * E2;
      auto rawtime_den =  1 - E4 - w * (2 + E2);
      double rawtime = - m_apvClockPeriod * rawtime_num / rawtime_den;

      if (std::isnan(float(m_triggerBin)))
        B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

      if (m_returnRawClusterTime)
        time = rawtime;
      else
        time = m_ELS3TimeCal.getCorrectedTime(rawCluster.getSensorID(), rawCluster.isUSide(), 10, rawtime, m_triggerBin);

    }

  }  //SVD namespace
} //Belle2 namespace
