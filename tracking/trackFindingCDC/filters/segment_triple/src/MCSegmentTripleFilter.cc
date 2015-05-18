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

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentTripleFilter::MCSegmentTripleFilter(bool allowReverse) :
  m_param_allowReverse(allowReverse),
  m_mcAxialSegmentPairFilter(allowReverse)
{
}


void MCSegmentTripleFilter::clear()
{
  m_mcAxialSegmentPairFilter.clear();
}



void MCSegmentTripleFilter::initialize()
{
  m_mcAxialSegmentPairFilter.initialize();
}



void MCSegmentTripleFilter::terminate()
{
  m_mcAxialSegmentPairFilter.terminate();
}


void MCSegmentTripleFilter::setParameter(const std::string& key, const std::string& value)
{
  if (key == "symmetric") {
    if (value == "true") {
      m_param_allowReverse = true;
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else if (value == "false") {
      m_param_allowReverse = false;
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else {
      Super::setParameter(key, value);
    }
  } else {
    Super::setParameter(key, value);
  }
}

std::map<std::string, std::string> MCSegmentTripleFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["symmetric"] =  "Accept the segment triple if the reverse segment triple is correct "
                      "preserving the progagation reversal symmetry on this level of detail."
                      "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
}

bool MCSegmentTripleFilter::needsTruthInformation()
{
  return true;
}



CellWeight MCSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
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
  CellWeight pairWeight =  m_mcAxialSegmentPairFilter(segmentTriple);
  if (isNotACell(pairWeight)) return NOT_A_CELL;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  ForwardBackwardInfo startToMiddleFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrMiddleSegment);
  if (startToMiddleFBInfo == INVALID_INFO) return NOT_A_CELL;

  ForwardBackwardInfo middleToEndFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrMiddleSegment, ptrEndSegment);
  if (middleToEndFBInfo == INVALID_INFO) return NOT_A_CELL;


  if (startToMiddleFBInfo != middleToEndFBInfo) return NOT_A_CELL;


  if ((startToMiddleFBInfo == FORWARD and middleToEndFBInfo == FORWARD) or
      (m_param_allowReverse and startToMiddleFBInfo == BACKWARD and middleToEndFBInfo == BACKWARD)) {

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

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  CDCTrajectory3D trajectory3D = mcSegmentLookUp.getTrajectory3D(ptrStartSegment);

  segmentTriple.setTrajectory2D(trajectory3D.getTrajectory2D());
  segmentTriple.setTrajectorySZ(trajectory3D.getTrajectorySZ());

}
