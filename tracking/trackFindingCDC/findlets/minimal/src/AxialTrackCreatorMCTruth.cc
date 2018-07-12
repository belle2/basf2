/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorMCTruth.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <TRandom.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void AxialTrackCreatorMCTruth::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "reconstructedDriftLength"),
                                m_param_reconstructedDriftLength,
                                "Switch to assign the reconstructed drift length to each hit, "
                                "as it can be estimated from two dimensional information only.",
                                m_param_reconstructedDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "reconstructedPositions"),
                                m_param_reconstructedPositions,
                                "Switch to reconstruct the positions in the tracks "
                                "immitating the legendre finder.",
                                m_param_reconstructedPositions);

  moduleParamList->addParameter(prefixed(prefix, "fit"),
                                m_param_fit,
                                "Fit the track instead of forwarding the mc truth fit information",
                                m_param_fit);

  moduleParamList->addParameter(prefixed(prefix, "useOnlyBeforeTOP"),
                                m_param_useOnlyBeforeTOP,
                                "Cut tracks after the last layer of the CDC has been reached, "
                                "assuming the tracks left the CDC.",
                                m_param_useOnlyBeforeTOP);
}

std::string AxialTrackCreatorMCTruth::getDescription()
{
  return "Constructs tracks from wire hits using the mc truth information.";
}


void AxialTrackCreatorMCTruth::initialize()
{
  CDCMCManager::getInstance().requireTruthInformation();
  Super::initialize();
}


void AxialTrackCreatorMCTruth::beginEvent()
{
  CDCMCManager::getInstance().fill();
  Super::beginEvent();
}

void AxialTrackCreatorMCTruth::apply(const std::vector<CDCWireHit>& inputWireHits,
                                     std::vector<CDCTrack>& outputAxialTracks)
{
  const CDCMCTrackStore& mcTrackStore = CDCMCTrackStore::getInstance();
  const CDCSimHitLookUp& simHitLookUp = CDCSimHitLookUp::getInstance();

  using CDCHitVector = CDCMCTrackStore::CDCHitVector;

  const std::map<ITrackType, CDCHitVector>& mcTracksByMCParticleIdx =
    mcTrackStore.getMCTracksByMCParticleIdx();

  std::size_t nAxialTracks = mcTracksByMCParticleIdx.size();
  outputAxialTracks.reserve(outputAxialTracks.size() + nAxialTracks);

  for (const std::pair<ITrackType, CDCHitVector>& mcTracksAndMCParticleIdx : mcTracksByMCParticleIdx) {

    const CDCHitVector& mcTrack = mcTracksAndMCParticleIdx.second;

    outputAxialTracks.push_back(CDCTrack());
    CDCTrack& axialTrack = outputAxialTracks.back();
    bool reachedOuterMostLayer = false;
    for (const CDCHit* ptrHit : mcTrack) {

      if (m_param_useOnlyBeforeTOP) {
        // Cut tracks after the outermost layer has been reached and
        // the track starts to go inwards again.
        // But let all hits in the outermost layer survive.
        if (ptrHit->getISuperLayer() == 8 and ptrHit->getILayer() == 5) {
          reachedOuterMostLayer = true;
        }
        if (reachedOuterMostLayer and ptrHit->getILayer() != 5) {
          break;
        }
      }

      const CDCWireHit* wireHit = simHitLookUp.getWireHit(ptrHit, inputWireHits);
      if (not wireHit) continue;

      CDCRecoHit2D recoHit2D = simHitLookUp.getClosestPrimaryRecoHit2D(ptrHit, inputWireHits);
      if (not recoHit2D.isAxial()) continue;

      CDCRecoHit3D recoHit3D(recoHit2D.getRLWireHit(), {recoHit2D.getRecoPos2D(), 0}, NAN);
      axialTrack.push_back(recoHit3D);
    }

    // Discard short tracks
    if (axialTrack.size() < 5) outputAxialTracks.pop_back();
  }

  CDC::RealisticTDCCountTranslator tdcCountTranslator;
  const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();
  for (CDCTrack& track : outputAxialTracks) {
    for (CDCRecoHit3D& recoHit3D : track) {
      Vector2D recoPos2D = recoHit3D.getRecoPos2D();
      Vector2D flightDirection = recoHit3D.getFlightDirection2D();
      double alpha = recoPos2D.angleWith(flightDirection);

      const CDCWire& wire = recoHit3D.getWire();
      const CDCHit* hit = recoHit3D.getWireHit().getHit();
      const bool rl = recoHit3D.getRLInfo() == ERightLeft::c_Right;

      double driftLength = std::fabs(recoHit3D.getSignedRecoDriftLength());
      if (m_param_reconstructedDriftLength) {
        // Setup the drift length such that only information
        // that would be present in two dimensional reconstruction is used
        const double beta = 1;
        double flightTimeEstimate = 0;
        flightTimeEstimator.getFlightTime2D(recoPos2D, alpha, beta);

        driftLength =
          tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                            wire.getWireID(),
                                            flightTimeEstimate,
                                            rl,
                                            wire.getRefZ(),
                                            alpha);

        // As the legendre finder does not reestimate the drift length
        // We simply set it to the reference drift length for now.
        // Use version above once the reestimation comes along.
        driftLength = recoHit3D.getWireHit().getRefDriftLength();

      } else {
        // In case the true drift length should be kept at least smear it with its variance.
        double driftLengthVariance = tdcCountTranslator.getDriftLengthResolution(driftLength,
                                     wire.getWireID(),
                                     rl,
                                     wire.getRefZ(),
                                     alpha);

        driftLength += gRandom->Gaus(0, std::sqrt(driftLengthVariance));
      }
      bool snapRecoPos = true;
      recoHit3D.setRecoDriftLength(driftLength, snapRecoPos);
    }
  }

  if (m_param_fit) {
    CDCKarimakiFitter fitter;
    for (CDCTrack& track : outputAxialTracks) {
      CDCTrajectory2D trajectory2D = fitter.fit(track);
      trajectory2D.setLocalOrigin(Vector2D(0.0, 0.0));
      track.setStartTrajectory3D({trajectory2D, CDCTrajectorySZ::basicAssumption()});
    }
  } else {
    const CDCMCTrackLookUp& mcTrackLookUp = CDCMCTrackLookUp::getInstance();
    for (CDCTrack& track : outputAxialTracks) {
      CDCTrajectory3D trajectory3D = mcTrackLookUp.getTrajectory3D(&track);
      CDCTrajectory2D trajectory2D = trajectory3D.getTrajectory2D();
      track.setStartTrajectory3D({trajectory2D, CDCTrajectorySZ::basicAssumption()});
    }
  }

  if (m_param_reconstructedPositions) {
    for (CDCTrack& track : outputAxialTracks) {
      AxialTrackUtil::normalizeTrack(track);
    }
  }
}
