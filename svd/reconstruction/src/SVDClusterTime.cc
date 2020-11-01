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

    void SVDClusterTime::applyCoG6Time(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame)
    {
      //NOTE: timeError not computed!

      //the first frame is 0 by definition
      firstFrame = 0;

      //take APV clock
      double apvClockPeriod = 1 / m_hwClock->getClockFrequency(Const::EDetector::SVD, "sampling");

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
          stripTime *= apvClockPeriod;
        } else {
          stripTime = -1;
          B2WARNING("Trying to divide by 0 (ZERO)! Sum of amplitudes is nullptr! Skipping this SVDShaperDigit!");
        }

        // correct strip by the CalPeak
        stripTime -= m_PulseShapeCal.getPeakTime(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID);

        // calibrate strip time (cellID not used)
        stripTime =  m_CoG6TimeCal.getCorrectedTime(rawCluster.getSensorID(), rawCluster.isUSide(), strip.cellID, stripTime, m_triggerBin);

        //set the strip time in the raw cluster
        rawCluster.setStripTime(i, stripTime);

        //update cluster time
        time += stripTime * strip.maxSample;
        sumAmplitudes += strip.maxSample;
      }

      //finally compute cluster time
      time = time / sumAmplitudes;
    }

    void SVDClusterTime::applyCoG3Time(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame)
    {
    }


    void SVDClusterTime::applyELS3Time(Belle2::SVD::RawCluster& rawCluster, double& time, double& timeError, int& firstFrame)
    {
    }

  }  //SVD namespace
} //Belle2 namespace
