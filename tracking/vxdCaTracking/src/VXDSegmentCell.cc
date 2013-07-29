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
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;



void VXDSegmentCell::kickFalseFriends(TVector3 primaryVertex)
{
  std::list<VXDSegmentCell*>::iterator nbIter = m_innerNeighbours.begin();
  TVector3* innerHit;
  int nNBs = 0;
  while (nbIter != m_innerNeighbours.end()) {
    VXDSegmentCell* pNextSeg = *nbIter;
    innerHit = pNextSeg->getInnerHit()->getHitCoordinates();
    if (pNextSeg->getState() != this->getState() - 1 || *innerHit == primaryVertex) {
      nbIter = this->eraseInnerNeighbour(nbIter);
    } else { ++nbIter; }
    nNBs++;
    if (nNBs > int(m_innerNeighbours.size())) { B2FATAL("ClusterInfo::isOverbooked(): iterator crash! nIterations: " << nNBs << ", nNBs: " << m_innerNeighbours.size()) }
  }
}
