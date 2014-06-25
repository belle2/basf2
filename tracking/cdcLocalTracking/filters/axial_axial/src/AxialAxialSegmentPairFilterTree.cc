/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/AxialAxialSegmentPairFilterTree.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

AxialAxialSegmentPairFilterTree::AxialAxialSegmentPairFilterTree() : AxialAxialSegmentPairFilterTreeBase("axial_axial", "Variables to select correct axial to axial segment combinations and the mc truth.")
{
}

AxialAxialSegmentPairFilterTree::~AxialAxialSegmentPairFilterTree()
{
}

bool AxialAxialSegmentPairFilterTree::setValues(const CellWeight& mcWeight, const CDCAxialAxialSegmentPair& axialAxialSegmentPair)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("EvaluateAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as start segment");
    return false;
  }

  if (ptrEndSegment == nullptr) {
    B2ERROR("EvaluateAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as end segment");
    return false;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  const CDCTrajectory2D& startFit =  startSegment.getTrajectory2D();
  const CDCTrajectory2D& endFit = endSegment.getTrajectory2D();
  const CDCTrajectory2D& commonFit = axialAxialSegmentPair.getTrajectory2D();

  bool mcDecision = not isNotACell(mcWeight);
  setValue < NAMED("mcWeight") > (mcWeight);
  setValue < NAMED("mcDecision") > (mcDecision);

  // Collect decision criterions

  // Sizes
  setValue < NAMED("startSegment_size") > (startSegment.size());
  setValue < NAMED("endSegment_size") > (endSegment.size());

  // Super layers numbers of segments
  setValue < NAMED("startSegment_iSuperLayer") > (startSegment.getISuperLayer());
  setValue < NAMED("endSegment_iSuperLayer") > (endSegment.getISuperLayer());

  // Super layers numbers of segments
  setValue < NAMED("startFit_startISuperLayer") > (startFit.getStartISuperLayer());
  setValue < NAMED("endFit_startISuperLayer") > (endFit.getStartISuperLayer());

  // Super layer extrapolation
  setValue < NAMED("startFit_nextISuperLayer") > (startFit.getNextISuperLayer());
  setValue < NAMED("startFit_nextAxialISuperLayer") > (startFit.getNextAxialISuperLayer());

  setValue < NAMED("endFit_previousISuperLayer") > (endFit.getPreviousISuperLayer());
  setValue < NAMED("endFit_previousAxialISuperLayer") > (endFit.getPreviousAxialISuperLayer());


  // Coalignment indicators
  setValue < NAMED("startFit_totalPerpS_startSegment") > (startFit.getTotalPerpS(startSegment));
  setValue < NAMED("endFit_totalPerpS_startSegment") > (endFit.getTotalPerpS(startSegment));
  setValue < NAMED("commonFit_totalPerpS_startSegment") > (commonFit.getTotalPerpS(startSegment));

  setValue < NAMED("startFit_totalPerpS_endSegment") > (startFit.getTotalPerpS(endSegment));
  setValue < NAMED("endFit_totalPerpS_endSegment") > (endFit.getTotalPerpS(endSegment));
  setValue < NAMED("commonFit_totalPerpS_endSegment") > (commonFit.getTotalPerpS(endSegment));

  setValue < NAMED("startFit_isForwardOrBackwardTo_startSegment") > (startFit.isForwardOrBackwardTo(startSegment));
  setValue < NAMED("endFit_isForwardOrBackwardTo_startSegment") > (endFit.isForwardOrBackwardTo(startSegment));
  setValue < NAMED("commonFit_isForwardOrBackwardTo_startSegment") > (commonFit.isForwardOrBackwardTo(startSegment));

  setValue < NAMED("startFit_isForwardOrBackwardTo_endSegment") > (startFit.isForwardOrBackwardTo(endSegment));
  setValue < NAMED("endFit_isForwardOrBackwardTo_endSegment") > (endFit.isForwardOrBackwardTo(endSegment));
  setValue < NAMED("commonFit_isForwardOrBackwardTo_endSegment") > (commonFit.isForwardOrBackwardTo(endSegment));



  setValue < NAMED("startFit_perpSGap") > (startFit.getPerpSGap(startSegment, endSegment));
  setValue < NAMED("endFit_perpSGap") > (endFit.getPerpSGap(startSegment, endSegment));
  setValue < NAMED("commonFit_perpSGap") > (commonFit.getPerpSGap(startSegment, endSegment));

  setValue < NAMED("startFit_perpSFrontOffset") > (startFit.getPerpSFrontOffset(startSegment, endSegment));
  setValue < NAMED("endFit_perpSFrontOffset") > (endFit.getPerpSFrontOffset(startSegment, endSegment));
  setValue < NAMED("commonFit_perpSFrontOffset") > (commonFit.getPerpSFrontOffset(startSegment, endSegment));

  setValue < NAMED("startFit_perpSBackOffset") > (startFit.getPerpSBackOffset(startSegment, endSegment));
  setValue < NAMED("endFit_perpSBackOffset") > (endFit.getPerpSBackOffset(startSegment, endSegment));
  setValue < NAMED("commonFit_perpSBackOffset") > (commonFit.getPerpSBackOffset(startSegment, endSegment));

  setValue < NAMED("startFit_dist2DToCenter_endSegment") > (startFit.getDist2DToCenter(endSegment));
  setValue < NAMED("endFit_dist2DToCenter_startSegment") > (endFit.getDist2DToCenter(startSegment));

  setValue < NAMED("startFit_dist2DToFront_endSegment") > (startFit.getDist2DToFront(endSegment));
  setValue < NAMED("endFit_dist2DToBack_startSegment") > (endFit.getDist2DToBack(startSegment));

  setValue < NAMED("startFit_absMom2D") > (startFit.getAbsMom2D());
  setValue < NAMED("endFit_absMom2D") > (endFit.getAbsMom2D());

  // Vector2D startCOM = startSegment.getCenterOfMass2D();
  // Vector2D endCOM = endSegment.getCenterOfMass2D();
  // Vector2D startCenter = startFit.getClosest(startCOM);
  // Vector2D endCenter = endFit.getClosest(endCOM);
  // Vector2D startExtrapolatedToEndCenter = startFit.getClosest(endCenter);
  // Vector2D endExtrapolatedToStartCenter = endFit.getClosest(startCenter);

  Vector2D startMomAtCenter = startFit.getUnitMom2DAtCenter(startSegment);
  Vector2D endMomAtCenter = endFit.getUnitMom2DAtCenter(endSegment);

  Vector2D startMomAtExtrapolation = startFit.getUnitMom2DAtCenter(endSegment);
  Vector2D endMomAtExtrapolation = endFit.getUnitMom2DAtCenter(startSegment);

  setValue < NAMED("momAngleAtCenter_startSegment") > (startMomAtCenter.angleWith(endMomAtExtrapolation));
  setValue < NAMED("momAngleAtCenter_endSegment") > (endMomAtCenter.angleWith(startMomAtExtrapolation));

  /*
  //make a cut - make this more sophisticated at some point
  //double cosDeviation = endCenter.cosWith(pointOnFromTrack);
  //double tolerance = cos(PI / 180);
  */
  return true;
}
