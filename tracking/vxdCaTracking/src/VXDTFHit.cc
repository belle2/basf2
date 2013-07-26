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
//  if (m_clusterInfoU != NULL ) { return m_clusterInfoU->getOwnIndex(); }
  if (m_clusterInfoU != NULL) { return m_clusterInfoU->getRealIndex(); }
  return -1;
} /**< returns index position of clusterInfo in container, only set for SVDHits */



ClusterInfo* VXDTFHit::getClusterInfoU() const { return m_clusterInfoU; } /**< returns pointer to ClusterInfo U, is NULL if value is not set */



int VXDTFHit::getClusterIndexV() const
{
//  if (m_clusterInfoV != NULL ) { return m_clusterInfoV->getOwnIndex(); }
  if (m_clusterInfoV != NULL) { return m_clusterInfoV->getRealIndex(); }
  return -1;
} /**< returns index position of clusterInfo in container,  only set for SVDHits */



ClusterInfo* VXDTFHit::getClusterInfoV() const { return m_clusterInfoV; } /**< returns pointer to ClusterInfo V, is NULL if value is not set */



int VXDTFHit::getClusterIndexUV() const
{
//  if (m_clusterInfoUV != NULL ) { return m_clusterInfoUV->getOwnIndex(); }
  if (m_clusterInfoUV != NULL) { return m_clusterInfoUV->getRealIndex(); }
  return -1;
} /**< returns index position of clusterInfo in container,  only set for PXDHits */
ClusterInfo* VXDTFHit::getClusterInfoUV() const { return m_clusterInfoUV; }  /**< returns pointer to ClusterInfo UV, is NULL if value is not set */



bool VXDTFHit::isReserved() const
{
  if (m_clusterInfoU != NULL) { if (m_clusterInfoU->isReserved() == true) { return true; } }
  if (m_clusterInfoV != NULL) { if (m_clusterInfoV->isReserved() == true) { return true; } }
  if (m_clusterInfoUV != NULL) { if (m_clusterInfoUV->isReserved() == true) { return true; } }
  return false;
}


// void removeTrackCandidate() {
//  m_attachedTrackCandidates--;
//  if (m_clusterInfoU != NULL) { m_clusterInfoU->removeTrackCandidate(); }
//   if (m_clusterInfoV != NULL) { m_clusterInfoV->removeTrackCandidate(); }
//   if (m_clusterInfoUV != NULL) { m_clusterInfoUV->removeTrackCandidate(); }
//
//
// } /**< decrease number of TCs using this hit */