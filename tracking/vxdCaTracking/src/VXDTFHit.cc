/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/VXDTFHit.h"
#include "../include/FullSecID.h"
#include <framework/gearbox/Const.h>
#include "../include/ClusterInfo.h"
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;


/** operator overload **/
bool Belle2::Tracking::VXDTFHit::operator==(const VXDTFHit& b) const
{
  B2FATAL("somebody is using the '=='-operator of VXDTFHit, although it does no valid comparison!");
  if (getVxdID() != b.getVxdID()) { return false; }   /// ensures that hits are from the same sensor

  if (this->getDetectorType() == Const::PXD) {   // PXD
    return getClusterIndexUV() == b.getClusterIndexUV();
  } else if (this->getDetectorType() == Const::SVD) {   // SVD
    return (getClusterIndexU() == b.getClusterIndexU() or getClusterIndexV() == b.getClusterIndexV());  // returns True if one of the cases are True
  }
  return getTimeStamp() == b.getTimeStamp();
}

bool VXDTFHit::operator<(const VXDTFHit& b) const
{
  B2FATAL("somebody is using the '=='-operator of VXDTFHit, although it does no valid comparison!");
  return getTimeStamp() < b.getTimeStamp();
}

bool VXDTFHit::operator>(const VXDTFHit& b) const
{
  B2FATAL("somebody is using the '=='-operator of VXDTFHit, although it does no valid comparison!");
  return getTimeStamp() > b.getTimeStamp();
}


std::string VXDTFHit::getSectorString() { return FullSecID(m_papaSector).getFullSecString(); }


int VXDTFHit::getClusterIndexU() const
{
  if (m_clusterIndexU != NULL) { return m_clusterIndexU->getOwnIndex(); }
  return -1;
} /**< returns index position of clusterInfo in container, only set for SVDHits */

ClusterInfo* VXDTFHit::getClusterInfoU() { return m_clusterIndexU; } /**< returns pointer to ClusterInfo U, is NULL if value is not set */

int VXDTFHit::getClusterIndexV() const
{
  if (m_clusterIndexV != NULL) { return m_clusterIndexV->getOwnIndex(); }
  return -1;
} /**< returns index position of clusterInfo in container,  only set for SVDHits */

ClusterInfo* VXDTFHit::getClusterInfoV() { return m_clusterIndexV; } /**< returns pointer to ClusterInfo V, is NULL if value is not set */

int VXDTFHit::getClusterIndexUV() const
{
  if (m_clusterIndexUV != NULL) { return m_clusterIndexUV->getOwnIndex(); }
  return -1;
} /**< returns index position of clusterInfo in container,  only set for PXDHits */

ClusterInfo* VXDTFHit::getClusterInfoUV() { return m_clusterIndexUV; }

bool VXDTFHit::isReserved()
{
  if (m_clusterIndexU != NULL) { if (m_clusterIndexU->isReserved() == true) { return true; } }
  if (m_clusterIndexV != NULL) { if (m_clusterIndexV->isReserved() == true) { return true; } }
  if (m_clusterIndexUV != NULL) { if (m_clusterIndexUV->isReserved() == true) { return true; } }
  return false;
}