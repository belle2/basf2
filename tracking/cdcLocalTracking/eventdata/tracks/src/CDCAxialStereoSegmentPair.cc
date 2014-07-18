/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCAxialStereoSegmentPair.h"

#include <tracking/cdcLocalTracking/fitting/CDCAxialStereoFusion.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCAxialStereoSegmentPair)



CDCAxialStereoSegmentPair::CDCAxialStereoSegmentPair() : m_startSegment(nullptr),  m_endSegment(nullptr)
{

}



CDCAxialStereoSegmentPair::CDCAxialStereoSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                                                     const CDCAxialRecoSegment2D* endSegment):
  m_startSegment(startSegment),
  m_endSegment(endSegment)
{
  if (not startSegment) B2ERROR("CDCAxialStereoSegmentPair initialized with nullptr as start segment");
  if (not endSegment) B2ERROR("CDCAxialStereoSegmentPair initialized with nullptr as end segment");
}



CDCAxialStereoSegmentPair::CDCAxialStereoSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                                                     const CDCAxialRecoSegment2D* endSegment,
                                                     const CDCTrajectory3D& trajectory3D) :
  m_startSegment(startSegment),
  m_endSegment(endSegment),
  m_trajectory3D(trajectory3D)
{
  if (not startSegment) B2ERROR("CDCAxialStereoSegmentPair initialized with nullptr as start segment");
  if (not endSegment) B2ERROR("CDCAxialStereoSegmentPair initialized with nullptr as end segment");
}



CDCAxialStereoSegmentPair::~CDCAxialStereoSegmentPair()
{
}


void CDCAxialStereoSegmentPair::fuseTrajectories() const
{

  clearTrajectory3D();

  const CDCRecoSegment2D* ptrStartSegment = getStartSegment();
  const CDCRecoSegment2D* ptrEndSegment = getEndSegment();

  if (not ptrStartSegment) {
    B2WARNING("Start segment unset.");
    return;
  }

  if (not ptrEndSegment) {
    B2WARNING("End segment unset.");
    return;
  }

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  m_trajectory3D = ::fuseTrajectories(startSegment, endSegment);

}

