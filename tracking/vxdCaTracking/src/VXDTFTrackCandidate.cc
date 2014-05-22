/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/VXDTFTrackCandidate.h"
#include <math.h>
#include <framework/logging/Logger.h>
#include "tracking/vxdCaTracking/ClusterInfo.h"


using namespace std;
using namespace Belle2;





/** getter **/





/** setter **/



// const vector<int>& VXDTFTrackCandidate::getSVDHitIndices()
// {
//   m_svdHitIndices.clear();
//   m_svdHitIndices.reserve(m_attachedHits.size());
//   int index;
//   for (VXDTFHit * aHit : m_attachedHits) {
//     if (aHit->getDetectorType() == Const::SVD) { /* SVD */
//       index = aHit->getClusterIndexU();
//       if (index != -1) { m_svdHitIndices.push_back(index); }
//       index = aHit->getClusterIndexV();
//       if (index != -1) { m_svdHitIndices.push_back(index); }
//     }
//   }
//   return m_svdHitIndices;
// } /**< returns indices of svdClusters forming current TC */


// const vector<int>& VXDTFTrackCandidate::getPXDHitIndices()
// {
//   m_pxdHitIndices.clear();
//   m_pxdHitIndices.reserve(m_attachedHits.size());
//   int index;
//   for (VXDTFHit * aHit : m_attachedHits) {
//     if (aHit->getDetectorType() == Const::PXD) { /* PXD */
//       index = aHit->getClusterIndexUV();
//       if (index != -1) { m_pxdHitIndices.push_back(index); }
//     }
//   }
//   return m_pxdHitIndices;
// } /**< returns indices of svdClusters forming current TC */


list<int> VXDTFTrackCandidate::getHopfieldHitIndices()
{
  list<int> indices;
  for (VXDTFHit * aHit : m_attachedHits) {
    if ((aHit->getDetectorType() == Const::PXD) or (aHit->getDetectorType() == Const::TEST)) { /*PXD or Telescope */
      indices.push_back(aHit->getClusterInfoUV()->getOwnIndex());
    } else { /* SVD */
      indices.push_back(aHit->getClusterInfoU()->getOwnIndex());
      indices.push_back(aHit->getClusterInfoV()->getOwnIndex());
    }
  }
  return indices;
}


// void VXDTFTrackCandidate::setCondition(bool newCondition)
// {
//   if (m_alive == true && newCondition == false) {   // in this case, the TC will be deactivated
//     for (VXDTFHit * aHit : m_attachedHits) {
//       aHit->removeTrackCandidate();
//       // TODO: for each ClusterInfo in aHit-> removeTrackCandidate(this);
//     }
//   } else if (m_alive == false && newCondition == true) {   // in this case the TC will be (re)activated
//     for (VXDTFHit * aHit : m_attachedHits) {
//       aHit->addTrackCandidate();
//     }
//   }
//   m_alive = newCondition;
// }


void VXDTFTrackCandidate::removeVirtualHit()   /// removing virtual hit/segment from TC after collecting the whole TC. This is done after the TCC and the TCC filter since the TCC needs the information provided by the virtual hit/segment.
{
  // since both containers are vectors and therefore do not support arbitrary deletion of entries, the following procedure is lacking of elegance
  int virtualIndex = -1; // contains the index number of the virtual hit/segment (currently only one hit/segment per TC supported)
  int numOfEntries = m_attachedHits.size();
  for (int thisHit = 0 ; thisHit < numOfEntries; ++thisHit) {
    if (m_attachedHits[thisHit]->getDetectorType() == Const::IR) {
      virtualIndex = thisHit;
      break; // since there are no curling tracks supported so far, only one virtual hit per TC is possible
    }
  }
  if (virtualIndex > -1) {
    vector<VXDTFHit*> tempHitVector;
    tempHitVector.reserve(numOfEntries);
    for (int thisHit = 0 ; thisHit < numOfEntries; ++thisHit) {
      if (virtualIndex == thisHit) { continue; }
      tempHitVector.push_back(m_attachedHits[thisHit]);
    }
    m_attachedHits = tempHitVector;
  }

  virtualIndex = -1;
  numOfEntries = m_attachedCells.size();
  for (int thisSeg = 0 ; thisSeg < numOfEntries; ++thisSeg) {
    if (m_attachedCells[thisSeg]->getInnerHit()->getDetectorType() == Const::IR or
        m_attachedCells[thisSeg]->getOuterHit()->getDetectorType() == Const::IR) {
      virtualIndex = thisSeg;
      break; // since there are no curling tracks supported so far, only one virtual segment per TC is possible
    }
  }
  if (virtualIndex > -1) {
    vector<VXDSegmentCell*> tempSegVector;
    tempSegVector.reserve(numOfEntries);
    for (int thisSeg = 0 ; thisSeg < numOfEntries; ++thisSeg) {
      if (virtualIndex == thisSeg) { continue; }
      tempSegVector.push_back(m_attachedCells[thisSeg]);
    }
    m_attachedCells = tempSegVector;
  }
}



TVector3 VXDTFTrackCandidate::getInitialMomentum()
{
  if (m_initialValuesSet == false) { B2FATAL(" getInitialMomentum executed although no values set yet - arborting..."); }
  return m_initialMomentum;
}



bool VXDTFTrackCandidate::checkReserved()
{
  int countSuccessfull = 0, countTotal = 0, alreadyReservedByMe = 0, alreadyReservedByAnother = 0;
  bool successfull;
  for (VXDTFHit * aHit : m_attachedHits) {
    if ((aHit->getDetectorType() == Const::PXD) or (aHit->getDetectorType() == Const::TEST)) { /*PXD or Telescope*/
      countTotal++;

      successfull = aHit->getClusterInfoUV()->checkReserved(this);
      if (successfull == true) { alreadyReservedByMe++; continue; }

      if (aHit->getClusterInfoUV()->isReserved() == true) { alreadyReservedByAnother++; continue; }

      successfull = aHit->getClusterInfoUV()->setReserved(this);
      if (successfull == true) ++countSuccessfull;

    } else { /* SVD */
      countTotal += 2;

      successfull = aHit->getClusterInfoU()->checkReserved(this);
      if (successfull == true) { alreadyReservedByMe++; continue; }

      if (aHit->getClusterInfoU()->isReserved() == true) { alreadyReservedByAnother++; continue; }

      successfull = aHit->getClusterInfoU()->setReserved(this);
      if (successfull == true) ++countSuccessfull;


      successfull = aHit->getClusterInfoV()->checkReserved(this);
      if (successfull == true) { alreadyReservedByMe++; continue; }

      if (aHit->getClusterInfoV()->isReserved() == true) { alreadyReservedByAnother++; continue; }

      successfull = aHit->getClusterInfoV()->setReserved(this);
      if (successfull == true) ++countSuccessfull;
    }
  }
  if ((countSuccessfull + alreadyReservedByMe) != countTotal) {
    B2INFO("VXDTFTrackCandidate:checkReserved: TC " << getTrackNumber() << " failed to reserve Clusters! successfull: " << countSuccessfull << ", alreadyReservedByMe: " << alreadyReservedByMe << ", alreadyReservedByAnother: " << alreadyReservedByAnother << ", total: " << countTotal)
    return false;
  }
  B2INFO("VXDTFTrackCandidate:checkReserved: TC " << getTrackNumber() << " reserved Clusters! successfull: " << countSuccessfull << ", alreadyReservedByMe: " << alreadyReservedByMe << ", alreadyReservedByAnother: " << alreadyReservedByAnother << ", total: " << countTotal)
  return true;
}



bool VXDTFTrackCandidate::setReserved()
{
  int countSuccessfull = 0, countTotal = 0;
  bool successfull;
  for (VXDTFHit * aHit : m_attachedHits) {
    if (aHit->getDetectorType() == Const::PXD) { /*PXD */
      countTotal++;
      if (aHit->getClusterInfoUV() != NULL) {
        successfull = aHit->getClusterInfoUV()->setReserved(this);
        if (successfull == true) { ++countSuccessfull; }
      } else {
        B2WARNING("aHit in sector " << aHit->getSectorString() << " has broken PXDCluster id/pointer" << aHit->getClusterIndexUV() << "/" << aHit->getClusterInfoUV())
      }
    } else if (aHit->getDetectorType() == Const::TEST) { /*Telescope */
      countTotal++;
      if (aHit->getClusterInfoUV() != NULL) {
        successfull = aHit->getClusterInfoUV()->setReserved(this);
        if (successfull == true) { ++countSuccessfull; }
      } else {
        B2WARNING("aHit in sector " << aHit->getSectorString() << " has broken TELCluster id/pointer" << aHit->getClusterIndexUV() << "/" << aHit->getClusterInfoUV())
      }
    } else { /* SVD */
      countTotal += 2;
      if (aHit->getClusterInfoU() != NULL) {
        successfull = aHit->getClusterInfoU()->setReserved(this);
        if (successfull == true) { ++countSuccessfull; }
      } else {
        B2WARNING("aHit in sector " << aHit->getSectorString() << " has broken SVDClusterU id/pointer" << aHit->getClusterIndexUV() << "/" << aHit->getClusterInfoUV())
      }

      if (aHit->getClusterInfoV() != NULL) {
        successfull = aHit->getClusterInfoV()->setReserved(this);
        if (successfull == true) { ++countSuccessfull; }
      } else {
        B2WARNING("aHit in sector " << aHit->getSectorString() << " has broken SVDClusterV id/pointer" << aHit->getClusterIndexUV() << "/" << aHit->getClusterInfoUV())
      }
    }
  }
  if (countSuccessfull != countTotal) {
    B2WARNING("VXDTFTrackCandidate:setReserved: TC " << getTrackNumber() << " failed to reserve Clusters! " << countSuccessfull << " of " << countTotal << " clusters could be reserved")
    return false;
  }
  m_reserved = true;
  return true;
}