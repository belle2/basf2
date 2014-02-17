/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCSegmentTripleFilter.h"

#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>


using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

MCSegmentTripleFilter::MCSegmentTripleFilter() : m_mcLookUp(CDCMCLookUp::Instance()) {;}
MCSegmentTripleFilter::MCSegmentTripleFilter(const CDCMCLookUp& mcLookUp) : m_mcLookUp(mcLookUp) {;}

MCSegmentTripleFilter::~MCSegmentTripleFilter()
{
}

CellWeight MCSegmentTripleFilter::isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair)
{

  return m_mcAxialAxialSegmentPairFilter.isGoodAxialAxialSegmentPair(axialAxialSegmentPair);

  const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodAxialAxialSegmentPair invoked with nullptr as start segment");
    return NOT_A_CELL;
  }

  if (ptrEndSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodAxialAxialSegmentPair invoked with nullptr as end segment");
    return NOT_A_CELL;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;



  if (startSegment.empty() or endSegment.empty()) return NOT_A_CELL;

  //check if the segments have the same trackid
  std::pair<float, ITrackType> efficiencyOfStart = m_mcLookUp.getHighestEfficieny(startSegment);
  std::pair<float, ITrackType> efficiencyOfEnd   = m_mcLookUp.getHighestEfficieny(endSegment);

  if (efficiencyOfStart.first < 0.5 or efficiencyOfEnd.first < 0.5 or
      efficiencyOfStart.second != efficiencyOfEnd.second or
      efficiencyOfStart.second == INVALID_ITRACK) return NOT_A_CELL;

  ITrackType iTrack = efficiencyOfStart.second;

  //check if the alignement is correct
  const CDCSimHit* firstSimHitOfStart = m_mcLookUp.getFirstSimHit(startSegment, iTrack);
  const CDCSimHit* lastSimHitOfStart = m_mcLookUp.getLastSimHit(startSegment, iTrack);

  const CDCSimHit* firstSimHitOfEnd = m_mcLookUp.getFirstSimHit(endSegment, iTrack);
  const CDCSimHit* lastSimHitOfEnd  = m_mcLookUp.getLastSimHit(endSegment, iTrack);

  if (firstSimHitOfStart == nullptr or lastSimHitOfStart == nullptr or
      firstSimHitOfEnd   == nullptr or lastSimHitOfEnd   == nullptr) return NOT_A_CELL;

  if (not(firstSimHitOfStart->getFlightTime() < lastSimHitOfStart->getFlightTime() and
          lastSimHitOfStart->getFlightTime()  < firstSimHitOfEnd->getFlightTime()  and
          firstSimHitOfEnd->getFlightTime()   < lastSimHitOfEnd->getFlightTime())) return NOT_A_CELL;

  //check if there are two many superlayers to be traversed
  int nSuperlayers = m_mcLookUp.getNSuperLayersTraversed(startSegment, endSegment);
  if (abs(nSuperlayers) > 2) return NOT_A_CELL;

  return true;

}



CellWeight MCSegmentTripleFilter::isGoodSegmentTriple(const CDCSegmentTriple& segmentTriple)
{

  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStart();
  const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentTriple.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as start segment");
    return NOT_A_CELL;
  }
  if (ptrMiddleSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as middle segment");
    return NOT_A_CELL;
  }
  if (ptrEndSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as end segment");
    return NOT_A_CELL;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  CellWeight result = isGoodTriple(startSegment, middleSegment, endSegment);
  if (not isNotACell(result)) {
    setTrajectoryOf(segmentTriple);
  }
  return result;

}


CellState MCSegmentTripleFilter::isGoodTriple(const CDCAxialRecoSegment2D& startSegment,
                                              const CDCStereoRecoSegment2D& middleSegment,
                                              const CDCAxialRecoSegment2D& endSegment)
{


  if (startSegment.empty() or middleSegment.empty() or endSegment.empty()) return NOT_A_CELL;

  //check if the segments have the same trackid
  std::pair<float, ITrackType> efficiencyOfStart  = m_mcLookUp.getHighestEfficieny(startSegment);
  std::pair<float, ITrackType> efficiencyOfMiddle = m_mcLookUp.getHighestEfficieny(middleSegment);

  //end has been checked in the isGoodPair call
  std::pair<float, ITrackType> efficiencyOfEnd    = m_mcLookUp.getHighestEfficieny(endSegment);

  if (efficiencyOfMiddle.first < 0.5 or
      efficiencyOfStart.second != efficiencyOfMiddle.second or
      efficiencyOfEnd.second != efficiencyOfMiddle.second or
      efficiencyOfMiddle.second == INVALID_ITRACK
     ) {

    return NOT_A_CELL;

  }

  ITrackType iTrack = efficiencyOfStart.second;

  //check if the alignement is correct
  const CDCSimHit* lastSimHitOfStart = m_mcLookUp.getLastSimHit(startSegment, iTrack);

  const CDCSimHit* firstSimHitOfMiddle = m_mcLookUp.getFirstSimHit(middleSegment, iTrack);
  const CDCSimHit* lastSimHitOfMiddle = m_mcLookUp.getLastSimHit(middleSegment, iTrack);

  const CDCSimHit* firstSimHitOfEnd = m_mcLookUp.getFirstSimHit(endSegment, iTrack);

  if (lastSimHitOfStart  == nullptr or firstSimHitOfMiddle == nullptr or
      lastSimHitOfMiddle == nullptr or firstSimHitOfEnd    == nullptr) return NOT_A_CELL;

  if (not(lastSimHitOfStart->getFlightTime()   < firstSimHitOfMiddle->getFlightTime() and
          firstSimHitOfMiddle->getFlightTime() < lastSimHitOfMiddle->getFlightTime()  and
          lastSimHitOfMiddle->getFlightTime()  < firstSimHitOfEnd->getFlightTime())) {

    return NOT_A_CELL;

  }

  CellState cellWeight = startSegment.size() + middleSegment.size() + endSegment.size();
  return cellWeight;

}


void MCSegmentTripleFilter::setTrajectoryOf(const CDCSegmentTriple& segmentTriple)
{

  const CDCAxialRecoSegment2D* startSegmentPtr = segmentTriple.getStart();
  if (startSegmentPtr == nullptr) {
    B2WARNING("Start segment of segmentTriple is nullptr. Could not set fits.");
    return;
  }

  const CDCAxialRecoSegment2D& startSegment = *startSegmentPtr;

  std::pair<float, ITrackType> efficiencyOfStart = m_mcLookUp.getHighestEfficieny(startSegment);
  ITrackType iTrack = efficiencyOfStart.second;

  const CDCSimHit* firstSimHitOfStart = m_mcLookUp.getFirstSimHit(startSegment, iTrack);
  const Belle2::MCParticle* mcPartOfStart =  m_mcLookUp.getMCParticle(startSegment);


  if (firstSimHitOfStart == nullptr) {
    B2WARNING("First simhit of CDCRecoSegment is nullptr. Could not set fits.");
    return;
  }
  if (mcPartOfStart == nullptr) {
    B2WARNING("Major mc particle of CDCRecoSegment is nullptr. Could not set fits.");
    return;
  }

  Vector3D mom3D = firstSimHitOfStart->getMomentum();
  Vector3D pos3D = firstSimHitOfStart->getPosTrack();

  SignType chargeSign = sign(mcPartOfStart->getCharge());

  CDCTrajectory2D trajectory2D;
  trajectory2D.setStartPosMom2D(pos3D.xy(), mom3D.xy(), chargeSign);

  SignType settedChargeSign = trajectory2D.getChargeSign();

  //B2WARNING("settedChargeSign " << settedChargeSign);
  //B2WARNING("chargeSign " << chargeSign);

  if (chargeSign != settedChargeSign) {

    B2WARNING("Charge sign of mc particle is not the same as the one of the fit");

  }
  //cin >> settedChargeSign;

  CDCTrajectorySZ trajectorySZ(mom3D.z() / mom3D.polarR(), pos3D.z());

  segmentTriple.setTrajectory2D(trajectory2D);
  segmentTriple.setTrajectorySZ(trajectorySZ);

}


