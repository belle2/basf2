/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/VXDSegmentCell.h"

using namespace std;
using namespace Belle2;
// using namespace Tracking;


VXDSegmentCell::VXDSegmentCell(VXDTFHit* pOuterHit, VXDTFHit* pInnerHit, MapOfSectors::iterator pOuterSector, MapOfSectors::iterator pInnerSector):
  m_pOuterHit(pOuterHit),
  m_pInnerHit(pInnerHit),
  m_pOuterSector(pOuterSector),
  m_pInnerSector(pInnerSector) { m_state = 0; m_activated = true; m_seed = true; m_stateUpgrade = false; }


std::list<Belle2::VXDSegmentCell*>::iterator VXDSegmentCell::eraseInnerNeighbour(std::list<VXDSegmentCell*>::iterator it)
{
  it = m_innerNeighbours.erase(it);
  return it;
} //items.erase(i++);  or  i = items.erase(i);

VXDTFHit* VXDSegmentCell::getInnerHit() const { return m_pInnerHit; }

VXDTFHit* VXDSegmentCell::getOuterHit() const { return m_pOuterHit; }

list<VXDSegmentCell*>& VXDSegmentCell::getInnerNeighbours() { return m_innerNeighbours; }
const list<VXDSegmentCell*>& VXDSegmentCell::getAllInnerNeighbours() const { return m_allInnerNeighbours; }
list<VXDSegmentCell*>& VXDSegmentCell::getOuterNeighbours() { return m_outerNeighbours; }

void VXDSegmentCell::kickFalseFriends(TVector3 primaryVertex)
{
  std::list<Belle2::VXDSegmentCell*>::iterator nbIter = m_innerNeighbours.begin();
  TVector3 innerHit;
  while (nbIter != m_innerNeighbours.end()) {
    VXDSegmentCell* pNextSeg = *nbIter;
    innerHit = pNextSeg->getInnerHit()->getHitCoordinates();
    if (pNextSeg->getState() != this->getState() - 1 || innerHit == primaryVertex) {
      nbIter = this->eraseInnerNeighbour(nbIter);
    } else { ++nbIter; }
  }
}

void VXDSegmentCell::copyNeighbourList()
{
  m_allInnerNeighbours = m_innerNeighbours;
}

void VXDSegmentCell::addInnerNeighbour(Belle2::VXDSegmentCell* aSegment) { m_innerNeighbours.push_back(aSegment); }

void VXDSegmentCell::addOuterNeighbour(Belle2::VXDSegmentCell* aSegment) { m_outerNeighbours.push_back(aSegment); }
