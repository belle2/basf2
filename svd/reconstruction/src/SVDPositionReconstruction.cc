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


      double sumStripCharge = getSumOfStripCharges();

      float noiseFirstStrip =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, m_strips.at(0).cellID);
      //      float noiseLastStrip =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, m_strips.at(m_strips.size() - 1).cellID);
      //      float noiseAverage = (noiseFirstStrip + noiseLastStrip) /2;
      float noiseAverage = noiseFirstStrip;

      double cutAdjacent = m_ClusterCal.getMinAdjSNR(m_vxdID, m_isUside);

      // if cluster size == 1
      // add a strip charge equal to the zero-suppression threshold to compute the error
      if (m_strips.size() == 1) {
        double phantomCharge = cutAdjacent * noiseFirstStrip;
        positionError = pitch * phantomCharge / (sumStripCharge + phantomCharge);

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
      double sumStripCharge = getSumOfStripCharges();

      // error propagation formula:
      for (auto aStrip : m_strips) {

        SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(aStrip, m_vxdID, m_isUside);
        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, aStrip.cellID);
        chargeReco->setAverageNoise(aStrip.noise, averageNoiseInElectrons);

        double stripPos = m_isUside ? info.getUCellPosition(aStrip.cellID) : info.getVCellPosition(aStrip.cellID);

        double stripCharge  = chargeReco->getStripCharge();

        double first = stripCharge / sumStripCharge * pitch / sqrt(12);
        //        B2INFO("first = " << stripCharge << "/" << sumStripCharge << "*" << pitch << "/" << sqrt(12) << " = " << first);
        double second = (stripPos - getCoGPosition()) / sumStripCharge * averageNoiseInElectrons;
        //        B2INFO("second = " << "(" << stripPos << " - " << getCoGPosition() << ")/" << sumStripCharge << "*" << averageNoiseInElectrons <<
        " = " << second);
        positionError = TMath::Power(first, 2) + TMath::Power(second, 2);

      }

      return sqrt(positionError);

    }


    double SVDPositionReconstruction::getAHTPosition()
    {

      // NOTE:
      // tail and head are the two strips at the edge of the cluster

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);
      double pitch = m_isUside ? info.getUPitch() : info.getVPitch();

      //informations about the head strip
      int headStripCellID = m_strips.at(m_strips.size() - 1).cellID;
      SVDChargeReconstruction* chargeRecoHead = new SVDChargeReconstruction(m_strips.at(m_strips.size() - 1), m_vxdID, m_isUside);
      float averageNoiseInElectronsHead =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, headStripCellID);
      chargeRecoHead->setAverageNoise(m_strips.at(m_strips.size() - 1).noise, averageNoiseInElectronsHead);

      double headStripCharge = chargeRecoHead->getStripCharge();
      //informations about the tail strip
      int tailStripCellID = m_strips.at(0).cellID;
      SVDChargeReconstruction* chargeRecoTail = new SVDChargeReconstruction(m_strips.at(0), m_vxdID, m_isUside);
      float averageNoiseInElectronsTail =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, tailStripCellID);
      chargeRecoTail->setAverageNoise(m_strips.at(0).noise, averageNoiseInElectronsTail);

      double tailStripCharge = chargeRecoTail->getStripCharge();


      // average strip charge of the center of the cluster

      double centreCharge = (getSumOfStripCharges() - tailStripCharge - headStripCharge) / (m_strips.size() - 2);

      tailStripCharge = (tailStripCharge < centreCharge) ? tailStripCharge : centreCharge;
      headStripCharge = (headStripCharge < centreCharge) ? headStripCharge : centreCharge;
      double tailPos = m_isUside ? info.getUCellPosition(tailStripCellID) : info.getVCellPosition(tailStripCellID);
      double headPos = m_isUside ? info.getUCellPosition(headStripCellID) : info.getVCellPosition(headStripCellID);
      double position = 0.5 * (tailPos + headPos + (headStripCharge - tailStripCharge) / centreCharge * pitch);

      return position;

    }

    double SVDPositionReconstruction::getAHTPositionError()
    {

      // NOTE:
      // tail and head are the two strips at the edge of the cluster

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);
      double pitch = m_isUside ? info.getUPitch() : info.getVPitch();

      //informations about the head strip
      int headStripCellID = m_strips.at(m_strips.size() - 1).cellID;
      SVDChargeReconstruction* chargeRecoHead = new SVDChargeReconstruction(m_strips.at(m_strips.size() - 1), m_vxdID, m_isUside);
      float averageNoiseInElectronsHead =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, headStripCellID);
      chargeRecoHead->setAverageNoise(m_strips.at(m_strips.size() - 1).noise, averageNoiseInElectronsHead);

      double headStripCharge = chargeRecoHead->getStripCharge();
      //informations about the tail strip
      int tailStripCellID = m_strips.at(0).cellID;
      SVDChargeReconstruction* chargeRecoTail = new SVDChargeReconstruction(m_strips.at(0), m_vxdID, m_isUside);
      float averageNoiseInElectronsTail =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, tailStripCellID);
      chargeRecoTail->setAverageNoise(m_strips.at(0).noise, averageNoiseInElectronsTail);

      double tailStripCharge = chargeRecoTail->getStripCharge();

      // average strip charge of the center of the cluster

      double centreCharge = (getSumOfStripCharges() - tailStripCharge - headStripCharge) / (m_strips.size() - 2);

      tailStripCharge = (tailStripCharge < centreCharge) ? tailStripCharge : centreCharge;
      headStripCharge = (headStripCharge < centreCharge) ? headStripCharge : centreCharge;

      double cutAdjacent = m_ClusterCal.getMinAdjSNR(m_vxdID, m_isUside);
      double sn = centreCharge / cutAdjacent / getClusterNoise();

      // Rough estimates of Landau noise
      double landauHead = tailStripCharge / centreCharge;
      double landauTail = headStripCharge / centreCharge;
      double positionError = 0.5 * pitch * sqrt(1.0 / sn / sn +
                                                0.5 * landauHead * landauHead +
                                                0.5 * landauTail * landauTail);

      return positionError;

    }

    double SVDPositionReconstruction::getSumOfStripCharges()
    {

      double sumStripCharge = 0;

      // compute the sum of strip charges
      for (auto aStrip : m_strips) {

        SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(aStrip, m_vxdID, m_isUside);
        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, aStrip.cellID);
        chargeReco->setAverageNoise(aStrip.noise, averageNoiseInElectrons);

        double stripCharge  = chargeReco->getStripCharge();
        sumStripCharge += stripCharge;
      }
      return sumStripCharge;
    }

    double SVDPositionReconstruction::getClusterNoise()
    {

      double clusterNoise = 0;

      // compute the sum of strip charges
      for (auto aStrip : m_strips) {

        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(m_vxdID, m_isUside, aStrip.cellID);
        clusterNoise += averageNoiseInElectrons * averageNoiseInElectrons;
      }
      return sqrt(clusterNoise);
    }

  }  //SVD namespace
} //Belle2 namespace
