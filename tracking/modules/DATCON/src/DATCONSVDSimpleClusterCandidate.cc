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
#include <tracking/modules/DATCON/DATCONSVDSimpleClusterCandidate.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

using namespace std;

namespace Belle2 {

  DATCONSVDSimpleClusterCandidate::DATCONSVDSimpleClusterCandidate(VxdID vxdID, bool isUside)
    : m_vxdID(vxdID)
    , m_isUside(isUside)
    , m_charge(0)
    , m_seedCharge(0)
    , m_position(0)
    , m_seedStrip(0)
    , m_size(0) {};

  DATCONSVDSimpleClusterCandidate::DATCONSVDSimpleClusterCandidate(VxdID vxdID, bool isUside, unsigned short maxClusterSize)
    : m_vxdID(vxdID)
    , m_isUside(isUside)
    , m_maxClusterSize(maxClusterSize)
    , m_charge(0)
    , m_seedCharge(0)
    , m_position(0)
    , m_seedStrip(0)
    , m_size(0) {};

  bool DATCONSVDSimpleClusterCandidate::add(VxdID vxdID, bool isUside, unsigned short index, unsigned short charge,
                                            unsigned short cellID)
  {

    bool added = false;

    // do not add if you are on the wrong sensor or side
    if ((m_vxdID != vxdID) || (m_isUside != isUside))
      return false;

    // add if it's the first strip
    if (m_size == 0)
      added = true;

    /**add if it adjacent to the last strip added
     * (we assume that SVDRecoDigits are ordered)
     * and if cluster size would still be <= 4 strips
     */
    if (m_size > 0 && cellID == m_strips.at(m_strips.size() - 1) + 1
        && cellID < m_strips.at(0) + m_maxClusterSize)
      added  = true;

    //add it to the vector od strips, update the seed charge and index:
    if (added) {
      m_size++;
      m_strips.push_back(cellID);
      m_charges.push_back(charge);
      m_digitIndices.push_back(index);

      if (charge > m_seedCharge) {
        m_seedCharge = charge;
//         m_seedIndex = m_strips.size() - 1;
      }
    }
    return added;

  };

  void DATCONSVDSimpleClusterCandidate::finalizeCluster()
  {

    B2ERROR("This mode is currently not supported, nothing will happen, no DATCONSVDClusters will be created!");
    return;

  };


  void DATCONSVDSimpleClusterCandidate::finalizeSimpleCluster()
  {

    const VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    const VXD::SensorInfoBase& info = geo.getSensorInfo(m_vxdID);

    double pitch = m_isUside ? info.getUPitch() : info.getVPitch();
    unsigned short numberofStrips = m_isUside ? info.getUCells() : info.getVCells();

    unsigned short minStripCellID = m_strips.at(0);

    unsigned short clusterSize    = m_strips.size();
    unsigned short seedStripIndex = clusterSize / 2 + 1;
//     unsigned short seedStrip      = (minStripCellID + clusterSize / 2);
    unsigned short seedStrip      = (minStripCellID + seedStripIndex - 1);
    double clusterPosition        = pitch * (seedStrip - numberofStrips / 2);

    m_size = clusterSize;
    m_seedStrip = seedStrip;
    m_seedStripIndex = seedStripIndex;
    m_seedCharge = m_charges.at(m_seedStripIndex - 1);
    m_position = clusterPosition;
  };


  bool DATCONSVDSimpleClusterCandidate::isGoodCluster()
  {

    bool isGood = false;

    if (m_seedCharge > 0)
      isGood = true;

    return isGood;
  };

} //Belle2 namespace
