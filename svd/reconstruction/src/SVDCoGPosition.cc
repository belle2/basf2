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
#include <svd/reconstruction/SVDCoGPosition.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDCoGPosition::getClusterPosition()
    {

      //as weighted average of the strip position with strip max sample
      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double position = 0;
      double sumAmplitudes = 0;

      for (auto s : strips) {
        position += s.maxSample * getStripPosition(s.cellID);
        sumAmplitudes += s.maxSample;
      }

      return position / sumAmplitudes;

    }

    double SVDCoGPosition::getClusterPositionError()
    {
      double positionError = 0;

      return positionError;

    }

    double SVDCoGPosition::getClusterPositionErrorSize1()
    {
      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);
      double pitch = m_isUside ? info.getUPitch() : info.getVPitch();

      std::vector<Belle2::SVD::stripInRawCluster> strips = m_rawCluster.getStripsInRawCluster();

      double phantomCharge = m_cutAdjacent * strips.at(0).noise;
      double positionError = pitch * phantomCharge / (strips.at(0).maxSample + phantomCharge);

      return positionError;

    }

  }  //SVD namespace
} //Belle2 namespace
