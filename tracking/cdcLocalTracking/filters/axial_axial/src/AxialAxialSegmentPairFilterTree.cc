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

  setValue < NAMED("startSegment_size") > (startSegment.size());
  setValue < NAMED("endSegment_size") > (endSegment.size());

  setValue < NAMED("startSegment_iSuperLayer") > (startSegment.getISuperLayer());
  setValue < NAMED("endSegment_iSuperLayer") > (endSegment.getISuperLayer());

  setValue < NAMED("startFit_startISuperLayer") > (startFit.getStartISuperLayer());
  setValue < NAMED("endFit_startISuperLayer") > (endFit.getStartISuperLayer());

  setValue < NAMED("startFit_nextISuperLayer") > (startFit.getNextISuperLayer());
  setValue < NAMED("endFit_previousISuperLayer") > (endFit.getPreviousISuperLayer());

  setValue < NAMED("startFit_nextAxialISuperLayer") > (startFit.getNextAxialISuperLayer());
  setValue < NAMED("endFit_previousAxialISuperLayer") > (endFit.getPreviousAxialISuperLayer());



  setValue < NAMED("startSegment_isForwardTrajectory_startFit") > (startSegment.isForwardTrajectory(startFit));
  setValue < NAMED("startSegment_isForwardTrajectory_endFit") > (startSegment.isForwardTrajectory(endFit));
  setValue < NAMED("startSegment_isForwardTrajectory_commonFit") > (startSegment.isForwardTrajectory(commonFit));

  setValue < NAMED("endSegment_isForwardTrajectory_startFit") > (endSegment.isForwardTrajectory(startFit));
  setValue < NAMED("endSegment_isForwardTrajectory_endFit") > (endSegment.isForwardTrajectory(endFit));
  setValue < NAMED("endSegment_isForwardTrajectory_commonFit") > (endSegment.isForwardTrajectory(commonFit));

  setValue < NAMED("startSegment_totalPerpS_startFit") > (startSegment.getTotalPerpS(startFit));
  setValue < NAMED("startSegment_totalPerpS_endFit") > (startSegment.getTotalPerpS(endFit));
  setValue < NAMED("startSegment_totalPerpS_commonFit") > (startSegment.getTotalPerpS(commonFit));

  setValue < NAMED("endSegment_totalPerpS_startFit") > (endSegment.getTotalPerpS(startFit));
  setValue < NAMED("endSegment_totalPerpS_endFit") > (endSegment.getTotalPerpS(endFit));
  setValue < NAMED("endSegment_totalPerpS_commonFit") > (endSegment.getTotalPerpS(commonFit));

  setValue < NAMED("perpS_gap_startFit") > (endSegment.getFrontPerpS(startFit) - startSegment.getBackPerpS(startFit));
  setValue < NAMED("perpS_gap_endFit") > (endSegment.getFrontPerpS(endFit) - startSegment.getBackPerpS(endFit));
  setValue < NAMED("perpS_gap_commonFit") > (endSegment.getFrontPerpS(commonFit) - startSegment.getBackPerpS(commonFit));

  setValue < NAMED("perpS_offset_startHits_startFit") > (endSegment.getFrontPerpS(startFit) - startSegment.getFrontPerpS(startFit));
  setValue < NAMED("perpS_offset_startHits_endFit") > (endSegment.getFrontPerpS(endFit) - startSegment.getFrontPerpS(endFit));
  setValue < NAMED("perpS_offset_startHits_commonFit") > (endSegment.getFrontPerpS(commonFit) - startSegment.getFrontPerpS(commonFit));

  setValue < NAMED("perpS_offset_endHits_startFit") > (endSegment.getBackPerpS(startFit) - startSegment.getBackPerpS(startFit));
  setValue < NAMED("perpS_offset_endHits_endFit") > (endSegment.getBackPerpS(endFit) - startSegment.getBackPerpS(endFit));
  setValue < NAMED("perpS_offset_endHits_commonFit") > (endSegment.getBackPerpS(commonFit) - startSegment.getBackPerpS(commonFit));

  Vector2D startCOM = startSegment.getCenterOfMass2D();
  Vector2D endCOM = endSegment.getCenterOfMass2D();

  setValue < NAMED("startSegment_com_x") > (startCOM.x());
  setValue < NAMED("startSegment_com_y") > (startCOM.y());

  setValue < NAMED("endSegment_com_x") > (endCOM.x());
  setValue < NAMED("endSegment_com_y") > (endCOM.y());


  FloatType startAbsMom = startFit.getAbsMom2D();
  FloatType endAbsMom = endFit.getAbsMom2D();

  setValue < NAMED("startSegment_absmom") > (startAbsMom);
  setValue < NAMED("endSegment_absmom") > (endAbsMom);


  Vector2D startCenter = startFit.getClosest(startCOM);
  Vector2D endCenter = endFit.getClosest(endCOM);

  setValue < NAMED("startSegment_center_x") > (startCenter.x());
  setValue < NAMED("startSegment_center_y") > (startCenter.y());

  setValue < NAMED("endSegment_center_x") > (endCenter.x());
  setValue < NAMED("endSegment_center_y") > (endCenter.y());


  Vector2D startExtrapolatedToEndCenter = startFit.getClosest(endCenter);
  Vector2D endExtrapolatedToStartCenter = endFit.getClosest(startCenter);

  setValue < NAMED("startSegment_extrapolation_x") > (startExtrapolatedToEndCenter.x());
  setValue < NAMED("startSegment_extrapolation_y") > (startExtrapolatedToEndCenter.y());

  setValue < NAMED("endSegment_extrapolation_x") > (endExtrapolatedToStartCenter.x());
  setValue < NAMED("endSegment_extrapolation_y") > (endExtrapolatedToStartCenter.y());


  setValue < NAMED("startSegment_extrapolation_to_center_distance") > (startExtrapolatedToEndCenter.distance(endCenter));
  setValue < NAMED("endSegment_extrapolation_to_center_distance") > (endExtrapolatedToStartCenter.distance(startCenter));

  Vector2D startMomAtCenter = startFit.getUnitMom2D(startCenter);
  Vector2D endMomAtCenter = endFit.getUnitMom2D(endCenter);

  Vector2D startMomAtExtrapolation = startFit.getUnitMom2D(startExtrapolatedToEndCenter);
  Vector2D endMomAtExtrapolation = endFit.getUnitMom2D(endExtrapolatedToStartCenter);

  setValue < NAMED("startFit_center_unitMom_x") > (startMomAtCenter.x());
  setValue < NAMED("startFit_center_unitMom_y") > (startMomAtCenter.y());

  setValue < NAMED("startFit_extrapolation_unitMom_x") > (startMomAtExtrapolation.x());
  setValue < NAMED("startFit_extrapolation_unitMom_y") > (startMomAtExtrapolation.y());


  setValue < NAMED("endFit_center_unitMom_x") > (endMomAtCenter.x());
  setValue < NAMED("endFit_center_unitMom_y") > (endMomAtCenter.y());

  setValue < NAMED("endFit_extrapolation_unitMom_x") > (endMomAtExtrapolation.x());
  setValue < NAMED("endFit_extrapolation_unitMom_y") > (endMomAtExtrapolation.y());

  setValue < NAMED("start_to_end_mom_angle") > (startMomAtCenter.angleWith(endMomAtExtrapolation));
  setValue < NAMED("end_to_start_mom_angle") > (endMomAtCenter.angleWith(startMomAtExtrapolation));

  /*
  m_distanceAtStart = startCenter.distance(endExtrapolatedToStartCenter);
  m_distanceAtEnd = endCenter.distance(startExtrapolatedToEndCenter);

  //make a cut - make this more sophisticated at some point
  //double cosDeviation = endCenter.cosWith(pointOnFromTrack);
  //double tolerance = cos(PI / 180);
  */
  return true;
}
