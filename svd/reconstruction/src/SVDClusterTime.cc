/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SVDClusterTime.h>
#include <svd/reconstruction/SVDMaxSumAlgorithm.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <TMath.h>

using namespace std;

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

        if (isnan(m_triggerBin))
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

      // ISSUES:
      // 1. time error not computer

      timeError = 0;

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

      if (isnan(m_triggerBin))
        B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

      if (m_returnRawClusterTime)
        time = rawtime;
      else
        //cellID not used for calibration
        time = m_CoG3TimeCal.getCorrectedTime(rawCluster.getSensorID(), rawCluster.isUSide(), 10, rawtime, m_triggerBin);

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

      if (isnan(m_triggerBin))
        B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

      if (m_returnRawClusterTime)
        time = rawtime;
      else
        time = m_ELS3TimeCal.getCorrectedTime(rawCluster.getSensorID(), rawCluster.isUSide(), 10, rawtime, m_triggerBin);

    }

  }  //SVD namespace
} //Belle2 namespace
