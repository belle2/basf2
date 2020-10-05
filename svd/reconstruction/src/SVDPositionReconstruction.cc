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


      double sumStripCharge = 0;

      //first compute the sum of strip charges
      for (auto aStrip : m_strips) {

        SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(aStrip, m_vxdID, m_isUside);
        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, aStrip.cellID);
        chargeReco->setAverageNoise(aStrip.noise, averageNoiseInElectrons);

        double stripCharge  = chargeReco->getStripCharge();
        sumStripCharge += stripCharge;
      }

      float noiseFirstStrip =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, m_strips.at(0).cellID);
      float noiseLastStrip =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, m_strips.at(m_strips.size() - 1).cellID);
      //      float noiseAverage = (noiseFirstStrip + noiseLastStrip) /2;
      float noiseAverage = noiseFirstStrip;

      double cutAdjacent = m_ClusterCal.getMinAdjSNR(m_vxdID, m_isUside);

      // if cluster size == 1
      // add a strip charge equal to the zero-suppression threshold to compute the error
      if (m_strips.size() == 1) {
        double phantomCharge = cutAdjacent * noiseFirstStrip;
        positionError = pitch * phantomCharge / (sumStripCharge + phantomCharge);
        //  B2INFO("NEW size = 1, pitch = "<<pitch<<" cutAdjacent = "<<cutAdjacent<<", noiseFirstStrip = "<<noiseFirstStrip<<" ERROR  = "<<positionError);
        return positionError;
      }

      // if cluster size > 1
      double a = cutAdjacent;
      double sn = sumStripCharge / noiseAverage;
      positionError = a * pitch / sn;
      return positionError;

    }

    double SVDPositionReconstruction::getCoGPositionErrorPropagation()
    {
      // just do error propagation of the weighted average with:
      // assume error on strip position  = pitch / sqrt(12)
      // assume error on strip charge = noise
      // strip position in cm
      // strip charge in electrons

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);
      double pitch = m_isUside ? info.getUPitch() : info.getVPitch();

      if (m_strips.size() == 1)
        return pitch / sqrt(12);

      double positionError = 0;
      double sumStripCharge = 0;

      //first compute the sum of strip charges
      for (auto aStrip : m_strips) {

        SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(aStrip, m_vxdID, m_isUside);
        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, aStrip.cellID);
        chargeReco->setAverageNoise(aStrip.noise, averageNoiseInElectrons);

        double stripCharge  = chargeReco->getStripCharge();
        sumStripCharge += stripCharge;
      }

      // error propagation formula:
      for (auto aStrip : m_strips) {

        SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(aStrip, m_vxdID, m_isUside);
        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, aStrip.cellID);
        chargeReco->setAverageNoise(aStrip.noise, averageNoiseInElectrons);

        double stripPos = m_isUside ? info.getUCellPosition(aStrip.cellID) : info.getVCellPosition(aStrip.cellID);

        double stripCharge  = chargeReco->getStripCharge();

        double first = stripCharge / sumStripCharge * pitch / sqrt(12);
        B2INFO("first = " << stripCharge << "/" << sumStripCharge << "*" << pitch << "/" << sqrt(12) << " = " << first);
        double second = (stripPos - getCoGPosition()) / sumStripCharge * averageNoiseInElectrons;
        B2INFO("second = " << "(" << stripPos << " - " << getCoGPosition() << ")/" << sumStripCharge << "*" << averageNoiseInElectrons <<
               " = " << second);
        positionError = TMath::Power(first, 2) + TMath::Power(second, 2);

      }

      return sqrt(positionError);

    }

  }  //SVD namespace
} //Belle2 namespace
