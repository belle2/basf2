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
#include <svd/reconstruction/SVDTimeReconstruction.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    std::pair<int, double> SVDCoG3Time::getFirstFrameAndClusterTime(const Belle2::SVD::RawCluster& rawCluster)
    {

      bool inElectrons = false;

      SVDTimeReconstruction timeReco(rawCluster.getClsSamples(inElectrons),
                                     rawCluster.getSensorID(), rawCluster.isUSide());

      timeReco.setTriggerBin(m_triggerBin);

      return timeReco.getCoG3FirstFrameAndTime();

    }

    double SVDCoG3Time::getClusterTimeError(const Belle2::SVD::RawCluster& rawCluster)
    {


      bool inElectrons = false;

      SVDTimeReconstruction timeReco(rawCluster.getClsSamples(inElectrons),
                                     rawCluster.getSensorID(), rawCluster.isUSide());

      timeReco.setTriggerBin(m_triggerBin);

      double timeError = timeReco.getCoG3TimeError();

      return  timeError;

    }

  }  //SVD namespace
} //Belle2 namespace
