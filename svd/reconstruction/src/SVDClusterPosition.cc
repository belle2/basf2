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
#include <svd/reconstruction/SVDClusterPosition.h>
#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    double SVDClusterPosition::getStripPosition(int cellID)
    {

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);

      double position = 0;
      position = m_isUside ? info.getUCellPosition(cellID) : info.getVCellPosition(cellID);

      return position;
    }

    double SVDClusterPosition::getStripPositionError()
    {

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);

      double pitch = m_isUside ? info.getUPitch() : info.getVPitch();

      double positionError = pitch / std::sqrt(12);

      return positionError;

    }

  }  //SVD namespace
} //Belle2 namespace
