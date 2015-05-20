/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segment_pair/MCSegmentPairFilter.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void MCSegmentPairFilter::setParameter(const std::string& key, const std::string& value)
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

std::map<std::string, std::string> MCSegmentPairFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["symmetric"] =  "Accept the axial stereo segment pair if the reverse axial stereo segment pair is correct "
                      "preserving the progagation reversal symmetry on this level of detail."
                      "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
}

bool MCSegmentPairFilter::needsTruthInformation()
{
  return true;
}


CellWeight MCSegmentPairFilter::operator()(const CDCSegmentPair& segmentPair)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentPair.getStartSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentPair.getEndSegment();

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

    if (abs(startNPassedSuperLayers - endNPassedSuperLayers) > 1) return NOT_A_CELL;

    // Do fits
    CDCTrajectory3D mcTrajectory3D = mcSegmentLookUp.getTrajectory3D(ptrStartSegment);
    segmentPair.setTrajectory3D(mcTrajectory3D);

    return startSegment.size() + endSegment.size();
  }

  return NOT_A_CELL;

}
