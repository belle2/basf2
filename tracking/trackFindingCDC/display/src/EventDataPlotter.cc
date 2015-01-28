/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/EventDataPlotter.h"

#include <tracking/trackFindingCDC/display/SVGPrimitivePlotter.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

EventDataPlotter::EventDataPlotter(bool animate) :
  m_ptrPrimitivePlotter(new SVGPrimitivePlotter(AttributeMap {
  {"stroke", "orange"},
  {"stroke-width", "0.4"},
  {"fill", "none"}
})),
m_animate(animate)
{
}

EventDataPlotter::EventDataPlotter(PrimitivePlotter* ptrPrimitivePlotter, bool animate) :
  m_ptrPrimitivePlotter(ptrPrimitivePlotter),
  m_animate(animate)
{
  if (not m_ptrPrimitivePlotter) {
    B2WARNING("EventDataPlotter initialized with nullptr. Using default backend SVGPrimitivePlotter.");
    m_ptrPrimitivePlotter = new SVGPrimitivePlotter();
  }
}

EventDataPlotter::EventDataPlotter(const EventDataPlotter& eventDataPlotter) :
  m_ptrPrimitivePlotter(eventDataPlotter.m_ptrPrimitivePlotter ? eventDataPlotter.m_ptrPrimitivePlotter->clone() : nullptr),
  m_animate(eventDataPlotter.m_animate)
{
}

EventDataPlotter::~EventDataPlotter()
{
  delete m_ptrPrimitivePlotter;
  m_ptrPrimitivePlotter = nullptr;
}

EventDataPlotter* EventDataPlotter::clone()
{
  return new EventDataPlotter(*this);
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

void EventDataPlotter::setCanvasWidth(const float& width)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.setCanvasWidth(width);
  }
}

void EventDataPlotter::setCanvasHeight(const float& height)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.setCanvasHeight(height);
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
    AttributeMap groupAttributeMap {
      {"_showAt", getAnimationTimeFromNanoSeconds(tof)}
    };
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

/// --------------------- Draw Circle2D ------------------------
void EventDataPlotter::draw(const Circle2D& circle,
                            AttributeMap attributeMap)
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
  const float wireRadius = 0.1;
  const Vector2D& refPos = wire.getRefPos2D();

  draw(Circle2D(refPos, wireRadius), attributeMap);
}

/// --------------------- Draw CDCWireSuperLayer ------------------------
void EventDataPlotter::draw(const CDCWireSuperLayer& wireSuperLayer, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  float centerX = 0.0;
  float centerY = 0.0;

  float innerR = wireSuperLayer.getInnerPolarR();
  primitivePlotter.drawCircle(centerX, centerY, innerR, attributeMap);

  float outerR = wireSuperLayer.getOuterPolarR();
  primitivePlotter.drawCircle(centerX, centerY, outerR, attributeMap);
}

/// --------------------- Draw CDCWireTopology------------------------
void EventDataPlotter::draw(const CDCWireTopology& wireTopology, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  for (const CDCWireSuperLayer & wireSuperLayer : wireTopology.getWireSuperLayers()) {
    AttributeMap superLayerAttributes {
      {"fill" , wireSuperLayer.isAxial() ? "black" : "lightgray"},
      {"stroke" , "none"}
    };
    primitivePlotter.startGroup(superLayerAttributes);
    for (const CDCWireLayer & wireLayer : wireSuperLayer) {
      for (const CDCWire & wire : wireLayer) {
        draw(wire, attributeMap);
      }
    }
    primitivePlotter.endGroup();
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
  primitivePlotter.drawCircle(x, y, radius, attributeMap);

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

  float supportPointRadius = 0.2;
  Circle2D supportPoint(recoPos2D, supportPointRadius);
  draw(supportPoint, attributeMap);

  primitivePlotter.endGroup();
}


/// --------------------- Draw CDCRecoHit2D ------------------------
void EventDataPlotter::draw(const CDCRecoTangent& recoTangent, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const Vector2D fromPos = recoTangent.getFromRecoPos2D();
  const float fromX =  fromPos.x();
  const float fromY =  fromPos.y();

  const Vector2D toPos = recoTangent.getToRecoPos2D();
  const float toX =  toPos.x();
  const float toY =  toPos.y();

  primitivePlotter.drawLine(fromX, fromY, toX, toY, attributeMap);

  float touchPointRadius = 0.015;
  const Circle2D fromTouchPoint(fromPos, touchPointRadius);
  draw(fromTouchPoint, attributeMap);

  const Circle2D toTouchPoint(toPos, touchPointRadius);
  draw(toTouchPoint, attributeMap);
}

/// --------------------- Draw CDCTrajectory2D ------------------------
void EventDataPlotter::draw(const CDCTrajectory2D& trajectory2D, AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  AttributeMap defaultAttributeMap {};

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

  if (trajectory2D.getLocalCircle().isCircle()) {

    const float radius = trajectory2D.getLocalCircle().absRadius();

    const Vector2D trajectoryExit = trajectory2D.getExit();
    const float& endX = trajectoryExit.x();
    const float& endY = trajectoryExit.y();

    if (trajectoryExit.hasNAN()) {
      // No exit point out of the cdc could be detected.
      // Draw full circle

      const Vector2D center = trajectory2D.getGlobalCircle().center();
      const float radius = trajectory2D.getLocalCircle().absRadius();
      const float& centerX = center.x();
      const float& centerY = center.y();

      primitivePlotter.drawCircle(centerX, centerY, radius);


    } else {

      const Vector2D start = trajectory2D.getSupport();
      const float& startX = start.x();
      const float& startY = start.y();

      const int curvature = -charge;
      const bool sweepFlag = curvature > 0;

      // check if exit point is on the close or
      // on the far side of the circle
      const bool longArc = (trajectory2D.calcPerpS(trajectoryExit) > 0) ? false : true;
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
    const Vector2D start = trajectory2D.getSupport();
    const float& startX = start.x();
    const float& startY = start.y();

    const Vector2D trajectoryExit = trajectory2D.getExit();
    const float endX = trajectoryExit.x();
    const float endY = trajectoryExit.y();

    if (trajectoryExit.hasNAN()) {
      B2WARNING("Could not compute point off exit in a straight line case.");
    } else {
      primitivePlotter.drawLine(startX, startY, endX, endY, attributeMap);
    }
  }

}


void EventDataPlotter::draw(const genfit::TrackCand& gfTrackCand, const AttributeMap& attributeMap)
{
  StoreArray<CDCHit> storedHits;
  auto hitIDs = gfTrackCand.getHitIDs(Const::CDC);

  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  primitivePlotter.startGroup(attributeMap);

  for (int hitID : hitIDs) {
    const CDCHit* ptrHit = storedHits[hitID];
    if (ptrHit) {
      const CDCHit& hit = *ptrHit;
      draw(hit);
    }
  }

  primitivePlotter.endGroup();

}



