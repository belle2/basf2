/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorMCTruth.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/dataobjects/CDCHit.h>

#include <TRandom.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SegmentCreatorMCTruth::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "reconstructedDriftLength"),
                                m_param_reconstructedDriftLength,
                                "Switch to assign the reconstructed drift length to each hit, "
                                "as it can be estimated from two dimensional information only.",
                                m_param_reconstructedDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "reconstructedPositions"),
                                m_param_reconstructedPositions,
                                "Switch to reconstruct the positions in the segments "
                                "immitating the facet ca picking up all correct hits.",
                                m_param_reconstructedPositions);
}

std::string SegmentCreatorMCTruth::getDescription()
{
  return "Constructs segments from wire hits using the mc truth information.";
}


void SegmentCreatorMCTruth::initialize()
{
  CDCMCManager::getInstance().requireTruthInformation();
  Super::initialize();
}


void SegmentCreatorMCTruth::beginEvent()
{
  CDCMCManager::getInstance().fill();
  Super::beginEvent();
}

void SegmentCreatorMCTruth::apply(const std::vector<CDCWireHit>& inputWireHits,
                                  std::vector<CDCSegment2D>& outputSegments)
{
  const CDCMCTrackStore& mcTrackStore = CDCMCTrackStore::getInstance();
  const CDCSimHitLookUp& simHitLookUp = CDCSimHitLookUp::getInstance();

  using CDCHitVector = CDCMCTrackStore::CDCHitVector;

  const std::map<ITrackType, std::vector<CDCHitVector>>& mcSegmentsByMCParticleIdx =
                                                       mcTrackStore.getMCSegmentsByMCParticleIdx();

  std::size_t nSegments = 0;
  for (const std::pair<ITrackType, std::vector<CDCHitVector>>& mcSegmentsAndMCParticleIdx : mcSegmentsByMCParticleIdx) {
    const std::vector<CDCHitVector>& mcSegments = mcSegmentsAndMCParticleIdx.second;
    nSegments += mcSegments.size();
  }

  outputSegments.reserve(outputSegments.size() + nSegments);
  for (const std::pair<ITrackType, std::vector<CDCHitVector>>& mcSegmentsAndMCParticleIdx : mcSegmentsByMCParticleIdx) {

    const std::vector<CDCHitVector>& mcSegments = mcSegmentsAndMCParticleIdx.second;
    for (const CDCHitVector& mcSegment : mcSegments) {
      outputSegments.push_back(CDCSegment2D());
      CDCSegment2D& segment2D = outputSegments.back();
      for (const CDCHit* ptrHit : mcSegment) {
        const CDCWireHit* wireHit = simHitLookUp.getWireHit(ptrHit, inputWireHits);
        if (not wireHit) continue;

        CDCRecoHit2D recoHit2D = simHitLookUp.getClosestPrimaryRecoHit2D(ptrHit, inputWireHits);
        segment2D.push_back(recoHit2D);
      }
      if (segment2D.size() < 3) outputSegments.pop_back();
    }
  }

  CDC::RealisticTDCCountTranslator tdcCountTranslator;
  const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();
  for (CDCSegment2D& segment : outputSegments) {
    for (CDCRecoHit2D& recoHit2D : segment) {
      Vector2D flightDirection = recoHit2D.getFlightDirection2D();
      Vector2D recoPos2D = recoHit2D.getRecoPos2D();
      double alpha = recoPos2D.angleWith(flightDirection);

      const CDCWire& wire = recoHit2D.getWire();
      const CDCHit* hit = recoHit2D.getWireHit().getHit();
      const bool rl = recoHit2D.getRLInfo() == ERightLeft::c_Right;

      double driftLength = recoHit2D.getRefDriftLength();
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
      recoHit2D.setRefDriftLength(driftLength, snapRecoPos);
    }
  }

  if (m_param_reconstructedPositions) {
    for (CDCSegment2D& segment : outputSegments) {
      if (segment.size() > 1) {
        CDCRLWireHitSegment rlWireHitSegment = segment.getRLWireHitSegment();
        segment = CDCSegment2D::reconstructUsingFacets(rlWireHitSegment);
      }
    }
  }

  for (CDCSegment2D& segment : outputSegments) {
    segment.receiveISuperCluster();
  }
  std::sort(outputSegments.begin(), outputSegments.end());
}
