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

#include <TDatabasePDG.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCSegmentLookUp.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

MCSegmentTripleFilter::MCSegmentTripleFilter()
{
}



MCSegmentTripleFilter::~MCSegmentTripleFilter()
{
}



void MCSegmentTripleFilter::clear()
{
  m_mcAxialAxialSegmentPairFilter.clear();
}



void MCSegmentTripleFilter::initialize()
{
  m_mcAxialAxialSegmentPairFilter.initialize();
}



void MCSegmentTripleFilter::terminate()
{
  m_mcAxialAxialSegmentPairFilter.terminate();
}



CellWeight MCSegmentTripleFilter::isGoodSegmentTriple(const CDCSegmentTriple& segmentTriple, bool allowBackward) const
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

  /// Recheck the axial axial compatability
  CellWeight pairWeight =  m_mcAxialAxialSegmentPairFilter.isGoodAxialAxialSegmentPair(segmentTriple, allowBackward);
  if (isNotACell(pairWeight)) return NOT_A_CELL;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  ForwardBackwardInfo startToMiddleFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrMiddleSegment);
  if (startToMiddleFBInfo == INVALID_INFO) return NOT_A_CELL;

  ForwardBackwardInfo middleToEndFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrMiddleSegment, ptrEndSegment);
  if (middleToEndFBInfo == INVALID_INFO) return NOT_A_CELL;


  if (startToMiddleFBInfo != middleToEndFBInfo) return NOT_A_CELL;


  if ((startToMiddleFBInfo == FORWARD and middleToEndFBInfo == FORWARD) or
      (allowBackward and startToMiddleFBInfo == BACKWARD and middleToEndFBInfo == BACKWARD)) {

    // Do fits
    setTrajectoryOf(segmentTriple);

    CellState cellWeight = startSegment.size() + middleSegment.size() + endSegment.size();
    return cellWeight;

  }

  return NOT_A_CELL;

}



void MCSegmentTripleFilter::setTrajectoryOf(const CDCSegmentTriple& segmentTriple) const
{
  if (segmentTriple.getTrajectorySZ().isFitted()) {
    // SZ trajectory has been fitted before. Skipping
    // A fit sz trajectory implies a 2d trajectory to be fitted, but not the other way around
    return;
  }

  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStart();
  if (not ptrStartSegment) {
    B2WARNING("Start segment of segmentTriple is nullptr. Could not set fits.");
    return;
  }

  //const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();
  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  const CDCHit* ptrFirstHit = mcSegmentLookUp.getFirstHit(ptrStartSegment);

  const CDCSimHit* ptrPrimarySimHit = mcHitLookUp.getClosestPrimarySimHit(ptrFirstHit);


  if (not ptrPrimarySimHit) {
    // If there is no primary SimHit simply use the secondary simhit as reference
    ptrPrimarySimHit = mcHitLookUp.getSimHit(ptrFirstHit);
    if (not ptrPrimarySimHit) {
      B2WARNING("First simhit of CDCRecoSegment is nullptr. Could not set fits.");
      return;
    }
  }

  const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

  Vector3D mom3D = primarySimHit.getMomentum();
  Vector3D pos3D = primarySimHit.getPosTrack();



  int pdgCode = primarySimHit.getPDGCode();
  const TParticlePDG* ptrTPDGParticle = TDatabasePDG::Instance()->GetParticle(pdgCode);

  if (not ptrTPDGParticle) {
    B2WARNING("No particle for PDG code " << pdgCode << ". Could not set fits");
  }

  const TParticlePDG& tPDGParticle = *ptrTPDGParticle;

  double charge = tPDGParticle.Charge() / 3.0;

  SignType chargeSign = sign(charge);



  CDCTrajectory2D trajectory2D;
  trajectory2D.setStartPosMom2D(pos3D.xy(), mom3D.xy(), chargeSign);

  SignType settedChargeSign = trajectory2D.getChargeSign();

  if (chargeSign != settedChargeSign) {
    B2WARNING("Charge sign of mc particle is not the same as the one of the fit");
  }



  CDCTrajectorySZ trajectorySZ(mom3D.z() / mom3D.polarR(), pos3D.z());

  segmentTriple.setTrajectory2D(trajectory2D);
  segmentTriple.setTrajectorySZ(trajectorySZ);

}


