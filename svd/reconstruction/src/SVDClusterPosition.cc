/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <Eigen/Dense>

using namespace std;

namespace Belle2 {

  namespace SVD {


    void SVDClusterPosition::applyCoGPosition(const Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError)
    {

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

      //now compute position error
      positionError = 0;
      double pitch = rawCluster.isUSide() ? info.getUPitch() : info.getVPitch();
      double sumStripCharge = getSumOfStripCharges(rawCluster);

      positionError = m_CoGOnlyErr.getPositionError(rawCluster.getSensorID(), rawCluster.isUSide(), 0,
                                                    sumStripCharge / getClusterNoise(rawCluster), rawCluster.getSize(), pitch);

    }



    void SVDClusterPosition::applyAHTPosition(const Belle2::SVD::RawCluster& rawCluster, double& position, double& positionError)
    {

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

      //now compute position error
      double cutAdjacent = m_ClusterCal.getMinAdjSNR(rawCluster.getSensorID(), rawCluster.isUSide());
      double sn = centreCharge / cutAdjacent / getClusterNoise(rawCluster);

      // Rough estimates of Landau noise
      double landauHead = tailStripCharge / centreCharge;
      double landauTail = headStripCharge / centreCharge;
      positionError = 0.5 * pitch * sqrt(1.0 / sn / sn +
                                         0.5 * landauHead * landauHead +
                                         0.5 * landauTail * landauTail);

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

    double SVDClusterPosition::getClusterNoise(const Belle2::SVD::RawCluster& rawCluster)
    {

      double clusterNoise = 0;

      //take the strips in the rawCluster
      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      // compute the cluster noise as sum in quadrature of strip noise
      for (auto aStrip : strips) {

        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(rawCluster.getSensorID(), rawCluster.isUSide(), aStrip.cellID);
        clusterNoise += averageNoiseInElectrons * averageNoiseInElectrons;
      }
      return sqrt(clusterNoise);
    }

    double SVDClusterPosition::getAverageStripNoise(const Belle2::SVD::RawCluster& rawCluster)
    {

      double averageNoise = 0;

      //take the strips in the rawCluster
      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      // compute the average strip noise
      for (auto aStrip : strips) {

        float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(rawCluster.getSensorID(), rawCluster.isUSide(), aStrip.cellID);
        averageNoise += averageNoiseInElectrons;
      }
      return averageNoise / strips.size();
    }

    void SVDClusterPosition::reconstructStrips(Belle2::SVD::RawCluster& rawCluster)
    {


      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

      //loop on strips
      for (int i = 0; i < (int)strips.size(); i++) {

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        RawCluster tmp(rawCluster.getSensorID(), rawCluster.isUSide(), 0, 0);
        if (tmp.add(rawCluster.getSensorID(), rawCluster.isUSide(), strip)) {

          // time computation
          if (m_stripTimeAlgo.compare("dontdo") != 0) {

            double time = 0;
            double timeError = 0;
            int firstFrame = 0;

            if (m_stripTimeAlgo == "ELS3") {
              SVDELS3Time ct;
              ct.computeClusterTime(tmp, time, timeError, firstFrame);

            } else if (m_stripTimeAlgo == "CoG3") {
              SVDCoG3Time ct;
              ct.computeClusterTime(tmp, time, timeError, firstFrame);
            }
            rawCluster.setStripTime(i, time);
          }

          // charge computation
          // may be not needed in case the cluster position is computed using APV samples
          // and not using reconstructed strips
          if (m_stripChargeAlgo.compare("dontdo") != 0) {
            double charge = 0;
            double SNR;
            double seedCharge;

            if (m_stripChargeAlgo == "ELS3") {
              // ELS3 can return non-sense values for off-time or low-charge strips)
              // if returned charge is negative or more than 30% different than MaxSample, we use MaxSample
              // without notice to the user!

              SVDELS3Charge cc;
              cc.computeClusterCharge(tmp, charge, SNR, seedCharge);

              double maxSample_charge = 0;
              SVDMaxSampleCharge maxSample_cc;
              maxSample_cc.computeClusterCharge(tmp, maxSample_charge, SNR, seedCharge);

              if ((abs(charge - maxSample_charge) / maxSample_charge > 0.3) || charge < 0)
                rawCluster.setStripCharge(i, maxSample_charge);
              else
                rawCluster.setStripCharge(i, charge);

            } else if (m_stripChargeAlgo == "SumSamples") {
              SVDSumSamplesCharge cc;
              cc.computeClusterCharge(tmp, charge, SNR, seedCharge);
              rawCluster.setStripCharge(i, charge);
            } else  {
              // MaxSample is used when the algorithm is not recognized
              SVDMaxSampleCharge cc;
              cc.computeClusterCharge(tmp, charge, SNR, seedCharge);
              rawCluster.setStripCharge(i, charge);
            }
          }
        } else
          B2ERROR("this should not happen...");

      }

    }

    void SVDClusterPosition::applyUnfolding(Belle2::SVD::RawCluster& rawCluster)
    {


      std::vector<Belle2::SVD::StripInRawCluster> strips = rawCluster.getStripsInRawCluster();
      double unfoldingCoefficient = m_ClusterCal.getUnfoldingCoeff(rawCluster.getSensorID(), rawCluster.isUSide());
      unsigned int Size = strips.size();
      double threshold = 0;
      Eigen::VectorXd Charges(Size);
      Eigen::MatrixXd Couplings(Size, Size);
      // Unfolding Matrix
      for (unsigned int i = 0; i < Size; ++i) {
        for (unsigned int j = 0; j < Size; ++j) {
          if (i == j) {Couplings(i, j) = 1 - 2 * unfoldingCoefficient;}
          else if (j == i + 1) {Couplings(i, j) = unfoldingCoefficient;}
          else if (j == i - 1) {Couplings(i, j) = unfoldingCoefficient;}
          else {Couplings(i, j) = 0;}
        }

        Belle2::SVD::StripInRawCluster strip = strips.at(i);

        RawCluster tmp(rawCluster.getSensorID(), rawCluster.isUSide(), 0, 0);
        if (tmp.add(rawCluster.getSensorID(), rawCluster.isUSide(), strip)) {
          //Fill a vector with strip charges
          Charges(i) = strip.charge;
        }

      }
      //Apply the unfolding
      Charges = Couplings.inverse() * Charges;
      for (unsigned i = 0; i < Size; i++) {
        if (Charges(i) < threshold) {Charges(i) = 0;}
        rawCluster.setStripCharge(i, Charges(i));
      }

    }

  }  //SVD namespace
} //Belle2 namespace
