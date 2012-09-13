/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/VXDTFTrackCandidate.h>
// #include <tracking/dataobjects/VXDTFHit.h>
// #include <tracking/dataobjects/VXDSegmentCell.h>

#include <math.h>

#include <boost/foreach.hpp>


using namespace std;
using namespace Belle2;


ClassImp(VXDTFTrackCandidate)
/** getter **/
std::vector<Belle2::VXDSegmentCell*> VXDTFTrackCandidate::getSegments() { return m_attachedCells; }
std::vector<Belle2::VXDTFHit*> VXDTFTrackCandidate::getHits() { return m_attachedHits; }

/** setter **/
void VXDTFTrackCandidate::addSVDClusterIndex(int anIndex) { m_svdHitIndices.push_back(anIndex); }
void VXDTFTrackCandidate::addPXDClusterIndex(int anIndex) { m_pxdHitIndices.push_back(anIndex); }
void VXDTFTrackCandidate::addHopfieldClusterIndex(int anIndex) { m_hopfieldHitIndices.push_back(anIndex); }


void VXDTFTrackCandidate::addSegments(Belle2::VXDSegmentCell* pCell) { m_attachedCells.push_back(pCell); }

// void VXDTFTrackCandidate::addSegments(std::vector<VXDSegmentCell*> vCells) { /// TODO: check whether this memberfunction is still needed
//  int num = vCells.size();
//  for (int i = 0; i < num; i++) {
//    m_attachedCells.push_back(vCells[i]);
//  }
// }
void VXDTFTrackCandidate::addHits(VXDTFHit* pHit) { m_attachedHits.push_back(pHit); }

// void VXDTFTrackCandidate::addHits(std::vector<VXDTFHit*> vHits) { /// TODO: check whether this memberfunction is still needed
//  int num = vHits.size();
//  for (int i = 0; i < num; i++) {
//    m_attachedHits.push_back(vHits[i]);
//  }
// }

void VXDTFTrackCandidate::setOverlappingState(bool newState) { m_overlapping = newState; }

void VXDTFTrackCandidate::setTrackQuality(double newVal) { m_qualityIndex = newVal; }

void VXDTFTrackCandidate::setQQQ(double qqqScore, double maxScore)
{
  m_qqq = sqrt(qqqScore / maxScore);
}

void VXDTFTrackCandidate::setCondition(bool newCondition)
{
  m_alive = newCondition;
  if (newCondition == false) {
    BOOST_FOREACH(VXDTFHit * aHit, m_attachedHits) {
      aHit->removeTrackCandidate();
    }
  }
}

void VXDTFTrackCandidate::setNeuronValue(float aValue) { m_neuronValue = aValue; }

void VXDTFTrackCandidate::removeVirtualHit()   /// removing virtual hit/segment from TC after collecting the whole TC. This is done after the TCC and the TCC filter since the TCC needs the information provided by the virtual hit/segment.
{
  // since both containers are vectors and therefore do not support arbitrary deletion of entries, the following procedure is lacking of elegance
  int virtualIndex = -1; // contains the index number of the virtual hit/segment (currently only one hit/segment per TC supported)
  int numOfEntries = m_attachedHits.size();
  for (int thisHit = 0 ; thisHit < numOfEntries; ++thisHit) {
    if (m_attachedHits[thisHit]->getDetectorType() == -1) {
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
    if (m_attachedCells[thisSeg]->getInnerHit()->getDetectorType() == -1 or
        m_attachedCells[thisSeg]->getOuterHit()->getDetectorType() == -1) {
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

void VXDTFTrackCandidate::setInitialValue(TVector3 aHit, TVector3 pVector, int pdg) { m_initialHit = aHit; m_initialMomentum = pVector; m_pdgCode = pdg; }

void VXDTFTrackCandidate::setPassIndex(int anIndex) { m_passIndex = anIndex; }
