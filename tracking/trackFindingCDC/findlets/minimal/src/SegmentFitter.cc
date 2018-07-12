/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBFieldUtil.h>
#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string SegmentFitter::getDescription()
{
  return "Fits each segment with a selectable method";
}

void SegmentFitter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "karimakiFit"),
                                m_param_karimakiFit,
                                "Switch to select Karimaki method for fitting instead of Riemann fit",
                                m_param_karimakiFit);

  moduleParamList->addParameter(prefixed(prefix, "fitPos"),
                                m_param_fitPosString,
                                "Positional information of the hits to be used in the fit. "
                                "Options are 'recoPos', 'rlDriftCircle', 'wirePos'.",
                                m_param_fitPosString);

  moduleParamList->addParameter(prefixed(prefix, "fitVariance"),
                                m_param_fitVarianceString,
                                "Positional information of the hits to be used in the fit. "
                                "Options are 'unit', 'driftLength', 'pseudo', 'proper'.",
                                m_param_fitVarianceString);

  moduleParamList->addParameter(prefixed(prefix, "updateDriftLength"),
                                m_param_updateDriftLength,
                                "Switch to reestimate the drift length",
                                m_param_updateDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "updateRecoPos"),
                                m_param_updateRecoPos,
                                "Switch to reestimate the position and right left passage information",
                                m_param_updateRecoPos);

  m_driftLengthEstimator.exposeParameters(moduleParamList, prefix);
}


void SegmentFitter::initialize()
{
  Super::initialize();
  if (m_param_fitPosString != std::string("")) {
    try {
      m_fitPos = getFitPos(m_param_fitPosString);
    } catch (std::invalid_argument& e) {
      B2ERROR("Unexpected fitPos parameter : '" << m_param_fitPosString);
    }
  }

  if (m_param_fitVarianceString != std::string("")) {
    try {
      m_fitVariance = getFitVariance(m_param_fitVarianceString);
    } catch (std::invalid_argument& e) {
      B2ERROR("Unexpected fitVariance parameter : '" << m_param_fitVarianceString);
    }
  }

  if (m_param_karimakiFit) {
    if (m_fitPos != EFitPos::c_RecoPos) {
      B2WARNING("Karimaki fitter only works with the reconstructed position as input.");
    }
    m_fitPos = EFitPos::c_RecoPos;
  }
}

void SegmentFitter::apply(std::vector<CDCSegment2D>& outputSegments)
{
  // Update the drift length
  if (m_param_updateDriftLength) {
    for (CDCSegment2D& segment : outputSegments) {
      m_driftLengthEstimator.updateDriftLength(segment);
    }
  }

  for (const CDCSegment2D& segment : outputSegments) {
    CDCObservations2D observations2D(m_fitPos, m_fitVariance);
    observations2D.appendRange(segment);

    if (m_param_karimakiFit or observations2D.size() < 4) {
      // Karimaki only works with the reconstructed position
      if (m_fitPos != EFitPos::c_RecoPos) {
        observations2D.clear();
        observations2D.setFitPos(EFitPos::c_RecoPos);
        observations2D.appendRange(segment);
      }
      CDCTrajectory2D trajectory2D = m_karimakiFitter.fit(observations2D);
      segment.setTrajectory2D(trajectory2D);
    } else {

      CDCTrajectory2D trajectory2D = m_riemannFitter.fit(observations2D);
      segment.setTrajectory2D(trajectory2D);
    }
  }

  if (m_param_updateRecoPos) {
    for (CDCSegment2D& segment : outputSegments) {
      const CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
      if (not trajectory2D.isFitted()) continue;
      int nRLChanges = 0;
      for (CDCRecoHit2D& recoHit2D : segment) {
        ERightLeft rlInfo = trajectory2D.isRightOrLeft(recoHit2D.getRefPos2D());
        if (rlInfo != recoHit2D.getRLInfo()) ++nRLChanges;
        recoHit2D.setRLInfo(rlInfo);
        const CDCRLWireHit& rlWireHit = recoHit2D.getRLWireHit();
        Vector2D recoPos2D = rlWireHit.reconstruct2D(trajectory2D);
        recoHit2D.setRecoPos2D(recoPos2D);
      }
      if (nRLChanges > 0) B2DEBUG(100, "RL changes " << nRLChanges);
    }
  }
}
