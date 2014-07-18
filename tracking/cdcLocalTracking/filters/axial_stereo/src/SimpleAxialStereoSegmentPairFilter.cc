/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/SimpleAxialStereoSegmentPairFilter.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

SimpleAxialStereoSegmentPairFilter::SimpleAxialStereoSegmentPairFilter() : m_riemannFitter()
{
  m_riemannFitter.useOnlyOrientation();
}

SimpleAxialStereoSegmentPairFilter::~SimpleAxialStereoSegmentPairFilter()
{

}


void SimpleAxialStereoSegmentPairFilter::clear()
{

}



void SimpleAxialStereoSegmentPairFilter::initialize()
{
}


void SimpleAxialStereoSegmentPairFilter::terminate()
{
}



CellWeight SimpleAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair(const CDCAxialStereoSegmentPair& axialStereoSegmentPair)
{

  const CDCAxialRecoSegment2D* ptrStartSegment = axialStereoSegmentPair.getStartSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();

  if (ptrStartSegment == nullptr) {
    B2ERROR("SimpleAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair invoked with nullptr as start segment");
    return NOT_A_CELL;
  }

  if (ptrEndSegment == nullptr) {
    B2ERROR("SimpleAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair invoked with nullptr as end segment");
    return NOT_A_CELL;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  //do fits
  const CDCTrajectory2D& startFit = getFittedTrajectory2D(startSegment);
  const CDCTrajectory2D& endFit = getFittedTrajectory2D(endSegment);


  // Check if segments are coaligned
  bool endSegmentIsCoaligned = startFit.getTotalPerpS(endSegment) >= 0.0;
  bool startSegmentIsCoaligned = endFit.getTotalPerpS(startSegment) >= 0.0;

  if (not endSegmentIsCoaligned or not startSegmentIsCoaligned) {
    return NOT_A_CELL;
  }

  // Check if there is a positive gap between start and end segment
  //FloatType startFitGap = startFit.getPerpSGap(startSegment, endSegment);
  //FloatType endFitGap = endFit.getPerpSGap(startSegment, endSegment);

  //if (startFitGap < 0 or startFitGap > 100 or endFitGap < 0 or endFitGap > 100) {
  //  return NOT_A_CELL;
  //}

  FloatType startFitFrontOffset = startFit.getPerpSFrontOffset(startSegment, endSegment);
  FloatType endFitBackOffset = endFit.getPerpSBackOffset(startSegment, endSegment);

  if (startFitFrontOffset < 0 or endFitBackOffset < 0) {
    return NOT_A_CELL;
  }

  const CDCTrajectory3D& combinedFit = getFittedTrajectory3D(axialStereoSegmentPair);
  combinedFit.getChi2();

  return startSegment.size() + endSegment.size();

}



const CDCTrajectory2D& SimpleAxialStereoSegmentPairFilter::getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const
{

  CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, segment);
  }
  return trajectory2D;

}



/// Returns the three dimensional trajectory of the axial stereo segment pair. Also fits it if necessary.
const CDCTrajectory3D& SimpleAxialStereoSegmentPairFilter::getFittedTrajectory3D(const CDCAxialStereoSegmentPair& axialStereoSegmentPair) const
{
  const CDCAxialRecoSegment2D* ptrStartSegment = axialStereoSegmentPair.getStartSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  //do fits if still necessary.
  const CDCTrajectory2D& startFit = getFittedTrajectory2D(startSegment);
  const CDCTrajectory2D& endFit = getFittedTrajectory2D(endSegment);

  // combine axial and stereo fit to three dimensional trajectory
  axialStereoSegmentPair.fuseTrajectories();
  return axialStereoSegmentPair.getTrajectory3D();

}

