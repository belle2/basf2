/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/sim/CDCSimpleSimulation.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/logging/Logger.h>

#include <TRandom.h>

#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCTrack CDCSimpleSimulation::simulate(const CDCTrajectory3D& trajectory3D)
{
  return std::move(simulate(std::vector<CDCTrajectory3D>(1, trajectory3D)).front());
}


vector<CDCTrack> CDCSimpleSimulation::simulate(const vector<CDCTrajectory3D>& trajectories3D)
{
  B2ASSERT("", m_wireHitTopology);

  vector<SimpleSimHit> simpleSimHits;
  const size_t nMCTracks = trajectories3D.size();

  for (size_t iMCTrack = 0;  iMCTrack < nMCTracks; ++iMCTrack) {
    const CDCTrajectory3D& trajectory3D = trajectories3D[iMCTrack];

    const UncertainHelix& localHelix = trajectory3D.getLocalHelix();
    const Vector3D& localOrigin = trajectory3D.getLocalOrigin();

    Helix globalHelix = localHelix;
    const FloatType arcLength2DOffset = globalHelix.passiveMoveBy(-localOrigin);
    vector<SimpleSimHit> simpleSimHitsForTrajectory = createHits(globalHelix, arcLength2DOffset);

    for (SimpleSimHit& simpleSimHit : simpleSimHitsForTrajectory) {
      simpleSimHit.iMCTrack = iMCTrack;
      simpleSimHits.push_back(simpleSimHit);
    }
  }

  // Sort the hits along side their wire hits
  std::stable_sort(simpleSimHits.begin(), simpleSimHits.end(),
  [](const SimpleSimHit & lhs, const SimpleSimHit & rhs) -> bool {
    return lhs.wireHit < rhs.wireHit;
  });

  // Discard multiple hits on the same wire up to the maximal exceeding the maximal desired number
  if (m_maxNHitOnWire > 0) {
    const CDCWire* lastWire = nullptr;
    size_t nSameWire = 0;
    const size_t maxNHitOnWire = m_maxNHitOnWire;

    auto exceedsMaxNHitOnWire =
    [&lastWire, &nSameWire, maxNHitOnWire](const SimpleSimHit & simpleSimHit) -> bool {

      if (&(simpleSimHit.wireHit.getWire()) == lastWire)
      {
        ++nSameWire;
      } else {
        nSameWire = 1;
        lastWire = &(simpleSimHit.wireHit.getWire());
      }
      return nSameWire > maxNHitOnWire ? true : false;
    };

    auto itLast = std::remove_if(simpleSimHits.begin(), simpleSimHits.end(), exceedsMaxNHitOnWire);
    simpleSimHits.erase(itLast, simpleSimHits.end());
  }

  // Write the created hits to the their storage place.
  CDCWireHitTopology& wireHitTopology = *m_wireHitTopology;
  {
    wireHitTopology.clear();
    for (SimpleSimHit& simpleSimHit : simpleSimHits) {
      wireHitTopology.m_wireHits.push_back(simpleSimHit.wireHit);
    }

    if (not wireHitTopology.m_wireHits.checkSorted()) {
      B2ERROR("Wire hits are not sorted after creation");
    }

    for (const CDCWireHit& wireHit : wireHitTopology.m_wireHits) {
      wireHitTopology.m_rlWireHits.push_back(CDCRLWireHit(&wireHit, LEFT));
      wireHitTopology.m_rlWireHits.push_back(CDCRLWireHit(&wireHit, RIGHT));
    }

    if (not wireHitTopology.m_rlWireHits.checkSorted()) {
      B2ERROR("Oriented wire hits are not sorted after creation");
    }
    // TODO: Decide if the EventMeta should be incremented after write.
  }

  // Now construct the tracks.
  vector<CDCTrack> mcTracks;
  mcTracks.resize(nMCTracks);
  const SortableVector<CDCWireHit>& wireHits = wireHitTopology.getWireHits();
  const size_t nWireHits = wireHits.size();

  for (size_t iWireHit = 0; iWireHit < nWireHits; ++iWireHit) {
    const CDCWireHit& wireHit = wireHits[iWireHit];
    const SimpleSimHit& simpleSimHit = simpleSimHits[iWireHit];

    CDCTrack& mcTrack = mcTracks[simpleSimHit.iMCTrack];

    const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(wireHit, simpleSimHit.rlInfo);

    CDCRecoHit3D recoHit3D(rlWireHit, simpleSimHit.pos3D, simpleSimHit.arcLength2D);
    mcTrack.push_back(recoHit3D);
  }

  /// Sort the hits by the order of their occurance
  for (CDCTrack& mcTrack : mcTracks) {
    mcTrack.sortByPerpS();
  }

  /// Assign mc trajectories to the tracks
  for (size_t iMCTrack = 0;  iMCTrack < nMCTracks; ++iMCTrack) {
    CDCTrack& mcTrack = mcTracks[iMCTrack];
    CDCTrajectory3D mcTrajectory = trajectories3D[iMCTrack];
    if (not mcTrack.empty()) {
      mcTrajectory.setLocalOrigin(mcTrack.front().getRecoPos3D());
      mcTrack.setStartTrajectory3D(mcTrajectory);
      mcTrajectory.setLocalOrigin(mcTrack.back().getRecoPos3D());
      mcTrack.setEndTrajectory3D(mcTrajectory);
    } else {
      mcTrack.setStartTrajectory3D(mcTrajectory);
      mcTrack.setEndTrajectory3D(mcTrajectory);
    }
  }

  return mcTracks;
}



std::vector<CDCSimpleSimulation::SimpleSimHit>
CDCSimpleSimulation::createHits(const Helix& globalHelix,
                                const FloatType& arcLength2DOffset) const
{

  vector<SimpleSimHit> simpleSimHits;

  CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  const double outerWallCylinderR = wireTopology.getOuterCylindricalR();

  const FloatType minR = globalHelix.minimalCylindricalR();
  const FloatType maxR = globalHelix.maximalCylindricalR();

  const double globalArcLength2DToOuterWall = globalHelix.arcLength2DToCylindricalR(outerWallCylinderR);
  const double localArcLength2DToOuterWall = arcLength2DOffset + globalArcLength2DToOuterWall;

  if (localArcLength2DToOuterWall < 0) {
    // Trajectory starts outside the CDC and initially flys away from it
    // Do not try to createHit hits for it
    B2WARNING("Simple simulation got trajectory outside CDC that moves away from the detector.");
    return simpleSimHits;
  }

  // Two dimensional arc length where the trajectory
  // * leaves the outer wall of the CDC or
  // * made a full circle (cut off for curlers)
  const bool isCurler = std::isnan(localArcLength2DToOuterWall);
  const double perimeterXY = globalHelix.perimeterXY();
  const double maxArcLength2D = isCurler ? fabs(perimeterXY) : localArcLength2DToOuterWall;

  if (isCurler) {
    B2INFO("Simulating curler");
  }

  for (const CDCWireLayer& wireLayer : wireTopology.getWireLayers()) {
    FloatType outerR = wireLayer.getOuterCylindricalR();
    FloatType innerR = wireLayer.getInnerCylindricalR();

    if ((maxR < innerR) or (outerR < minR)) {
      // Trajectory does not reaching the layer
      continue;
    }

    FloatType centerR = (min(outerR, maxR) + max(innerR, minR)) / 2;

    FloatType globalArcLength2D = globalHelix.arcLength2DToCylindricalR(centerR);
    FloatType localArcLength2D = arcLength2DOffset + globalArcLength2D;


    vector<SimpleSimHit> simpleSimHitsInLayer;
    if (localArcLength2D > 0 and localArcLength2D < maxArcLength2D) {

      Vector3D pos3DAtLayer = globalHelix.atArcLength2D(globalArcLength2D);
      const CDCWire& closestWire =  wireLayer.getClosestWire(pos3DAtLayer);

      simpleSimHitsInLayer = createHitsForLayer(closestWire, globalHelix, arcLength2DOffset);

      for (SimpleSimHit& simpleSimHit : simpleSimHitsInLayer) {
        if (simpleSimHit.arcLength2D < maxArcLength2D) {
          simpleSimHits.push_back(simpleSimHit);
        }
      }
    } else {
      B2INFO("Arc length to long");
    }

    bool oneSegment = outerR > maxR or innerR < minR;
    if (not oneSegment) {

      // Check the second branch for more hits
      double secondGlobalArcLength2D = -globalArcLength2D;
      double secondArcLength2DOffset = arcLength2DOffset;
      double secondLocalArcLength2D = secondArcLength2DOffset + secondGlobalArcLength2D;

      if (isCurler and secondLocalArcLength2D < 0) {
        secondLocalArcLength2D += perimeterXY;
        secondArcLength2DOffset += perimeterXY;
        secondGlobalArcLength2D += perimeterXY;
      }

      if (secondLocalArcLength2D > 0 and secondLocalArcLength2D < maxArcLength2D) {
        Vector3D pos3DAtLayer = globalHelix.atArcLength2D(secondGlobalArcLength2D);
        const CDCWire& closestWire =  wireLayer.getClosestWire(pos3DAtLayer);

        // Check again if the wire has been hit before
        bool wireAllReadyHit = false;
        for (const SimpleSimHit& simpleSimHit : simpleSimHits) {
          if (simpleSimHit.wireHit.hasWire(closestWire)) {
            wireAllReadyHit = true;
          }
        }
        if (not wireAllReadyHit) {
          std::vector<SimpleSimHit> secondSimpleSimHitsInLayer =
            createHitsForLayer(closestWire, globalHelix, arcLength2DOffset);

          for (SimpleSimHit& simpleSimHit : secondSimpleSimHitsInLayer) {
            if (simpleSimHit.arcLength2D < maxArcLength2D) {
              simpleSimHits.push_back(simpleSimHit);
            }
          }
        }
      }
    }
  }

  return simpleSimHits;
}

std::vector<CDCSimpleSimulation::SimpleSimHit>
CDCSimpleSimulation::createHitsForLayer(const CDCWire& nearWire,
                                        const Helix& globalHelix,
                                        const FloatType& arcLength2DOffset) const
{
  std::vector<SimpleSimHit> result;

  SimpleSimHit simpleSimHit = createHitForCell(nearWire, globalHelix, arcLength2DOffset);
  if (not std::isnan(simpleSimHit.wireHit.getRefDriftLength())) {
    result.push_back(simpleSimHit);
  }

  /// Iter counter clockwise for more hits
  const CDCWire* ccwWire = nearWire.getNeighborCCW();
  while (true) {
    SimpleSimHit simpleSimHitForWire = createHitForCell(*ccwWire, globalHelix, arcLength2DOffset);
    if (std::isnan(simpleSimHitForWire.arcLength2D) or
        std::isnan(simpleSimHitForWire.trueDriftLength)) {
      break;
    }
    result.push_back(simpleSimHitForWire);
    ccwWire = ccwWire->getNeighborCCW();
  }

  /// Iter clockwise for more hits
  const CDCWire* cwWire = nearWire.getNeighborCW();
  while (true) {
    SimpleSimHit simpleSimHitForWire = createHitForCell(*cwWire, globalHelix, arcLength2DOffset);
    if (std::isnan(simpleSimHitForWire.arcLength2D) or
        std::isnan(simpleSimHitForWire.trueDriftLength)) {
      break;
    }
    result.push_back(simpleSimHitForWire);
    cwWire = cwWire->getNeighborCW();
  }

  return result;
}


CDCSimpleSimulation::SimpleSimHit
CDCSimpleSimulation::createHitForCell(const CDCWire& wire,
                                      const Helix& globalHelix,
                                      const FloatType& arcLength2DOffset) const
{
  double arclength2D = globalHelix.arcLength2DToXY(wire.getRefPos2D());
  if ((arclength2D + arcLength2DOffset) < 0) {
    arclength2D += globalHelix.perimeterXY();
  }

  Vector3D pos3D = globalHelix.atArcLength2D(arclength2D);

  // Iterate the extrapolation once more to the stereo shifted position.
  const Vector2D correctedWirePos = wire.getWirePos2DAtZ(pos3D.z());
  double correctedArcLength2D = globalHelix.arcLength2DToXY(correctedWirePos);

  if ((correctedArcLength2D + arcLength2DOffset) < 0) {
    correctedArcLength2D += globalHelix.perimeterXY();
  }

  Vector3D correctedPos3D = globalHelix.atArcLength2D(correctedArcLength2D);

  const double trueDriftLength = wire.getDriftLength(correctedPos3D);
  const double smearedDriftLength = trueDriftLength + gRandom->Gaus(0, m_driftLengthSigma) * 0;

  double delayTime = getEventTime();
  if (m_addTOFDelay) {
    double arcLength3D = std::hypot(1, globalHelix.tanLambda()) * (correctedArcLength2D + arcLength2DOffset);
    delayTime += arcLength3D / Const::speedOfLight;
  }

  if (m_addInWireSignalDelay) {
    double backwardZ = wire.getBackwardZ();
    // Position where wire has been hit
    Vector3D wirePos = wire.getClosest(correctedPos3D);
    double distanceToBack = (wirePos.z() - backwardZ) * hypot(1, wire.getSkew() * wire.getRefCylindricalR());

    delayTime += distanceToBack / m_propSpeed;
  }

  double measuredDriftLength = smearedDriftLength + delayTime * m_driftSpeed;

  RightLeftInfo rlInfo = globalHelix.circleXY().isRightOrLeft(correctedWirePos);

  // if (not std::isnan(trueDriftLength)){
  //   B2INFO("Delay time " << delayTime);
  //   B2INFO("True dirft length " << trueDriftLength);
  //   B2INFO("Measured drift length " << measuredDriftLength);
  //   B2INFO("Absolute deviation " << measuredDriftLength - trueDriftLength);
  //   B2INFO("Relative deviation " << (measuredDriftLength / trueDriftLength - 1) * 100 << "%");
  // }

  return SimpleSimHit{
    CDCWireHit(wire.getWireID(), measuredDriftLength, m_driftLengthVariance),
    0,
    rlInfo,
    correctedPos3D,
    correctedArcLength2D,
    trueDriftLength
  };
}

