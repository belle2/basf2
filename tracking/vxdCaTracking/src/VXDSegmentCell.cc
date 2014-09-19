/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/VXDSegmentCell.h"
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;



void VXDSegmentCell::kickFalseFriends(TVector3 primaryVertex)
{
  std::list<VXDSegmentCell*>::iterator nbIter = m_innerNeighbours.begin();// iterator for the inner neighbours = nb
  TVector3* innerHit;
  int nNBs = 0; // counting neighbours for safety check
  while (nbIter != m_innerNeighbours.end()) {
    VXDSegmentCell* pNextSeg = *nbIter;
    innerHit = pNextSeg->getInnerHit()->getHitCoordinates();

    // will be deleted, if inner nb is not in a compatible state (CA -> inner Nb must have state -1) or it's the primary vertex (which shall not be part of the track candidate)
    if (pNextSeg->getState() != this->getState() - 1 || *innerHit == primaryVertex) {
      nbIter = this->eraseInnerNeighbour(nbIter);
    } else { ++nbIter; }
    nNBs++;
    if (nNBs > int(m_innerNeighbours.size())) { B2FATAL("ClusterInfo::isOverbooked(): iterator crash! nIterations: " << nNBs << ", nNBs: " << m_innerNeighbours.size()) }
  }
}
