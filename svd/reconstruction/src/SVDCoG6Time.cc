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
#include <svd/reconstruction/SVDTimeReconstruction.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {


    std::pair<int,  double> SVDCoG6Time::getFirstFrameAndClusterTime(const Belle2::SVD::RawCluster& rawCluster)
    {

      //as weighted average of the strip time with strip max sample

      std::vector<Belle2::SVD::stripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      double time = 0;
      double sumAmplitudes = 0;

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::stripInRawCluster strip = strips.at(i);

        SVDTimeReconstruction* timeReco = new SVDTimeReconstruction(strip, rawCluster.getSensorID(), rawCluster.isUSide());
        timeReco->setTriggerBin(m_triggerBin);

        double stripTime = timeReco->getCoG6Time();

        time += strip.maxSample * stripTime;
        sumAmplitudes += strip.maxSample;
      }

      int firstFrame = 0;

      return std::make_pair(firstFrame, time / sumAmplitudes);
    }

    double SVDCoG6Time::getClusterTimeError(const Belle2::SVD::RawCluster& rawCluster)
    {

      //as error on weighted average, neglecting error on weights

      std::vector<Belle2::SVD::stripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      double variance = 0;

      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::stripInRawCluster strip = strips.at(i);

        SVDTimeReconstruction* timeReco = new SVDTimeReconstruction(strip, rawCluster.getSensorID(), rawCluster.isUSide());
        timeReco->setTriggerBin(m_triggerBin);

        double stripTimeError = timeReco->getCoG6TimeError();

        variance += stripTimeError * stripTimeError;
      }

      return 1. / TMath::Sqrt(variance);
    }

  }  //SVD namespace
} //Belle2 namespace
