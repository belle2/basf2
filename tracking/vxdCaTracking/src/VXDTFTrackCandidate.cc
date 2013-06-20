/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/VXDTFTrackCandidate.h"
#include <math.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <boost/foreach.hpp>


using namespace std;
using namespace Belle2;



VXDTFTrackCandidate::VXDTFTrackCandidate(VXDTFTrackCandidate*& other):
  m_attachedHits((*other).m_attachedHits),
  m_attachedCells((*other).m_attachedCells),
  m_svdHitIndices((*other).m_svdHitIndices),
  m_pxdHitIndices((*other).m_pxdHitIndices),
  m_hopfieldHitIndices((*other).m_hopfieldHitIndices),
  m_overlapping((*other).m_overlapping),
  m_alive((*other).m_alive),
  m_qualityIndex((*other).m_qualityIndex)
{
  if (m_alive == true) { BOOST_FOREACH(VXDTFHit * aHit, m_attachedHits) { aHit->addTrackCandidate(); } }   // each time it gets copied, its hits have to be informed about that step
  /*m_neuronValue = 0; m_overlapping = false; m_alive = true; m_qualityIndex = 1.0;*/
}


/** getter **/
vector<TVector3*> VXDTFTrackCandidate::getHitCoordinates()
{
  vector<TVector3*> coordinates;
  BOOST_FOREACH(VXDTFHit * hit, m_attachedHits) {
    coordinates.push_back(hit->getHitCoordinates());
  }
  return coordinates;
}

bool VXDTFTrackCandidate::checkOverlappingState()
{
  int rivalsAlive = 0;
  BOOST_FOREACH(VXDTFTrackCandidate * rival, m_bookingRivals) {
    if (rival->getCondition() == false) continue;
    rivalsAlive++;
  }
  if (rivalsAlive != 0) { m_overlapping = true; return true; } else { m_overlapping = false; return false; }
}


/** setter **/
void VXDTFTrackCandidate::addBookingRival(VXDTFTrackCandidate* aTC)
{
  int ctr = 0;
  m_overlapping = true;
  BOOST_FOREACH(VXDTFTrackCandidate * rival, m_bookingRivals) {
    if (aTC != rival) { ctr++; }
  }
  if (int(m_bookingRivals.size()) == ctr) { m_bookingRivals.push_back(aTC); }

}


vector<int> VXDTFTrackCandidate::getSVDHitIndices()
{
  m_svdHitIndices.clear();
  BOOST_FOREACH(VXDTFHit * aHit, m_attachedHits) {
    if (aHit->getDetectorType() == Const::SVD) { /* SVD */
      m_svdHitIndices.push_back(aHit->getClusterIndexU());
      m_svdHitIndices.push_back(aHit->getClusterIndexV());
    }
  }
  return m_svdHitIndices;
} /**< returns indices of svdClusters forming current TC */

vector<int> VXDTFTrackCandidate::getPXDHitIndices()
{
  m_pxdHitIndices.clear();
  BOOST_FOREACH(VXDTFHit * aHit, m_attachedHits) {
    if (aHit->getDetectorType() == Const::PXD) { /* PXD */
      m_pxdHitIndices.push_back(aHit->getClusterIndexUV());
    }
  }
  return m_pxdHitIndices;
} /**< returns indices of svdClusters forming current TC */

list<int> VXDTFTrackCandidate::getHopfieldHitIndices()
{
  list<int> indices;
  BOOST_FOREACH(VXDTFHit * aHit, m_attachedHits) {
    if (aHit->getDetectorType() == Const::PXD) { /*PXD */
      indices.push_back(aHit->getClusterIndexUV());
    } else { /* SVD */
      indices.push_back(aHit->getClusterIndexU());
      indices.push_back(aHit->getClusterIndexV());
    }
  }
  return indices;
}



void VXDTFTrackCandidate::setCondition(bool newCondition)
{
  if (m_alive == true && newCondition == false) {   // in this case, the TC will be deactivated
    BOOST_FOREACH(VXDTFHit * aHit, m_attachedHits) {
      aHit->removeTrackCandidate();
    }
  } else if (m_alive == false && newCondition == true) {   // in this case the TC will be (re)activated
    BOOST_FOREACH(VXDTFHit * aHit, m_attachedHits) {
      aHit->addTrackCandidate();
    }
  }
  m_alive = newCondition;
}


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