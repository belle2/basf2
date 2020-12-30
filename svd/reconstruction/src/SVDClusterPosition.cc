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
#include <svd/reconstruction/SVDClusterPosition.h>
#include <svd/reconstruction/SVDCoG3Time.h>
#include <svd/reconstruction/SVDELS3Time.h>
#include <svd/reconstruction/SVDELS3Charge.h>
#include <svd/reconstruction/SVDMaxSampleCharge.h>
#include <svd/reconstruction/SVDSumSamplesCharge.h>
#include <svd/reconstruction/SVDMaxSumAlgorithm.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

#include <TMath.h>

using namespace std;

namespace Belle2 {

  namespace SVD {


    void SVDClusterPosition::applyCoGPosition(const Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError)
    {
      //NOTE: position error NOT computed!
      positionError = 0;

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(rawCluster.getSensorID());

      position = 0;
      double charge = 0;

      //take the strips in the rawCluster
      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      for (auto aStrip : strips) {

        double stripPos = rawCluster.isUSide() ? info.getUCellPosition(aStrip.cellID) : info.getVCellPosition(aStrip.cellID);

        //getting the charge of the strip
        double stripCharge  = aStrip.charge;

        position += stripPos * stripCharge;
        charge += stripCharge;
      }

      position /= charge;
    }


    void SVDClusterPosition::applyAHTPosition(const Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError)
    {

      //NOTE: position error NOT computed!
      positionError = 0;

      // NOTE:
      // tail and head are the two strips at the edge of the cluster

      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(rawCluster.getSensorID());
      double pitch = rawCluster.isUSide() ? info.getUPitch() : info.getVPitch();

      //take the strips in the rawCluster
      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      //informations about the head strip
      int headStripCellID = strips.at(strips.size() - 1).cellID;
      double headStripCharge = strips.at(strips.size() - 1).charge;
      //informations about the tail strip
      int tailStripCellID = strips.at(0).cellID;
      double tailStripCharge = strips.at(0).charge;

      // average strip charge of the center of the cluster

      double centreCharge = (getSumOfStripCharges(rawCluster) - tailStripCharge - headStripCharge) / (strips.size() - 2);

      tailStripCharge = (tailStripCharge < centreCharge) ? tailStripCharge : centreCharge;
      headStripCharge = (headStripCharge < centreCharge) ? headStripCharge : centreCharge;
      double tailPos = rawCluster.isUSide() ? info.getUCellPosition(tailStripCellID) : info.getVCellPosition(tailStripCellID);
      double headPos = rawCluster.isUSide() ? info.getUCellPosition(headStripCellID) : info.getVCellPosition(headStripCellID);
      position = 0.5 * (tailPos + headPos + (headStripCharge - tailStripCharge) / centreCharge * pitch);

    }

    double SVDClusterPosition::getSumOfStripCharges(const Belle2::SVD::RawCluster& rawCluster)
    {

      double sumStripCharge = 0;

      //take the strips in the rawCluster
      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      // compute the sum of strip charges
      for (auto aStrip : strips) {

        double stripCharge  = aStrip.charge;
        sumStripCharge += stripCharge;
      }
      return sumStripCharge;
    }

    void SVDClusterPosition::reconstructStrips(Belle2::SVD::RawCluster& rawCluster)
    {


      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      //loop on strips
      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        RawCluster tmp(rawCluster.getSensorID(), rawCluster.isUSide(), 0, 0);
        if (tmp.add(rawCluster.getSensorID(), rawCluster.isUSide(), strip)) {
          double time = 0;

          double timeError = 0;
          int firstFrame = 0;

          // time computation
          // this section is commented because not used in position reconstruction:
          /*    if(m_stripTimeAlgo == "ELS3"){
            SVDELS3Time ct;
            ct.computeClusterTime(tmp, time, timeError, firstFrame);

          } else if(m_stripTimeAlgo == "CoG3"){
            SVDCoG3Time ct;
            ct.computeClusterTime(tmp, time, timeError, firstFrame);
          }
          rawCluster.setStripTime(i, time);
          */

          //charge computation
          double charge = 0;
          double SNR;
          double seedCharge;
          if (m_stripChargeAlgo == "ELS3") {
            SVDELS3Charge cc;
            cc.computeClusterCharge(tmp, charge, SNR, seedCharge);
          } else if (m_stripChargeAlgo == "MaxSample") {
            SVDMaxSampleCharge cc;
            cc.computeClusterCharge(tmp, charge, SNR, seedCharge);
          } else if (m_stripChargeAlgo == "SumSamples") {
            SVDSumSamplesCharge cc;
            cc.computeClusterCharge(tmp, charge, SNR, seedCharge);
          }

          double CHARGE = 0;
          SVDMaxSampleCharge cc;
          cc.computeClusterCharge(tmp, CHARGE, SNR, seedCharge);

          if ((abs(charge - CHARGE) / CHARGE > 0.3)   || charge < 0)
            rawCluster.setStripCharge(i, CHARGE);
          else
            rawCluster.setStripCharge(i, charge);

        } else
          B2ERROR("this should not happen...");


      }

    }

  }  //SVD namespace
} //Belle2 namespace
