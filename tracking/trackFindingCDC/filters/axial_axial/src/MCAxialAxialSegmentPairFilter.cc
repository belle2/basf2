/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCAxialAxialSegmentPairFilter.h"

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCAxialAxialSegmentPairFilter::MCAxialAxialSegmentPairFilter(bool allowReverse) :
  m_param_allowReverse(allowReverse)
{
}


void MCAxialAxialSegmentPairFilter::setParameter(const std::string& key, const std::string& value)
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

std::map<std::string, std::string> MCAxialAxialSegmentPairFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["symmetric"] =  "Accept the axial axial segment pair if the reverse axial axial segment pair is correct "
                      "preserving the progagation reversal symmetry on this level of detail."
                      "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
}

bool MCAxialAxialSegmentPairFilter::needsTruthInformation()
{
  return true;
}

CellWeight MCAxialAxialSegmentPairFilter::operator()(const CDCAxialAxialSegmentPair& axialAxialSegmentPair)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  ForwardBackwardInfo pairFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrEndSegment);
  if (pairFBInfo == INVALID_INFO) return NOT_A_CELL;

  if (pairFBInfo == FORWARD or (m_param_allowReverse and pairFBInfo == BACKWARD)) {
    // Final check for the distance between the segment
    Index startNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(ptrStartSegment);
    if (startNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    Index endNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(ptrEndSegment);
    if (endNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    if (abs(startNPassedSuperLayers - endNPassedSuperLayers) > 2) return NOT_A_CELL;

    //do fits?
    return startSegment.size() + endSegment.size();
  }

  return NOT_A_CELL;

}
