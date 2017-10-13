/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/EventDataPlotter.h>

#include <tracking/trackFindingCDC/display/SVGPrimitivePlotter.h>
#include <tracking/trackFindingCDC/display/BoundingBox.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCTangent.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/geometry/Circle2D.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <TMatrixDSym.h>

#include <cmath>
#include <exception>

using namespace Belle2;
using namespace TrackFindingCDC;

EventDataPlotter::EventDataPlotter(bool animate, bool forwardFade)
  : m_ptrPrimitivePlotter(new SVGPrimitivePlotter(
                            AttributeMap{{"stroke", "orange"}, {"stroke-width", "0.55"}, {"fill", "none"}}))
, m_animate(animate)
, m_forwardFade(forwardFade)
{
}

EventDataPlotter::EventDataPlotter(std::unique_ptr<PrimitivePlotter> ptrPrimitivePlotter,
                                   bool animate,
                                   bool forwardFade)
  : m_ptrPrimitivePlotter(std::move(ptrPrimitivePlotter))
  , m_animate(animate)
  , m_forwardFade(forwardFade)
{
  B2ASSERT("EventDataPlotter initialized with nullptr. Using default backend SVGPrimitivePlotter.",
           m_ptrPrimitivePlotter);
}

EventDataPlotter::EventDataPlotter(const EventDataPlotter& eventDataPlotter)
  : m_ptrPrimitivePlotter(eventDataPlotter.m_ptrPrimitivePlotter->clone())
  , m_animate(eventDataPlotter.m_animate)
  , m_forwardFade(eventDataPlotter.m_forwardFade)
{
}

const std::string EventDataPlotter::save(const std::string& fileName)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.save(fileName);
  } else {
    return "";
  }
}

void EventDataPlotter::clear()
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.clear();
  }
}

BoundingBox EventDataPlotter::getBoundingBox() const
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.getBoundingBox();
  } else {
    return BoundingBox(0, 0, 0, 0);
  }
}

void EventDataPlotter::setBoundingBox(const BoundingBox& boundingBox)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.setBoundingBox(boundingBox);
  }
}

float EventDataPlotter::getCanvasWidth() const
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.getCanvasWidth();
  } else {
    return NAN;
  }
}

float EventDataPlotter::getCanvasHeight() const
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.getCanvasHeight();
  } else {
    return NAN;
  }
}

void EventDataPlotter::setCanvasWidth(float width)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.setCanvasWidth(width);
  }
}

void EventDataPlotter::setCanvasHeight(float height)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.setCanvasHeight(height);
  }
}

void EventDataPlotter::startGroup(const AttributeMap& attributeMap)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.startGroup(attributeMap);
  }
}

void EventDataPlotter::endGroup()
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.endGroup();
  }
}

void EventDataPlotter::startAnimationGroup(const Belle2::CDCSimHit& simHit)
{
  // In case the event should be animated
  // uncover the group of elements at the time of flight of the CDCSimHit.
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  if (m_animate) {
    float tof = simHit.getFlightTime();
    AttributeMap groupAttributeMap{{"_showAt", getAnimationTimeFromNanoSeconds(tof)}};
    primitivePlotter.startGroup(groupAttributeMap);

  } else {
    primitivePlotter.startGroup();
  }
}

void EventDataPlotter::startAnimationGroup(const Belle2::CDCHit* ptrHit)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  if (m_animate) {
    if (ptrHit) {
      const CDCHit& hit = *ptrHit;
      const CDCSimHit* ptrSimHit = hit.getRelated<CDCSimHit>();
      if (ptrSimHit) {
        const CDCSimHit& simHit = *ptrSimHit;
        startAnimationGroup(simHit);
        return;
      }
    }
  }
  primitivePlotter.startGroup();
}

void EventDataPlotter::drawInteractionPoint()
{
  Vector2D center(0.0, 0.0);
  float radius = 1.0;

  const Circle2D interactionPoint(center, radius);

  AttributeMap attributeMap{{"fill", "black"}, {"stroke-width", "0"}};

  draw(interactionPoint, attributeMap);
}

void EventDataPlotter::drawInnerCDCWall(const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const CDCWireSuperLayer& wireSuperLayer = wireTopology.getWireSuperLayers().front();

  float centerX = 0.0;
  float centerY = 0.0;
  float innerR = wireSuperLayer.getInnerCylindricalR();

  primitivePlotter.drawCircle(centerX, centerY, innerR, attributeMap);
}

void EventDataPlotter::drawOuterCDCWall(const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const CDCWireSuperLayer& wireSuperLayer = wireTopology.getWireSuperLayers().back();

  float centerX = 0.0;
  float centerY = 0.0;
  float outerR = wireSuperLayer.getOuterCylindricalR();

  primitivePlotter.drawCircle(centerX, centerY, outerR, attributeMap);
}

void EventDataPlotter::drawSuperLayerBoundaries(const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  for (const CDCWireSuperLayer& wireSuperLayer : wireTopology.getWireSuperLayers()) {
    float centerX = 0.0;
    float centerY = 0.0;
    float outerR = wireSuperLayer.getInnerCylindricalR();
    primitivePlotter.drawCircle(centerX, centerY, outerR, attributeMap);
  }
  drawOuterCDCWall(attributeMap);
}

void EventDataPlotter::drawLine(float startX,
                                float startY,
                                float endX,
                                float endY,
                                const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  primitivePlotter.drawLine(startX, startY, endX, endY, attributeMap);
}

/// --------------------- Draw Circle2D ------------------------
void EventDataPlotter::draw(const Circle2D& circle, AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  float radius = circle.radius();

  if (not attributeMap.count("fill") or attributeMap["fill"] != "") {
    if (attributeMap.count("stroke")) {
      attributeMap["fill"] = attributeMap["stroke"];
      attributeMap.erase("stroke");
    }
  }

  const Vector2D& pos = circle.center();

  float x = pos.x();
  float y = pos.y();

  primitivePlotter.drawCircle(x, y, radius, attributeMap);
}

/// --------------------- Draw CDCWire ------------------------
void EventDataPlotter::draw(const CDCWire& wire, const AttributeMap& attributeMap)
{
  const float wireRadius = 0.25;
  const Vector2D& refPos = wire.getRefPos2D();

  draw(Circle2D(refPos, wireRadius), attributeMap);
}

/// --------------------- Draw CDCWireSuperLayer ------------------------
void EventDataPlotter::draw(const CDCWireSuperLayer& wireSuperLayer,
                            const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  primitivePlotter.startGroup(attributeMap);
  for (const CDCWireLayer& wireLayer : wireSuperLayer) {
    for (const CDCWire& wire : wireLayer) {
      draw(wire);
    }
  }
  primitivePlotter.endGroup();
}

/// --------------------- Draw CDCWireTopology------------------------
void EventDataPlotter::draw(const CDCWireTopology& wireTopology, AttributeMap attributeMap)
{
  for (const CDCWireSuperLayer& wireSuperLayer : wireTopology.getWireSuperLayers()) {
    AttributeMap defaultSuperLayerAttributeMap{{"fill",
        wireSuperLayer.isAxial() ? "black" : "gray"
      },
      {"stroke", "none"}};

    AttributeMap superLayerAttributeMap(attributeMap);

    // Insert the values as defaults. Does not overwrite attributes with the same name.
    superLayerAttributeMap.insert(defaultSuperLayerAttributeMap.begin(),
                                  defaultSuperLayerAttributeMap.end());
    draw(wireSuperLayer, superLayerAttributeMap);
  }
}

/// --------------------- Draw CDCSimHit ------------------------
void EventDataPlotter::draw(const CDCSimHit& simHit, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  startAnimationGroup(simHit);

  // Draw hit position as a small circle
  TVector3 position = simHit.getPosTrack();
  float x = position.X();
  float y = position.Y();
  float radius = 0.2;

  primitivePlotter.drawCircle(x, y, radius, attributeMap);

  // Draw momentum as an arrow proportional to the transverse component of the momentum
  const float momentumToArrowLength = 1.5;

  TVector3 momentum = simHit.getMomentum();
  float endX = x + momentum.X() * momentumToArrowLength;
  float endY = y + momentum.Y() * momentumToArrowLength;

  primitivePlotter.drawArrow(x, y, endX, endY, attributeMap);

  primitivePlotter.endGroup();
}

/// --------------------- Draw CDCHit ------------------------
void EventDataPlotter::draw(const CDCHit& hit, const AttributeMap& attributeMap)
{
  CDCWireHit wireHit(&hit);
  draw(wireHit, attributeMap);
}

/// --------------------- Draw CDCWireHit ------------------------
void EventDataPlotter::draw(const CDCWireHit& wireHit, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  startAnimationGroup(wireHit.getHit());

  const Vector2D& refPos = wireHit.getRefPos2D();

  float x = refPos.x();
  float y = refPos.y();
  float radius = wireHit.getRefDriftLength();

  if (fabs(radius) < 100) {
    primitivePlotter.drawCircle(x, y, radius, attributeMap);
  }

  primitivePlotter.endGroup();
}

/// --------------------- Draw CDCRecoHit2D ------------------------
void EventDataPlotter::draw(const CDCRecoHit2D& recoHit2D, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCWireHit& wireHit = recoHit2D.getWireHit();

  startAnimationGroup(wireHit.getHit());

  const Vector2D& refPos2D = wireHit.getRefPos2D();
  const Vector2D& recoPos2D = recoHit2D.getRecoPos2D();

  float x = refPos2D.x();
  float y = refPos2D.y();
  float radius = wireHit.getRefDriftLength();
  primitivePlotter.drawCircle(x, y, radius, attributeMap);

  if (not recoPos2D.hasNAN()) {
    float supportPointRadius = 0.2;
    Circle2D supportPoint(recoPos2D, supportPointRadius);
    draw(supportPoint, attributeMap);
  }

  primitivePlotter.endGroup();
}

/// --------------------- Draw CDCRecoHit2D ------------------------
void EventDataPlotter::draw(const CDCTangent& tangent, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const Vector2D fromPos = tangent.getFromRecoPos2D();
  const float fromX = fromPos.x();
  const float fromY = fromPos.y();

  const Vector2D toPos = tangent.getToRecoPos2D();
  const float toX = toPos.x();
  const float toY = toPos.y();

  primitivePlotter.drawLine(fromX, fromY, toX, toY, attributeMap);

  float touchPointRadius = 0.015;
  const Circle2D fromTouchPoint(fromPos, touchPointRadius);
  draw(fromTouchPoint, attributeMap);

  const Circle2D toTouchPoint(toPos, touchPointRadius);
  draw(toTouchPoint, attributeMap);
}

void EventDataPlotter::draw(const Belle2::TrackFindingCDC::CDCRecoHit3D& recoHit3D,
                            const AttributeMap& attributeMap)
{
  draw(recoHit3D.getRecoHit2D(), attributeMap);
}

/// --------------------- Draw CDCTrajectory2D ------------------------
void EventDataPlotter::draw(const CDCTrajectory2D& trajectory2D, AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  AttributeMap defaultAttributeMap{};

  // Make the default color charge dependent
  int charge = trajectory2D.getChargeSign();
  if (charge > 0) {
    defaultAttributeMap["stroke"] = "red";
  } else if (charge < 0) {
    defaultAttributeMap["stroke"] = "blue";
  } else {
    defaultAttributeMap["stroke"] = "green";
  }

  // Add attributes if not present
  attributeMap.insert(defaultAttributeMap.begin(), defaultAttributeMap.end());

  Vector2D trajectoryExit = trajectory2D.getOuterExit();
  if (trajectoryExit.hasNAN()) {
    // Curlers do not leave the CDC
    // Stop the trajectory at the inner wall to be able to
    // see the start point
    trajectoryExit = trajectory2D.getInnerExit();
  }

  if (trajectory2D.getLocalCircle()->isCircle()) {
    if (trajectoryExit.hasNAN()) {
      // No exit point out of the cdc could be detected.
      // Draw full circle
      const float radius = trajectory2D.getLocalCircle()->absRadius();
      const Vector2D center = trajectory2D.getGlobalCircle().center();
      float centerX = center.x();
      float centerY = center.y();

      primitivePlotter.drawCircle(centerX, centerY, radius);

    } else {
      const float radius = trajectory2D.getLocalCircle()->absRadius();
      const Vector2D start = trajectory2D.getSupport();
      float startX = start.x();
      float startY = start.y();

      float endX = trajectoryExit.x();
      float endY = trajectoryExit.y();

      const int curvature = -charge;
      const bool sweepFlag = curvature > 0;

      // check if exit point is on the close or
      // on the far side of the circle
      const bool longArc = (trajectory2D.calcArcLength2D(trajectoryExit) > 0) ? false : true;
      primitivePlotter.drawCircleArc(startX,
                                     startY,
                                     endX,
                                     endY,
                                     radius,
                                     longArc,
                                     sweepFlag,
                                     attributeMap);
    }
  } else {
    // trajectory is a straight line
    if (trajectoryExit.hasNAN()) {
      B2WARNING("Could not compute point off exit in a straight line case.");
    } else {
      const Vector2D start = trajectory2D.getSupport();
      float startX = start.x();
      float startY = start.y();

      float endX = trajectoryExit.x();
      float endY = trajectoryExit.y();
      primitivePlotter.drawLine(startX, startY, endX, endY, attributeMap);
    }
  }
}

void EventDataPlotter::draw(const CDCWireHitCluster& wireHitCluster, const AttributeMap& attributeMap)
{
  drawRange(wireHitCluster, attributeMap);
}

void EventDataPlotter::draw(const CDCSegment2D& segment2D, const AttributeMap& attributeMap)
{
  if (m_forwardFade) {
    drawRangeWithFade(segment2D, attributeMap);
  } else {
    drawRange(segment2D, attributeMap);
  }
}

void EventDataPlotter::draw(const CDCSegment3D& segment3D, const AttributeMap& attributeMap)
{
  if (m_forwardFade) {
    drawRange(segment3D, attributeMap);
  } else {
    drawRange(segment3D, attributeMap);
  }
}

void EventDataPlotter::draw(const CDCAxialSegmentPair& axialSegmentPair,
                            const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCSegment2D* ptrFromSegment = axialSegmentPair.getStartSegment();
  const CDCSegment2D* ptrToSegment = axialSegmentPair.getEndSegment();

  if (not ptrFromSegment or not ptrToSegment) return;

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  const Vector2D& fromPos = fromSegment.back().getWire().getRefPos2D();
  const Vector2D& toPos = toSegment.front().getWire().getRefPos2D();

  if (fromPos.hasNAN()) {
    B2WARNING("Center of mass of first segment in a pair contains NAN values.");
    return;
  }

  if (toPos.hasNAN()) {
    B2WARNING("Center of mass of second segment in a pair contains NAN values.");
    return;
  }

  const float fromX = fromPos.x();
  const float fromY = fromPos.y();

  const float toX = toPos.x();
  const float toY = toPos.y();

  primitivePlotter.drawArrow(fromX, fromY, toX, toY, attributeMap);
}

void EventDataPlotter::draw(const CDCSegmentPair& segmentPair, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrToSegment = segmentPair.getToSegment();

  if (not ptrFromSegment or not ptrToSegment) return;

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  const Vector2D& fromPos = fromSegment.back().getWire().getRefPos2D();
  const Vector2D& toPos = toSegment.front().getWire().getRefPos2D();

  if (fromPos.hasNAN()) {
    B2WARNING("Center of mass of first segment in a pair contains NAN values.");
    return;
  }

  if (toPos.hasNAN()) {
    B2WARNING("Center of mass of second segment in a pair contains NAN values.");
    return;
  }

  const float fromX = fromPos.x();
  const float fromY = fromPos.y();

  const float toX = toPos.x();
  const float toY = toPos.y();

  primitivePlotter.drawArrow(fromX, fromY, toX, toY, attributeMap);
}

void EventDataPlotter::draw(const CDCSegmentTriple& segmentTriple, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCSegment2D* ptrStartSegment = segmentTriple.getStartSegment();
  const CDCSegment2D* ptrMiddleSegment = segmentTriple.getMiddleSegment();
  const CDCSegment2D* ptrEndSegment = segmentTriple.getEndSegment();

  if (not ptrStartSegment or not ptrMiddleSegment or not ptrEndSegment) return;

  const CDCSegment2D& startSegment = *ptrStartSegment;
  const CDCSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCSegment2D& endSegment = *ptrEndSegment;

  const Vector2D& startBackPos2D = startSegment.back().getRefPos2D();
  const Vector2D& middleFrontPos2D = middleSegment.front().getRefPos2D();
  const Vector2D& middleBackPos2D = middleSegment.back().getRefPos2D();
  const Vector2D& endFrontPos2D = endSegment.front().getRefPos2D();

  if (startBackPos2D.hasNAN()) {
    B2WARNING("Back position of start segment in a triple contains NAN values.");
    return;
  }

  if (middleFrontPos2D.hasNAN()) {
    B2WARNING("Front position of middle segment in a triple contains NAN values.");
    return;
  }

  if (middleBackPos2D.hasNAN()) {
    B2WARNING("Back position of middle segment in a triple contains NAN values.");
    return;
  }

  if (endFrontPos2D.hasNAN()) {
    B2WARNING("Front position of end segment in a triple contains NAN values.");
    return;
  }

  const float startBackX = startBackPos2D.x();
  const float startBackY = startBackPos2D.y();

  const float middleFrontX = middleFrontPos2D.x();
  const float middleFrontY = middleFrontPos2D.y();

  primitivePlotter.drawArrow(startBackX, startBackY, middleFrontX, middleFrontY, attributeMap);

  const float middleBackX = middleBackPos2D.x();
  const float middleBackY = middleBackPos2D.y();

  const float endFrontX = endFrontPos2D.x();
  const float endFrontY = endFrontPos2D.y();

  primitivePlotter.drawArrow(middleBackX, middleBackY, endFrontX, endFrontY, attributeMap);
}

void EventDataPlotter::draw(const CDCTrack& track, const AttributeMap& attributeMap)
{
  if (m_forwardFade) {
    drawRangeWithFade(track, attributeMap);
  } else {
    drawRange(track, attributeMap);
  }
}

void EventDataPlotter::draw(const RecoTrack& recoTrack, const AttributeMap& attributeMap)
{
  StoreArray<CDCHit> storedHits;

  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  primitivePlotter.startGroup(attributeMap);
  for (const CDCHit* ptrHit : recoTrack.getCDCHitList()) {
    if (ptrHit) {
      const CDCHit& hit = *ptrHit;
      draw(hit);
    }
  }
  primitivePlotter.endGroup();
}

void EventDataPlotter::drawTrajectory(const MCParticle& mcParticle, const AttributeMap& attributeMap)
{
  if (not mcParticle.isPrimaryParticle()) return;
  Vector3D pos(mcParticle.getVertex());
  Vector3D mom(mcParticle.getMomentum());
  double charge = mcParticle.getCharge();
  double time = mcParticle.getProductionTime();
  CDCTrajectory2D trajectory2D(pos.xy(), time, mom.xy(), charge);
  draw(trajectory2D, attributeMap);
}

void EventDataPlotter::drawTrajectory(const CDCSegment2D& segment,
                                      const AttributeMap& attributeMap)
{
  draw(segment.getTrajectory2D(), attributeMap);
}

void EventDataPlotter::drawTrajectory(const CDCSegmentTriple& segmentTriple,
                                      const AttributeMap& attributeMap)
{
  draw(segmentTriple.getTrajectory3D().getTrajectory2D(), attributeMap);
}

void EventDataPlotter::drawTrajectory(const CDCTrack& track, const AttributeMap& attributeMap)
{
  draw(track.getStartTrajectory3D().getTrajectory2D(), attributeMap);
}

void EventDataPlotter::drawTrajectory(const RecoTrack& recoTrack, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  primitivePlotter.startGroup(attributeMap);

  bool fitSuccessful = not recoTrack.getRepresentations().empty() and recoTrack.wasFitSuccessful();
  if (fitSuccessful) {
    std::vector<std::array<float, 2>> points;
    std::vector<std::array<float, 2>> tangents;

    for (auto recoHit : recoTrack.getRecoHitInformations()) {
      // skip for reco hits which have not been used in the fit (and therefore have no fitted information on the plane
      if (!recoHit->useInFit())
        continue;

      TVector3 pos;
      TVector3 mom;
      TMatrixDSym cov;

      try {
        const auto* trackPoint = recoTrack.getCreatedTrackPoint(recoHit);
        const auto* fittedResult = trackPoint->getFitterInfo();
        if (not fittedResult) {
          B2WARNING("Skipping unfitted track point");
          continue;
        }
        const genfit::MeasuredStateOnPlane& state = fittedResult->getFittedState();
        state.getPosMomCov(pos, mom, cov);
      } catch (genfit::Exception) {
        B2WARNING("Skipping state with strange pos, mom or cov");
        continue;
      }

      float x = pos.X();
      float y = pos.Y();
      float px = mom.X();
      float py = mom.Y();

      points.push_back({{x, y}});
      tangents.push_back({{px, py}});
    }
    primitivePlotter.drawCurve(points, tangents, attributeMap);
  }

  primitivePlotter.endGroup();
}
