/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <svd/reconstruction/SimpleClusterCandidate.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

using namespace std;

namespace Belle2 {

  namespace SVD {

    SimpleClusterCandidate::SimpleClusterCandidate(VxdID vxdID, bool isUside, int sizeHeadTail, double cutSeed, double cutAdjacent)
      : m_vxdID(vxdID)
      , m_isUside(isUside)
      , m_sizeHeadTail(sizeHeadTail)
      , m_cutSeed(cutSeed)
      , m_cutAdjacent(cutAdjacent)
      , m_charge(0)
      , m_chargeError(0)
      , m_seedCharge(0)
      , m_time(0)
      , m_timeError(0)
      , m_position(0)
      , m_positionError(0)
      , m_SNR(0)
      , m_strips(4) {m_strips.clear();};

    void SimpleClusterCandidate::add(const struct  stripInCluster& aStrip)
    {

      //      B2INFO("ADDING A STRIP");
      m_strips.push_back(aStrip);

      if (aStrip.charge > m_seedCharge) {
        m_seedCharge = aStrip.charge;
        m_seedIndex = m_strips.size() - 1;
      }

    };

    void SimpleClusterCandidate::finalizeCluster()
    {

      //      B2INFO("finalizing a cluster");
      const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);

      double pitch = m_isUside ? info.getUPitch() : info.getVPitch();

      int maxStripCellID = m_strips.at(m_strips.size() - 1).cellID;
      double maxStripCharge = m_strips.at(m_strips.size() - 1).charge;
      int minStripCellID = m_strips.at(0).cellID;
      double minStripCharge = m_strips.at(0).charge;

      int clusterSize = m_strips.size();

      double noise = 0;
      for (auto aStrip : m_strips) {
        double stripPos = m_isUside ? info.getUCellPosition(aStrip.cellID) : info.getVCellPosition(aStrip.cellID);
        m_position += stripPos * aStrip.charge;
        m_charge     += aStrip.charge;
        noise += aStrip.noise * aStrip.noise;
        //  B2INFO("position = "<<stripPos<<", charge = "<<aStrip.charge);
      }
      noise = sqrt(noise);
      if (clusterSize < m_sizeHeadTail) { // COG, size = 1 or 2
        m_position /= m_charge;
        // Compute position error
        if (clusterSize == 1) {
          // Add a strip charge equal to the zero-suppression threshold to compute the error
          double phantomCharge = m_cutAdjacent * m_strips.at(0).noise;
          m_positionError = pitch * phantomCharge / (m_charge + phantomCharge);
        } else {
          double a = m_cutAdjacent;
          double sn = m_charge / m_strips.at(0).noise;
          m_positionError = a * pitch / sn;
        }
      } else { // Head-tail
        double centreCharge = (m_charge - minStripCharge - maxStripCharge) / (clusterSize - 2);
        minStripCharge = (minStripCharge < centreCharge) ? minStripCharge : centreCharge;
        maxStripCharge = (maxStripCharge < centreCharge) ? maxStripCharge : centreCharge;
        double minPos = m_isUside ? info.getUCellPosition(minStripCellID) : info.getVCellPosition(minStripCellID);
        double maxPos = m_isUside ? info.getUCellPosition(maxStripCellID) : info.getVCellPosition(maxStripCellID);
        m_position = 0.5 * (minPos + maxPos + (maxStripCharge - minStripCharge) / centreCharge * pitch);
        double sn = centreCharge / m_cutAdjacent / noise;
        // Rough estimates of Landau noise
        double landauHead = minStripCharge / centreCharge;
        double landauTail = maxStripCharge / centreCharge;
        m_positionError = 0.5 * pitch * sqrt(1.0 / sn / sn +
                                             0.5 * landauHead * landauHead +
                                             0.5 * landauTail * landauTail);
      }

      //      B2INFO("size = "<< m_strips.size()<<" charge = " << m_charge << ", time = " << 0 << ", position = " << m_position << ", SNR = " << 0 << ", time error = " <<
      //      0);




    };

    bool SimpleClusterCandidate::isGoodCluster()
    {

      bool isGood = false;

      if (m_seedCharge > 0)
        isGood = true;

      //      B2INFO("this one is good? "<<isGood);
      return isGood;
    };

    void SimpleClusterCandidate::clear()
    {

      m_charge = 0;
      m_chargeError = 0;
      m_seedCharge = 0;
      m_time = 0;
      m_timeError = 0;
      m_position = 0;
      m_positionError = 0;
      m_SNR = 0;
      m_strips.clear();
      m_strips.resize(4);
    };


  }  //SVD namespace
} //Belle2 namespace
