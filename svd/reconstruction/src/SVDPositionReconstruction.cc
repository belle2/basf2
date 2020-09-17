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
#include <svd/reconstruction/SVDPositionReconstruction.h>
#include <svd/reconstruction/SVDChargeReconstruction.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {



    double SVDPositionReconstruction::getCoGPosition()
    {

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);

      double position = 0;
      double charge = 0;

      for (auto aStrip : m_strips) {

        SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(aStrip, m_vxdID, m_isUside);
        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, aStrip.cellID);
        chargeReco->setAverageNoise(aStrip.noise, averageNoiseInElectrons);

        double stripPos = m_isUside ? info.getUCellPosition(aStrip.cellID) : info.getVCellPosition(aStrip.cellID);

        //getting the charge with the algorithm set in the DBobject
        //  double stripCharge  = chargeReco->getStripCharge("fromRecoDBObject");
        double stripCharge  = chargeReco->getStripCharge();

        position += stripPos * stripCharge;
        charge += stripCharge;
      }

      return position / charge;
    }



    double SVDPositionReconstruction::getCoGPositionError()
    {

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);
      double pitch = m_isUside ? info.getUPitch() : info.getVPitch();

      double positionError = 0;

      /*
      SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(aStrip, m_vxdID, m_isUside);
      float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, aStrip.cellID);
      chargeReco->setAverageNoise(aStrip.noise, averageNoiseInElectrons);

      //if cluster size > 1
      double a = 3;
      double sn = m_charge / m_strips.at(0).noise;
      m_positionError = a * pitch / sn;


      if (m_strips.size() == 1) {
      // Add a strip charge equal to the zero-suppression threshold to compute the error
      double phantomCharge = m_cutAdjacent * m_strips.at(0).noise;
      positionError = pitch * phantomCharge / (m_charge + phantomCharge);
        }
      */
      return positionError;

    }

  }  //SVD namespace
} //Belle2 namespace
