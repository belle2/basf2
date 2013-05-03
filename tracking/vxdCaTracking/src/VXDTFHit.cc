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
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


VXDTFHit::VXDTFHit(TVector3 hitPos, int passIndex, int clusterIndexU, int clusterIndexV, int clusterIndexUV, int detectorType, unsigned int papaSector, VxdID aVxdID, float timeStamp):
  m_hit(hitPos),
  m_passIndex(passIndex),
  m_clusterIndexU(clusterIndexU), // SVD only
  m_clusterIndexV(clusterIndexV), // SVD only
  m_clusterIndexUV(clusterIndexUV),
  m_detectorType(detectorType),
  m_papaSector(papaSector),
  m_VxdID(aVxdID),
  m_timeStamp(timeStamp) { m_attachedTrackCandidates = 0; /*m_attachedCells = 0;*/ }

/** operator overload **/
bool VXDTFHit::operator==(const VXDTFHit& b) const
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


/** getter **/
TVector3 VXDTFHit::getHitCoordinates() const {return TVector3(m_hit.X(), m_hit.Y(), m_hit.Z()); } /// get global position of Hit
const std::vector<int>& VXDTFHit::getAttachedInnerCell() const  { return m_attachedInnerCells; }
const std::vector<int>& VXDTFHit::getAttachedOuterCell() const  { return m_attachedOuterCells; } /// not used so far
int VXDTFHit::getNumberOfSegments() { return int(m_attachedInnerCells.size() + m_attachedOuterCells.size()); }
std::string VXDTFHit::getSectorString()
{
  FullSecID fullSecID = FullSecID(m_papaSector);
  return fullSecID.getFullSecString();
}


/** setter **/
void VXDTFHit::addInnerCell(int newCell) { m_attachedInnerCells.push_back(newCell); }
void VXDTFHit::addOuterCell(int newCell) { m_attachedOuterCells.push_back(newCell); }
